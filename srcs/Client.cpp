/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/19 09:25:38 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

const size_t	Client::_buffer_size(1);

Client::Client(void) :
	_sd(),
	_fd(),
	_cgi_fd(),
	_addr(),
	_socket_len(),
	_virtual_servers(),
	_input_str(),
	_output_str(),
	_cgi_output_str(),
	_exchanges(),
	_closing(false) {}

Client::Client(int sd, struct sockaddr addr, socklen_t socket_len,
	const std::list<const VirtualServer*> &virtual_servers) :
	_sd(sd),
	_fd(),
	_cgi_fd(),
	_addr(addr),
	_socket_len(socket_len),
	_virtual_servers(virtual_servers),
	_input_str(),
	_output_str(),
	_cgi_output_str(),
	_exchanges(),
	_closing(false) {}

Client::Client(const Client &x) :
	_sd(x._sd),
	_fd(x._fd),
	_cgi_fd(x._cgi_fd),
	_addr(x._addr),
	_socket_len(x._socket_len),
	_virtual_servers(x._virtual_servers),
	_input_str(x._input_str),
	_output_str(x._output_str),
	_cgi_output_str(x._output_str),
	_exchanges(x._exchanges),
	_closing(x._closing) {}

Client::~Client(void) {}

Client
&Client::operator=(const Client &x) {
	_fd = x._fd;
	_cgi_fd = x._cgi_fd;
	_exchanges = x._exchanges;
	_input_str = x._input_str;
	_output_str = x._output_str;
	_cgi_output_str = x._cgi_output_str;
	_closing = x._closing;
	return (*this);
}

int
Client::get_sd(void) const {
	return (_sd);
}

int
Client::get_fd(void) const {
	return (_fd);
}

int
Client::get_cgi_fd(void) const {
	return (_cgi_fd);
}

/*
 * REQUEST RECEPTION
 */

int
Client::read_socket(void) {
	char	buffer[_buffer_size + 1];
	int		ret;

	if (0 >= (ret = read(_sd, buffer, _buffer_size))) {
		if (0 == ret)
			std::cout << "the client closed the connection." << std::endl;
		else
			std::cerr << "error during reading the socket: " << strerror(errno) << std::endl;
		return (FAILURE);
	}
	buffer[ret] = '\0';
	_input_str += std::string(buffer);
	_input_str_parsing();
	if (!_exchanges.empty() && _exchanges.front().first.get_status() == Request::REQUEST_RECEIVED)
		return (_process(_exchanges.front()));
	return (SUCCESS);
}

void
Client::_failure(exchange_t &exchange, status_code_t status_code ) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);

	_closing = true;
	request.set_compromising(true);
	request.set_status(Request::REQUEST_RECEIVED);
	response.get_status_line().set_status_code(status_code);
}

bool
Client::_request_line_received(const Request &request) const {
	return (request.get_status() == Request::START && std::string::npos != _input_str.find("\r\n"));
}

bool
Client::_header_received(const Request &request) const {
	return (request.get_status() == Request::REQUEST_LINE_RECEIVED
			&& !_headers_received(request)
			&& std::string::npos != _input_str.find("\r\n"));
}

bool
Client::_headers_received(const Request &request) const {
	return (request.get_status() == Request::REQUEST_LINE_RECEIVED
			&& !_input_str.compare(0, 2, "\r\n"));
}

bool
Client::_body_received(const Request &request) const {
	return (request.get_status() == Request::HEADERS_RECEIVED
			&& ((_transfer_encoding_chunked(request)
					&& _input_str.find("0\r\n\r\n") != std::string::npos)
				|| (request.get_headers().key_exists(CONTENT_LENGTH)
					&& _input_str.size() >= static_cast<unsigned long>(std::atol(request.get_headers().get_value(CONTENT_LENGTH).front().c_str())))));
}

bool
Client::_trailer_received(const Request &request) const {
	return (request.get_status() == Request::BODY_RECEIVED
			&& !_trailers_received(request)
			&& std::string::npos != _input_str.find("\r\n"));
}

//TODO:: pas du tout comme ca qu'on repere la fin des trailers, provisoire, pour test
bool
Client::_trailers_received(const Request &request) const {
	return (request.get_status() == Request::BODY_RECEIVED
			&& !_input_str.compare(0, 2, "\r\n"));
}

bool
Client::_transfer_encoding_chunked(const Request &request) {
	if (request.get_headers().key_exists(TRANSFER_ENCODING)) {
		const std::list<std::string>& transfer_encoding_values = request.get_headers().get_value(TRANSFER_ENCODING);
		if (!transfer_encoding_values.empty())
			if (std::find(transfer_encoding_values.begin(), transfer_encoding_values.end(),
				Syntax::encoding_types_tab[CHUNKED].name) != transfer_encoding_values.end())
				return true;
	}
	return false;
}

bool
Client::_body_expected(const Request &request) const {
	return (_transfer_encoding_chunked(request)
		|| request.get_headers().key_exists(CONTENT_LENGTH));
}

bool
Client::_trailer_expected(const Request &request) const {
	return ((request.get_headers().key_exists(TRAILER)));
}

void
Client::_input_str_parsing(void) {
	while (!_closing && !_input_str.empty()) {

		if (_exchanges.empty() || _exchanges.back().first.get_status() == Request::REQUEST_RECEIVED)
			_exchanges.push_back(std::make_pair(Request(_virtual_servers.front()), Response()));
		exchange_t	&current_exchange(_exchanges.back());
		Request		&request(current_exchange.first);

		if (_request_line_received(request) && SUCCESS != _collect_request_line_elements(current_exchange))
			return ;
		while (_header_received(request))
			_collect_header(current_exchange);
		if (_headers_received(request) && SUCCESS != _check_headers(current_exchange))
			return ;
		if (_body_received(request))
			_collect_body(current_exchange);
		while (_trailer_received(request))
			_collect_header(current_exchange);
		if (_trailers_received(request) && SUCCESS != _check_trailer(current_exchange))
			return ;
		if (request.get_status() != Request::REQUEST_RECEIVED)
			return ;

	}
}

int
Client::_collect_request_line_elements(exchange_t &exchange) {
	Request		&request(exchange.first);
	size_t		first_sp(0);
	size_t		scnd_sp(0);
	size_t		end_rl(_input_str.find("\r\n"));

	if (std::string::npos == (first_sp = _input_str.find_first_of(' '))
			|| std::string::npos == (scnd_sp = _input_str.find_first_of(' ', first_sp + 1))) {
		_input_str.erase(0, end_rl + 2);
		_failure(exchange, BAD_REQUEST);
		return (FAILURE);
	}
	request.get_request_line().set_method(_input_str.substr(0, first_sp));
	request.get_request_line().set_request_target(_input_str.substr(first_sp + 1, scnd_sp - first_sp - 1));
	if (request.get_request_line().get_request_target()[0] != '/') {
		_failure(exchange, BAD_REQUEST);
		return (FAILURE);
	}
	request.get_request_line().set_http_version(_input_str.substr(scnd_sp + 1, (end_rl - scnd_sp - 1)));
	_input_str.erase(0, end_rl + 2);
	if (DEFAULT_METHOD == request.get_request_line().get_method()) {
		_failure(exchange, NOT_IMPLEMENTED);
		return (FAILURE);
	}
	request.set_status(Request::REQUEST_LINE_RECEIVED);
	_pick_location(request);
	return (SUCCESS);
}

int
Client::_collect_header(exchange_t &exchange) {
	Request				&request(exchange.first);
	size_t				col(0);
	size_t				end_header(_input_str.find("\r\n"));
	header_t			current_header;

	if (std::string::npos != (col = _input_str.find_first_of(':'))) {
		current_header.name = _input_str.substr(0, col);
		current_header.unparsed_value = Syntax::trim_whitespaces(_input_str.substr(col + 1, (end_header - col - 1)));
		request.get_headers().insert(current_header);
	}
	_input_str.erase(0, end_header + 2);
	return (SUCCESS);
}

int
Client::_check_headers(exchange_t &exchange) {
	Request		&request(exchange.first);

	_input_str.erase(0, _input_str.find("\r\n") + 2);
	if (_process_request_headers(exchange.first) == FAILURE) {
		_failure(exchange, BAD_REQUEST);
		return (FAILURE);
	}
	if (_body_expected(request))
		request.set_status(Request::HEADERS_RECEIVED);
	else
		request.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

int
Client::_check_trailer(exchange_t &exchange) {
	exchange.first.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

int
Client::_collect_body(exchange_t &exchange) {
	size_t		body_length(0);
	Request		&request(exchange.first);

	if (_transfer_encoding_chunked(request))
		body_length = _input_str.find("0\r\n\r\n") + 5;
	else if (request.get_headers().key_exists(CONTENT_LENGTH))
		body_length = static_cast<unsigned long>(std::atol(request.get_headers().get_value(CONTENT_LENGTH).front().c_str()));
	request.set_body(_input_str.substr(0, body_length));
	_input_str.erase(0, body_length);
	if (_trailer_expected(request))
		request.set_status(Request::BODY_RECEIVED);
	else
		request.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

/* _pick_virtual_server :
 * get the right configuration of virtual host
 * based on header host and server_name
 *
 */

void
Client::_pick_virtual_server(Request &request) {
	std::vector<std::string> host_elements;
	std::list<std::string> server_names;

	request.set_virtual_server(_virtual_servers.front());
	if (!request.get_headers().key_exists(HOST))
		return ;
	for(std::list<const VirtualServer*>::const_iterator it = _virtual_servers.begin();
		it != _virtual_servers.end() ; it++) {
		server_names = (*it)->get_server_names();
		for (std::list<std::string>::const_iterator cit = server_names.begin();
			cit != server_names.end(); cit++) {
			if (*cit == request.get_headers().get_unparsed_value(HOST)) {
				request.set_virtual_server(*it);
				return ;
			}
		}
	}
}

void
Client::_pick_location(Request &request) {
	std::string					request_target(request.get_request_line().get_request_target());
	std::string					absolute_path(request_target.substr(0, request_target.find('?')));
	const std::list<Location>	&locations(request.get_virtual_server()->get_locations());

	for (std::list<Location>::const_iterator it(locations.begin()) ; it != locations.end() ; it++)
		if (!absolute_path.compare(0, (it->get_path()).size(), it->get_path())
				&& request.get_location()->get_path().size() < it->get_path().size())
			request.set_location(&(*it));
}

/*
 * Headers parsers
 */

bool Client::_comp_q_factor(const std::pair<std::string, float> & a, const std::pair<std::string, float> & b) {
	return a.second > b.second;
}

std::list<std::string>
Client::_parse_coma_q_factor(const std::string& unparsed_value) {
	std::list<std::string> new_list;
	std::vector<std::string> elements_split;
	std::list<std::pair<std::string, float> > elements;
	float q_factor;
	size_t size, q_factor_pos;

	elements_split = Syntax::split(unparsed_value, ", ");
	size = elements_split.size();
	for (size_t i = 0; i < size; i++) {
		q_factor_pos = elements_split[i].find(";q=");
		if (q_factor_pos != std::string::npos) {
			q_factor = strtod(elements_split[i].substr(q_factor_pos + 4).c_str(), NULL);
			elements_split[i] = elements_split[i].substr(0, q_factor_pos);
		}
		else
			q_factor = 1.0;
		elements.push_back(std::make_pair(elements_split[i], q_factor));
	}
	elements.sort(_comp_q_factor);
	for(std::list<std::pair<std::string, float> >::iterator it = elements.begin(); it != elements.end(); it++) {
		new_list.push_back(it->first);
	}
	return new_list;
}

int
Client::_request_accept_charset_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(ACCEPT_CHARSET);
	std::list<std::string> charsets_list = _parse_coma_q_factor(unparsed_header_value);

	request.get_headers().set_value(ACCEPT_CHARSET, charsets_list);
	return SUCCESS;
}

/* basic language tag check :
 * regular examples : fr, en-gb, en-US, fr-Latin-FR, cel-gaulish
 * accepted formats, parsed on '-' delimiter :
 * - language : 2 or 3 alpha char
 * - region (if exist) : >= 2 alpha-num char (both upper and lower case accepted)
 * - script (if exist) : >= 2 alpha-num char
 * - more than 3 compounds are accepted without format checking
 * - asterisk form '*'
 * case insensitive
 *
 */

bool
Client::_is_valid_language_tag(const std::string& language_tag) {
	std::vector<std::string> compounds = Syntax::split(language_tag, "-");
	std::string language, script, region;

	if (language_tag == "*")
		return true;
	language = compounds[0];
	if (!Syntax::str_is_alpha(language)
		|| language.size() < 2 || language.size() > 3)
		return false;
	if (compounds.size() > 2) {
		script = compounds[1];
		region = compounds[2];
		if (!Syntax::str_is_alnum(script) || !Syntax::str_is_alnum(region))
			return false;
		if (script.size() < 2 || region.size() < 2)
			return false;
	}
	if (compounds.size() == 2) {
		region = compounds[1];
		if (!Syntax::str_is_alnum(region) || region.size() < 2)
			return false;
	}
	return true;
}

int
Client::_request_accept_language_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(ACCEPT_LANGUAGE);
	std::list<std::string> languages_list = _parse_coma_q_factor(unparsed_header_value);

	for(std::list<std::string>::const_iterator it = languages_list.begin(); it != languages_list.end(); it++) {
		if (!_is_valid_language_tag(*it))
			return FAILURE;
	}
	request.get_headers().set_value(ACCEPT_LANGUAGE, languages_list);
	return SUCCESS;
}

int
Client::_request_authorization_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(AUTHORIZATION);
	std::vector<std::string> compounds = Syntax::split(unparsed_header_value, " ");
	std::list<std::string> definitive_value;

	if (compounds.size() != 2)
		return FAILURE;
	definitive_value.push_back(compounds[0]); // authorization type
	definitive_value.push_back(compounds[1]); // authorization credentials
	request.get_headers().set_value(AUTHORIZATION, definitive_value);
	return SUCCESS;
}

int
Client::_request_content_length_parser(Request &request) {
	std::string content_length_str = request.get_headers().get_unparsed_value(CONTENT_LENGTH);
	std::list<std::string> definitive_value;

	if(request.get_headers().key_exists(TRANSFER_ENCODING))
		return FAILURE;
	if (!Syntax::str_is_num(content_length_str))
		return FAILURE;
	definitive_value.push_back(content_length_str);
	request.get_headers().set_value(CONTENT_LENGTH, definitive_value);
	return (SUCCESS);
}

int
Client::_request_content_type_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(CONTENT_TYPE);
	std::list<std::string> content_type_list;
	std::string mime_type;
	std::string parameter;
	size_t semicolon_pos;

	semicolon_pos = unparsed_header_value.find_first_of(';');
	mime_type = unparsed_header_value.substr(0, semicolon_pos);
	content_type_list.push_back(mime_type);
	if (semicolon_pos != std::string::npos) {
		parameter = unparsed_header_value.substr(semicolon_pos + 1);
		content_type_list.push_back(parameter);
	}
	if (!Syntax::is_accepted_value<Syntax::mime_type_entry_t>(mime_type,
		Syntax::mime_types_tab, TOTAL_MIME_TYPES))
		return FAILURE;
	request.get_headers().set_value(CONTENT_TYPE, content_type_list);
	return SUCCESS;
}

bool
Client::is_valid_http_date(const std::string& date_str) {
	char* strptime_ret;
	int i = -1;
	std::string HTTP_date_fmt[3] = {"%a, %d %b %Y %T", "%A, %d-%b-%y %T %Z", "%a %b  %d %T %Y"};
	struct tm timeval;

	while(++i < 3) {
		strptime_ret = strptime(date_str.c_str(), HTTP_date_fmt[i].c_str(), &timeval);
		if (strptime_ret && (*strptime_ret == '\0' || *strptime_ret == 'G'))
			return true;
	}
	return false;
}

/* _date_handler :
 * accept the three HTTP dates formats, in order in the std::string array :
 * - real example : Sun, 06 Nov 1994 08:49:37 GMT
 * - obsolete 1 (RFC 850) example : Sunday, 06-Nov-94 08:49:37 GMT
 * - obsolete 2 (ANSI C) example : Sun Nov  6 08:49:37 1994
 *
 */

int
Client::_request_date_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(DATE);
	std::list<std::string> definitive_value;

	if (!is_valid_http_date(unparsed_header_value))
		return FAILURE;
	definitive_value.push_back(unparsed_header_value);
	request.get_headers().set_value(DATE, definitive_value);
	return SUCCESS;
}

int
Client::_request_host_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(HOST);
	std::vector<std::string> compounds;
	std::list<std::string> definitive_value;

	if (unparsed_header_value.find_first_of(WHITESPACES) != std::string::npos)
		return FAILURE;
	compounds = Syntax::split(unparsed_header_value, ":");
	if (compounds.size() > 2)
		return FAILURE;

	definitive_value.push_back(compounds[0]);// host name
	if (compounds.size() == 2)
		definitive_value.push_back(compounds[1]); //port
	request.get_headers().set_value(HOST, definitive_value);
	_pick_virtual_server(request);
	return (SUCCESS);
}

int
Client::_request_referer_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(REFERER);
	URI_form_t uri_form = Syntax::get_URI_form(unparsed_header_value);

	if (uri_form != ABSOLUTE_URI && uri_form != PARTIAL_URI)
		return FAILURE;
	request.get_headers().set_value(REFERER,
		std::list<std::string>(1, unparsed_header_value));
	return SUCCESS;
}

int
Client::_request_transfer_encoding_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(TRANSFER_ENCODING);
	std::list<std::string> encoding_types_list = _parse_coma_q_factor(unparsed_header_value);

	for(std::list<std::string>::iterator it = encoding_types_list.begin();
		it != encoding_types_list.end(); it++)
		if (!Syntax::is_accepted_value(*it, Syntax::encoding_types_tab, TOTAL_ENCODING_TYPES))
			return FAILURE;
	request.get_headers().set_value(TRANSFER_ENCODING, encoding_types_list);
	return SUCCESS;
}

int
Client::_request_user_agent_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(USER_AGENT);
	std::vector<std::string> compounds = Syntax::split(unparsed_header_value, " ");
	std::list<std::string> definitive_value;

	for(size_t i = 0; i < compounds.size(); i++)
		definitive_value.push_back(compounds[i]);
	request.get_headers().set_value(USER_AGENT, definitive_value);
	return SUCCESS;
}

void Client::_send_debug_str(const std::string& str) const {
	std::string to_send = str + "\n";
	size_t size = to_send.size();

	send(_sd, to_send.c_str(), size, 0);
}

int
Client::_process_request_headers(Request &request) {
	const AHTTPMessage::HTTPHeaders& headers = request.get_headers();

	int (Client::*handler_functions[])(Request &request) = {&Client::_request_accept_charset_parser,
		&Client::_request_accept_language_parser, &Client::_request_authorization_parser,
		&Client::_request_content_length_parser, &Client::_request_content_type_parser, &Client::_request_date_parser, NULL,
		&Client::_request_referer_parser, &Client::_request_transfer_encoding_parser, &Client::_request_user_agent_parser,
	};
	if (!request.get_headers().key_exists(HOST)
			|| _request_host_parser(request) == FAILURE)
		return FAILURE;
	for(size_t i = 0; i < TOTAL_REQUEST_HEADERS; i++) {
		if (Syntax::request_headers_tab[i].header_index != HOST
			&& headers.key_exists(Syntax::request_headers_tab[i].header_index)) {
			if ((this->*handler_functions[i])(request) == FAILURE)
				return FAILURE;
		}
	}
	return SUCCESS;
}

/*
 * RESPONSE SENDING
 */

int
Client::_process(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);

	request.set_status(Request::REQUEST_PROCESSED);
	response.get_status_line().set_http_version("HTTP/1.1");
	if (response.get_status_line().get_status_code() != TOTAL_STATUS_CODE)
		return (_process_error(exchange));
	if (_is_cgi_related(request))
		return (_process_cgi(exchange));
	if (request.get_request_line().get_method() == GET)
		return (_process_GET(exchange));
	return (FAILURE);
}


std::string
Client::_format_index_path(const std::string& dir_path, const std::string& index_file) {
	std::string definite_path = dir_path;
	std::string definite_index_file = index_file;

	if (*(--dir_path.end()) != '/')
		definite_path += "/";
	if (*index_file.begin() == '/')
		definite_index_file = index_file.substr(1);
	return definite_path + definite_index_file;
}

int
Client::_get_default_index(exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	std::string dir_path = response.get_target_path(), definite_path;
	std::list<std::string> index_list = request.get_location()->get_index();

	if (index_list.empty()) {
		definite_path = _format_index_path(dir_path, "index.html");
	} else {
		for(std::list<std::string>::iterator it = index_list.begin(); it != index_list.end(); it++) {
			definite_path = _format_index_path(dir_path, *it);
			if (REGULAR_FILE == Syntax::get_path_type(definite_path))
				break ;
		}
	}
	if (Syntax::get_path_type(definite_path) == INVALID_PATH)
		return FAILURE;
	response.set_target_path(definite_path);
	return SUCCESS;
}

std::string
Client::_format_autoindex_page(exchange_t& exchange, const std::set<std::string>& directory_names,
	const std::set<std::string>& file_names) {
	std::stringstream ss;
	std::string		target_path = exchange.second.get_target_path();
	std::string		request_target = exchange.first.get_request_line().get_request_target();
	std::string		dir_name(request_target.substr(0, request_target.find('?')));

	ss << "<html>" << std::endl << "<head>" << std::endl;
	ss << "<title>Index of </title></head>" << std::endl;
	ss << "<body bgcolor=\"white\">" << std::endl;
	ss << "<h1>Index of " << dir_name << "</h1><hr><pre>";
	ss << "<table><tr><th>Name</th><th>Last modification</th><th>Size</th></tr>";
	for (std::set<std::string>::iterator it = directory_names.begin(); it != directory_names.end(); it++)
		_format_autoindex_entry(ss, *it, target_path, true);
	for (std::set<std::string>::iterator it = file_names.begin(); it != file_names.end(); it++)
		_format_autoindex_entry(ss, *it, target_path, false);
	ss << "</table></pre><hr></body>" << std::endl << "</html>" << std::endl;
	return ss.str();
}

void
Client::_format_autoindex_entry(std::stringstream& ss, const std::string& filename,
	const std::string& target_path, bool is_dir) {
	std::string		definite_filename, fullpath, size_str;
	struct stat		stat_buf;
	char 			time_buf[64];
	time_t			last_modification;
	struct tm		*tm;

	fullpath = target_path + filename;
	if (stat(fullpath.c_str(), &stat_buf) == -1) {
		return;
	}
	last_modification = stat_buf.st_mtim.tv_sec;
	tm = localtime(&last_modification);
	strftime(time_buf, sizeof(time_buf), "%d-%b-%Y %H:%M", tm);
	definite_filename = is_dir ? filename + "/" : filename;
	ss << "<tr><td><a href=\"" << definite_filename << "\">";
	ss << definite_filename << "</a></td>";
	ss << "<td>--" << time_buf << "--</td>";
	ss << "<td>";
	if (is_dir)
		ss << "-";
	else
		ss << stat_buf.st_size;
	ss << "</td></tr>" << std::endl;
}

int
Client::_generate_autoindex(exchange_t &exchange) {
	Response		&response(exchange.second);
	DIR       		*directory;
	std::set<std::string> directory_names, file_names;
	struct dirent	*file_listing;


	directory = opendir(response.get_target_path().c_str());
	if (!directory) {
		response.get_status_line().set_status_code(FORBIDDEN);
		return FAILURE;
	}
	while ((file_listing = readdir(directory))) {
		if (file_listing->d_type == DT_DIR
			&& strcmp(file_listing->d_name, ".") == 0)
			continue;
		if (file_listing->d_type == DT_DIR)
			directory_names.insert(file_listing->d_name);
		else if (file_listing->d_type == DT_REG)
			file_names.insert(file_listing->d_name);
	}
	response.set_body(_format_autoindex_page(exchange, directory_names, file_names));
	_generate_basic_headers(exchange);
	closedir(directory);
	return (_build_output_str(exchange));
}

bool
Client::_is_cgi_related(const Request &request) const {
	std::string	path(_build_cgi_script_path(request));
	return (path.find(".") != std::string::npos
				&& path.substr(path.rfind(".")) == request.get_location()->get_cgi_extension());
}

std::string
Client::_build_cgi_script_path(const Request &request) const {
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	cgi_extension(request.get_location()->get_cgi_extension());
	std::string	path(request_target.substr(0, request_target.find(cgi_extension) + cgi_extension.size()));
	return (request.get_location()->get_cgi_path() + path);
}

int
Client::_create_cgi_child_process(void) {
	pid_t	pid;

	pid = fork();
	while (pid == -1 && errno == EAGAIN)
		pid = fork();
	return (pid);
}

int
Client::_process_cgi(exchange_t &exchange) {
	Request				&request(exchange.first);
	CGIMetaVariables	mv(request);
	pid_t				pid;
	int					req_pipe[2];
	int					res_pipe[2];
	pipe(req_pipe);
	pipe(res_pipe);
	std::cout << "PROCESS CGI" << std::endl;
	if (-1 == (pid = _create_cgi_child_process()))
		return (FAILURE);
	if (!pid) {
		close(req_pipe[1]);
		close(res_pipe[0]);
		dup2(req_pipe[0], STDIN_FILENO);
		dup2(res_pipe[1], STDOUT_FILENO);
		if (0 > execve(_build_cgi_script_path(request).c_str(), mv.get_tab(), mv.get_tab()))
			perror("execve");
		return (FAILURE);
	}
	close(req_pipe[0]);
	close(res_pipe[1]);
	write(req_pipe[1], request.get_body().c_str(), request.get_body().size());
	close(req_pipe[1]);
	_cgi_fd = res_pipe[0];
	return (SUCCESS);
}

int
Client::_process_GET(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);

	std::string	path(_build_resource_path(request));
	path_type_t path_type = Syntax::get_path_type(path);

	if (path_type == INVALID_PATH) {
		response.get_status_line().set_status_code(NOT_FOUND);
		return (_process_error(exchange));
	}
	response.set_target_path(path);
	if (path_type == DIRECTORY) {
		if (_get_default_index(exchange) == FAILURE) {
			if (request.get_location()->is_autoindex())
				return (_generate_autoindex(exchange));
			response.get_status_line().set_status_code(FORBIDDEN);
			return (_process_error(exchange));
		}
	}
	response.get_status_line().set_status_code(OK);
	return (_open_file_to_read(response.get_target_path()));
}

void
Client::_generate_basic_headers(exchange_t &exchange) {
	Response& response = exchange.second;
	status_code_t error_code = response.get_status_line().get_status_code();
	std::stringstream ss;

	ss << response.get_body().length();
	_response_server_handler(exchange);
	_response_date_handler(exchange);
	response.get_headers().insert(CONTENT_TYPE, "text/html");
	response.get_headers().insert(CONTENT_LENGTH, ss.str());
	if (error_code == FORBIDDEN)
		_response_www_authenticate_handler(exchange);
	if (error_code == SERVICE_UNAVAILABLE)
		_response_retry_after_handler(exchange);
}

void
Client::_generate_error_page(exchange_t &exchange) {
	Response& response = exchange.second;
	status_code_t error_code = response.get_status_line().get_status_code();
	std::stringstream ss;

	ss << "<html>" << std::endl << "<head>" << std::endl;
	ss << "<title>Error " << Syntax::status_codes_tab[error_code].code_str;
	ss << " - " << Syntax::status_codes_tab[error_code].reason_phrase;
	ss << "</title>" << std::endl << "</head>" << std::endl;
	ss << "<body bgcolor=\"white\">" << std::endl << "<center>" << std::endl;
	ss << "<h1>Error " << Syntax::status_codes_tab[error_code].code_str;
	ss << " - " << Syntax::status_codes_tab[error_code].reason_phrase;
	ss << "</h1>" << std::endl << "</center>" << std::endl;
	ss << "<hr><center>webserv/1.0</center>" << std::endl;
	ss << "</body>" << std::endl << "</html>" << std::endl;
	response.set_body(ss.str());
	_generate_basic_headers(exchange);
}

int
Client::_process_error(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);
	std::string error_page_path;
	std::list<status_code_t>	error_codes(request.get_virtual_server()->get_error_page_codes());

	request.get_headers().clear();
	response.get_headers().clear();
	if (!request.get_location())
		request.set_location(&request.get_virtual_server()->get_locations().back());
	for (std::list<status_code_t>::iterator it(error_codes.begin()) ; it != error_codes.end() ; it++) {
		if (response.get_status_line().get_status_code() == *it) {
			error_page_path = request.get_virtual_server()->get_error_page_path();
			response.set_target_path(error_page_path);
			return(_open_file_to_read(error_page_path));
		}
	}
	_generate_error_page(exchange);
	return (_build_output_str(exchange));
}


std::string
Client::_build_resource_path(Request &request) {
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	absolute_path(request_target.substr(0, request_target.find('?')));
	std::string	location_root(request.get_location()->get_root());

	return (location_root + absolute_path);
}

int
Client::_open_file_to_read(const std::string &path) {
	if (0 > (_fd = open(path.c_str(), O_RDONLY))) {
		std::cerr << "error during opening a file: ";
		std::cerr << strerror(errno) << std::endl;
		return (FAILURE);
	}
	return (SUCCESS);
}

int
Client::read_cgi(void) {
	char		buffer[_buffer_size + 1];
	int			ret;

	ret = read(_cgi_fd, buffer, _buffer_size);
	if (ret < 0) {
		close(_cgi_fd);
		return (FAILURE);
	}
	if (ret == 0) {
		close(_cgi_fd);
		_cgi_fd = 0;
		std::cout << "end of file" << std::endl;
		return (_cgi_output_str_parsing());
	}
	buffer[ret] = '\0';
	std::cout << buffer;
	_cgi_output_str += buffer;
	return (SUCCESS);
}

int
Client::_cgi_output_str_parsing(void) {
	CGIResponse	cgi_response;

	while (std::string::npos != _cgi_output_str.find("\n")
			&& _cgi_output_str.compare(0, 1, "\n"))
		_collect_cgi_header(cgi_response);
	_cgi_output_str.erase(0, _cgi_output_str.find("\n") + 1);
	cgi_response.set_body(_cgi_output_str);
	_cgi_output_str.clear();
	cgi_response.get_headers().render();
	return (SUCCESS);
}

void
Client::_collect_cgi_header(CGIResponse &cgi_response) {
	size_t				col(0);
	size_t				end_header(_cgi_output_str.find("\n"));
	header_t			current_header;

	if (std::string::npos != (col = _cgi_output_str.find_first_of(':'))) {
		current_header.name = _cgi_output_str.substr(0, col);
		current_header.unparsed_value = Syntax::trim_whitespaces(_cgi_output_str.substr(col + 1, (end_header - col - 1)));
		cgi_response.get_headers().insert(current_header);
	}
	_cgi_output_str.erase(0, end_header + 1);
}

int
Client::read_file(void) {
	exchange_t	&exchange(_exchanges.front());
	Response	&response(exchange.second);
	char		buffer[_buffer_size + 1];
	int			ret;

	ret = read(_fd, buffer, _buffer_size);
	if (ret < 0) {
		close(_fd);
		return (FAILURE);
	}
	if (ret == 0) {
		close(_fd);
		_fd = 0;
		if (_process_response_headers(exchange) == FAILURE)
			return (_process_error(exchange));
		return (_build_output_str(exchange));
	}
	buffer[ret] = '\0';
	response.set_body(response.get_body() + std::string(buffer));
	return (SUCCESS);
}

int
Client::_build_output_str(exchange_t &exchange) {
	Response	&response(exchange.second);
	AHTTPMessage::HTTPHeaders& headers = response.get_headers();

	_output_str.clear();
	_output_str += response.get_status_line().get_http_version();
	_output_str += " ";
	_output_str += Syntax::status_codes_tab[response.get_status_line().get_status_code()].code_str;
	_output_str += " ";
	_output_str += Syntax::status_codes_tab[response.get_status_line().get_status_code()].reason_phrase;
	_output_str += "\r\n";
	for(size_t i = 0; i < TOTAL_RESPONSE_HEADERS; i++) {
		if (headers.key_exists(Syntax::response_headers_tab[i].header_index)) {
			_output_str += Syntax::response_headers_tab[i].name + ": ";
			_output_str += headers.get_unparsed_value(Syntax::response_headers_tab[i].header_index);
			_output_str += "\r\n";
		}
	}
	_output_str += "\r\n";
	_output_str += response.get_body();
	return (_write_socket(exchange));
}

int
Client::_write_socket(exchange_t &exchange) {
	Request		&request(exchange.first);

	write(_sd, _output_str.c_str(), _output_str.size());
	if (request.get_compromising())
		return (FAILURE);
	_exchanges.pop_front();
	if (!_exchanges.empty() && _exchanges.front().first.get_status() == Request::REQUEST_RECEIVED)
		return (_process(_exchanges.front()));
	return (SUCCESS);
}

void
Client::_pick_content_type(exchange_t &exchange) {
	Response& response = exchange.second;
	const std::string& path = response.get_target_path();
	std::string content_type = Syntax::mime_types_tab[APPLICATION_OCTET_STREAM].name;
	std::string extension;
	size_t extension_point_pos;


	extension_point_pos = path.find_last_of('.');
	if (extension_point_pos != std::string::npos) {
		extension = path.substr(extension_point_pos);
		for(size_t i = 0; i < TOTAL_MIME_TYPES; i++) {
			if (extension == Syntax::mime_types_tab[i].ext) {
				content_type = Syntax::mime_types_tab[i].name;
				break ;
			}
		}
	}
	response.set_content_type(content_type);
}

int
Client::_process_response_headers(exchange_t &exchange) {
	int (Client::*response_handlers[])(exchange_t &) = {&Client::_response_allow_handler,
		&Client::_response_content_language_handler, &Client::_response_content_length_handler,
		&Client::_response_content_location_handler, &Client::_response_content_type_handler,
		&Client::_response_date_handler, &Client::_response_last_modified_handler,
		&Client::_response_location_handler, &Client::_response_location_handler,
		&Client::_response_retry_after_handler, &Client::_response_server_handler,
		&Client::_response_transfer_encoding_handler, &Client::_response_www_authenticate_handler};

	_pick_content_type(exchange);
	for (size_t i = 0; i < TOTAL_RESPONSE_HEADERS; i++) {
		if (!(this->*response_handlers[i])(exchange))
			return FAILURE;
	}
	return SUCCESS;
}

bool
Client::_is_allowed_method(const std::list<std::string>& allowed_methods, method_t method) {
	for(std::list<std::string>::const_iterator it = allowed_methods.begin();
		it != allowed_methods.end(); it++) {
		if (*it == Syntax::method_tab[method].name)
			return true;
	}
	return false;
}

int
Client::_response_allow_handler(exchange_t &exchange) {
	Request &request = exchange.first;
	Response &response = exchange.second;
	std::string method_output;
	std::list<std::string> allowed_methods = request.get_location()->get_methods();

	if (!allowed_methods.empty() && !response.is_error_page()) {
		if (!_is_allowed_method(allowed_methods, request.get_request_line().get_method())) {
			response.get_status_line().set_status_code(METHOD_NOT_ALLOWED);
			return FAILURE;
		}
		for (std::list<std::string>::iterator it = allowed_methods.begin(); it != allowed_methods.end(); it++) {
			method_output += *it + ", ";
		}
		method_output = method_output.substr(0, method_output.size() - 2);
		response.get_headers().insert(ALLOW, method_output);
	}
	return SUCCESS;
}

std::string
Client::_html_content_language_parser(const Response& response) {
	std::string content_language_str, line_tag;
	const std::string& body = response.get_body();
	size_t html_tag_pos, end_html_tag_pos, lang_pos, end_lang_pos;

	html_tag_pos = body.find("<html");
	if (html_tag_pos != std::string::npos) {
		line_tag = body.substr(html_tag_pos);
		end_html_tag_pos = line_tag.find_first_of('>');
		if (end_html_tag_pos != std::string::npos) {
			line_tag = line_tag.substr(0, end_html_tag_pos);
			lang_pos = line_tag.find("lang=\"");
			if (lang_pos != std::string::npos) {
				line_tag = line_tag.substr(lang_pos + 6);
				end_lang_pos = line_tag.find_first_of('"');
				if (end_lang_pos != std::string::npos) {
					content_language_str = line_tag.substr(0, end_lang_pos);
					if (!_is_valid_language_tag(content_language_str))
						content_language_str.clear();
				}
			}
		}
	}
	return content_language_str;
}

std::string
Client::_xml_content_language_parser(const Response& response) {
	std::string content_language_str, line_tag;
	const std::string& body = response.get_body();
	size_t lang_pos, end_lang_pos;

	lang_pos = body.find("xml:lang=\"");
	if (lang_pos != std::string::npos) {
		line_tag = body.substr(lang_pos + 10);
		end_lang_pos = line_tag.find_first_of('"');
		if (end_lang_pos != std::string::npos) {
			content_language_str = line_tag.substr(0, end_lang_pos);
			if (!_is_valid_language_tag(content_language_str))
				content_language_str.clear();
		}
	}
	return content_language_str;
}

bool
Client::_is_accepted_language(const std::string& language_found, const std::list<std::string>& allowed_languages) {
	if (allowed_languages.empty())
		return true;
	for(std::list<std::string>::const_iterator it = allowed_languages.begin(); it != allowed_languages.end(); it++) {
		if (*it == "*")
			return true;
		if (language_found.find(*it) != std::string::npos)
			return true;
	}
	return false;
}

int
Client::_response_content_language_handler(exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	std::string language;
	const std::string& content_type = response.get_content_type();

	if (content_type == Syntax::mime_types_tab[TEXT_HTML].name)
		language = _html_content_language_parser(response);
	else if (content_type == Syntax::mime_types_tab[APPLICATION_XML].name)
		language = _xml_content_language_parser(response);
	if (!language.empty()) {
		if(request.get_headers().key_exists(ACCEPT_LANGUAGE) &&
			!_is_accepted_language(language, request.get_headers().get_value(ACCEPT_LANGUAGE))) {
			response.get_status_line().set_status_code(NOT_ACCEPTABLE);
			return FAILURE;
		}
		response.get_headers().insert(CONTENT_LANGUAGE, language);
	}
	return SUCCESS;
}

int
Client::_response_content_length_handler(exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	struct stat buf;
	std::stringstream ss;

	if (stat(response.get_target_path().c_str(), &buf) != -1) {
		if (buf.st_size > static_cast<long>(request.get_virtual_server()->get_client_max_body_size())) {
			response.get_status_line().set_status_code(PAYLOAD_TOO_LARGE);
			return FAILURE;
		}
		ss << buf.st_size;
		response.get_headers().insert(CONTENT_LENGTH, ss.str());
	}
	return SUCCESS;
}

int
Client::_response_content_location_handler(exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	location_str(request_target.substr(0, request_target.find('?')));

	if (response.is_error_page())
		return SUCCESS;
	response.get_headers().insert(CONTENT_LOCATION, location_str);
	return SUCCESS;
}

bool
Client::_is_accepted_charset(const std::string& charset_found, const std::list<std::string>& allowed_charsets) {
	if (allowed_charsets.empty())
		return true;
	for (std::list<std::string>::const_iterator it = allowed_charsets.begin(); it != allowed_charsets.end(); it++) {
		if (*it == "*")
			return true;
		if (charset_found.find(Syntax::str_to_lower(*it)) != std::string::npos)
			return true;
	}
	return false;
}

std::string
Client::_html_charset_parser(const Response& response) {
	const std::string& body = response.get_body();
	std::string header_bloc, charset, meta_tag("<meta charset=\"") ;
	size_t header_bloc_begin, header_bloc_end, meta_bloc_begin, meta_bloc_end;

	header_bloc_begin = body.find("<head");
	if (header_bloc_begin != std::string::npos) {
		header_bloc = body.substr(header_bloc_begin);
		header_bloc_end = header_bloc.find("</head>");
		if (header_bloc_end != std::string::npos) {
			header_bloc = header_bloc.substr(0, header_bloc_end);
			meta_bloc_begin = header_bloc.find(meta_tag);
			if (meta_bloc_begin != std::string::npos) {
				charset = header_bloc.substr(meta_bloc_begin + meta_tag.size());
				meta_bloc_end = charset.find_first_of('"');
				if (meta_bloc_end != std::string::npos) {
					charset = Syntax::str_to_lower(charset.substr(0, meta_bloc_end));
				}
			}
		}
	}
	return charset;
}

std::string
Client::_xml_charset_parser(const Response& response) {
	const std::string& body = response.get_body();
	std::string xml_tag, encoding, charset;
	size_t xml_tag_begin, xml_tag_end, encoding_begin, encoding_end;

	xml_tag_begin = body.find("<?xml");
	if (xml_tag_begin != std::string::npos) {
		xml_tag = body.substr(xml_tag_begin);
		xml_tag_end = xml_tag.find("?>");
		if (xml_tag_end != std::string::npos) {
			xml_tag = xml_tag.substr(0, xml_tag_end);
			encoding_begin = xml_tag.find("encoding=\"");
			if (encoding_begin != std::string::npos) {
				encoding = xml_tag.substr(encoding_begin + 10);
				encoding_end = encoding.find_first_of('"');
				if (encoding_end != std::string::npos) {
					charset = Syntax::str_to_lower(encoding.substr(0, encoding_end));
				}
			}
		}
	}
	return charset;
}

int
Client::_response_content_type_handler(exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	std::string charset, content_type;

	content_type = response.get_content_type();
	if (content_type == Syntax::mime_types_tab[TEXT_HTML].name)
		charset = _html_charset_parser(response);
	else if (content_type == Syntax::mime_types_tab[APPLICATION_XML].name)
		charset = _xml_charset_parser(response);
	if (!charset.empty()) {
		if (request.get_headers().key_exists(ACCEPT_CHARSET) &&
			!_is_accepted_charset(charset, request.get_headers().get_value(ACCEPT_CHARSET))) {
			response.get_status_line().set_status_code(NOT_ACCEPTABLE);
			return FAILURE;
		}
		content_type += "; charset=" + charset;
	}
	response.get_headers().insert(CONTENT_TYPE, content_type);
	return SUCCESS;
}

std::string
Client::get_current_HTTP_date(void) {
	struct tm *date = NULL;
	char buff[64];
	struct timeval tv;

	gettimeofday(&tv, NULL);
	date = localtime(&(tv.tv_sec));
	strftime(buff, sizeof(buff), "%a, %d %b %Y %T GMT+02", date);
	return std::string(buff);
}

int
Client::_response_date_handler(exchange_t &exchange) {
	Response& response = exchange.second;

	response.get_headers().insert(DATE, get_current_HTTP_date());
	return 1;
}

int
Client::_response_last_modified_handler(exchange_t &exchange) {
	Response& response = exchange.second;
	struct stat buf;
	struct tm *date = NULL;
	char time_buf[64];

	if (stat(response.get_target_path().c_str(), &buf) != -1) {
		date = localtime(&buf.st_mtim.tv_sec);
		strftime(time_buf, sizeof(time_buf), "%a, %d %b %Y %T GMT+02", date);
		response.get_headers().insert(LAST_MODIFIED, std::string(time_buf));
	}
	return SUCCESS;
}

int
Client::_response_location_handler(exchange_t &exchange) {
	Response& response = exchange.second;
	int status_code = response.get_status_line().get_status_code();

	if (Syntax::is_redirection_code(Syntax::status_codes_tab[status_code].code_int)
		|| status_code == CREATED) {
		//TODO: need handler for POST method
	}
	return SUCCESS;
}

int
Client::_response_retry_after_handler(exchange_t &exchange) {
	Response& response = exchange.second;
	std::stringstream ss;

	if (response.get_status_line().get_status_code() == SERVICE_UNAVAILABLE) {
		ss << DELAY_RETRY_AFTER;
		response.get_headers().insert(RETRY_AFTER, ss.str());
	}
	return SUCCESS;
}

int
Client::_response_server_handler(exchange_t &exchange) {
	Response& response = exchange.second;

	response.get_headers().insert(SERVER, "webserv/1.0");
	return SUCCESS;
}

int
Client::_response_transfer_encoding_handler(exchange_t &exchange) {
	(void)exchange;
	//TODO: manage chunked format
	return SUCCESS;
}

int
Client::_response_www_authenticate_handler(exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	int status_code = response.get_status_line().get_status_code();

	if (status_code == UNAUTHORIZED && request.get_headers().key_exists(AUTHORIZATION))
		response.get_headers().insert(WWW_AUTHENTICATE, request.get_headers().get_value(AUTHORIZATION).front());
	return SUCCESS;
}
