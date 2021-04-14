/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/14 08:01:23 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

const size_t	Client::_buffer_size(900);

Client::Client(void) :
	_sd(),
	_fd(),
	_addr(),
	_socket_len(),
	_configs(),
	_input_str(),
	_output_str(),
	_exchanges(),
	_closing(false) {}

Client::Client(int sd, struct sockaddr addr, socklen_t socket_len, const std::list<const Config*> &configs) :
	_sd(sd),
	_fd(0),
	_addr(addr),
	_socket_len(socket_len),
	_configs(configs),
	_input_str(),
	_output_str(),
	_exchanges(),
	_closing(false) {}

Client::Client(const Client &x) :
	_sd(x._sd),
	_fd(x._fd),
	_addr(x._addr),
	_socket_len(x._socket_len),
	_configs(x._configs),
	_input_str(x._input_str),
	_output_str(x._output_str),
	_exchanges(x._exchanges),
	_closing(x._closing) {}

Client::~Client(void) {}

Client
&Client::operator=(const Client &x) {
	_fd = x._fd;
	_exchanges = x._exchanges;
	_input_str = x._input_str;
	_output_str = x._output_str;
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
			std::cout << "error during reading the socket." << std::endl;
		return (FAILURE);
	}
	buffer[ret] = '\0';
	_input_str += (std::string(buffer));
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
				|| (request.get_headers().key_exists(Syntax::headers_tab[CONTENT_LENGTH].name)
					&& _input_str.size() >= static_cast<unsigned long>(std::atol(request.get_headers().get_value(Syntax::headers_tab[CONTENT_LENGTH].name).front().c_str())))));
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
	if (request.get_headers().key_exists(Syntax::headers_tab[TRANSFER_ENCODING].name)) {
		const std::list<std::string>& transfer_encoding_values = request.get_headers().get_value(Syntax::headers_tab[TRANSFER_ENCODING].name);
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
		|| request.get_headers().key_exists(Syntax::headers_tab[CONTENT_LENGTH].name));
}

bool
Client::_trailer_expected(const Request &request) const {
	return ((request.get_headers().key_exists("Trailer")));
}

void
Client::_input_str_parsing(void) {
	while (!_closing && !_input_str.empty()) {
		if (_exchanges.empty() || _exchanges.back().first.get_status() == Request::REQUEST_RECEIVED)
			_exchanges.push_back(std::make_pair(Request(), Response()));
		exchange_t	&current_exchange(_exchanges.back());
		if (_request_line_received(current_exchange.first) && SUCCESS != _collect_request_line_elements(current_exchange))
			return ;
		while (_header_received(current_exchange.first))
			_collect_header(current_exchange);
		if (_headers_received(current_exchange.first) && SUCCESS != _check_headers(current_exchange))
			return ;
		if (_body_received(current_exchange.first))
			_collect_body(current_exchange);
		while (_trailer_received(current_exchange.first))
			_collect_header(current_exchange);
		if (_trailers_received(current_exchange.first) && SUCCESS != _check_trailer(current_exchange))
			return ;
		if (current_exchange.first.get_status() != Request::REQUEST_RECEIVED)
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
	request.get_request_line().set_http_version(_input_str.substr(scnd_sp + 1, (end_rl - scnd_sp - 1)));
	_input_str.erase(0, end_rl + 2);
	if (DEFAULT_METHOD == request.get_request_line().get_method()) {
		_failure(exchange, NOT_IMPLEMENTED);
		return (FAILURE);
	}
	exchange.first.set_status(Request::REQUEST_LINE_RECEIVED);
	return (SUCCESS);
}

int
Client::_collect_header(exchange_t &exchange) {
	Request							&request(exchange.first);
	size_t							col(0);
	size_t							end_header(_input_str.find("\r\n"));
	AHTTPMessage::Headers::header_t	current_header;

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
	if (_headers_parsers(exchange.first) == FAILURE) {
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
	else if (request.get_headers().key_exists(Syntax::headers_tab[CONTENT_LENGTH].name))
		body_length = static_cast<unsigned long>(std::atol(request.get_headers().get_value(Syntax::headers_tab[CONTENT_LENGTH].name).front().c_str()));
	request.set_body(_input_str.substr(0, body_length));
	_input_str.erase(0, body_length);
	if (_trailer_expected(request))
		request.set_status(Request::BODY_RECEIVED);
	else
		request.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

/*
 * Headers parsers
 */


/* _extract_virtual_server :
 * get the right configuration of virtual host
 * based on header host and server_name
 *
 */

void
Client::_extract_virtual_server(Request &current_request, const std::string& host_value) {
	std::string host_requested;
	std::vector<std::string> host_elements;
	std::list<std::string> server_names;
	const Config* virtual_server = NULL;

	for(std::list<const Config*>::const_iterator it = _configs.begin();
		it != _configs.end() && !virtual_server; it++) {
		server_names = (*it)->getServerNames();
		for (std::list<std::string>::const_iterator cit = server_names.begin();
			cit != server_names.end(); cit++) {
			if (*cit == host_value) {
				virtual_server = *it;
				break;
			}
		}
	}
	virtual_server = virtual_server ? virtual_server : _configs.front();
	current_request.set_virtual_server(virtual_server);
}

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
Client::_header_accept_charset_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(Syntax::headers_tab[ACCEPT_CHARSETS].name);
	std::list<std::string> charsets_list = _parse_coma_q_factor(unparsed_header_value);

	request.get_headers().set_value(Syntax::headers_tab[ACCEPT_CHARSETS].name, charsets_list);
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
	if (compounds.size() == 1) {
		region = compounds[2];
		if (!Syntax::str_is_alnum(region) || region.size() < 2)
			return false;
	}
	return true;
}

int
Client::_header_accept_language_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(Syntax::headers_tab[ACCEPT_LANGUAGE].name);
	std::list<std::string> languages_list = _parse_coma_q_factor(unparsed_header_value);

	for(std::list<std::string>::const_iterator it = languages_list.begin(); it != languages_list.end(); it++) {
		if (!_is_valid_language_tag(*it))
			return FAILURE;
	}
	request.get_headers().set_value(Syntax::headers_tab[ACCEPT_LANGUAGE].name, languages_list);
	return SUCCESS;
}

int
Client::_header_authorization_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(Syntax::headers_tab[AUTHORIZATION].name);
	std::vector<std::string> compounds = Syntax::split(unparsed_header_value, " ");
	std::list<std::string> definitive_value;

	if (compounds.size() != 2)
		return FAILURE;
	definitive_value.push_back(compounds[0]); // authorization type
	definitive_value.push_back(compounds[1]); // authorization credentials
	request.get_headers().set_value(Syntax::headers_tab[AUTHORIZATION].name, definitive_value);
	return SUCCESS;
}

int
Client::_header_content_length_parser(Request &request) {
	std::string content_length_str = request.get_headers().get_unparsed_value(Syntax::headers_tab[CONTENT_LENGTH].name);
	std::list<std::string> definitive_value;

	if(request.get_headers().key_exists(Syntax::headers_tab[TRANSFER_ENCODING].name))
		return FAILURE;
	if (!Syntax::str_is_num(content_length_str))
		return FAILURE;
	definitive_value.push_back(content_length_str);
	request.get_headers().set_value(Syntax::headers_tab[CONTENT_LENGTH].name, definitive_value);
	return (SUCCESS);
}

std::string
Client::_build_effective_request_URI(const Request::RequestLine& requestLine,
	const std::string& header_host_value) {
	std::string http("http://");
	std::string port;
	size_t colon_pos;
	method_t method = requestLine.get_method();
	std::string target = requestLine.get_request_target();

	colon_pos = header_host_value.find(':');
	if (colon_pos != std::string::npos)
		port = header_host_value.substr(colon_pos + 1);
	if (port == "443")
		http = "https://";
	if (header_host_value.find(http) != std::string::npos)
		http.clear();
	if (method == CONNECT || method == OPTIONS || target.find(header_host_value) != std::string::npos)
		return http + target;
	if (target == "*")
		target.clear();
	return http + header_host_value + target;
}

int
Client::_header_content_type_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(Syntax::headers_tab[CONTENT_TYPE].name);
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
	request.get_headers().set_value(Syntax::headers_tab[CONTENT_TYPE].name, content_type_list);
	return SUCCESS;
}

/* _date_handler :
 * accept the three HTTP dates formats, in order in the std::string array :
 * - real example : Sun, 06 Nov 1994 08:49:37 GMT
 * - obsolete 1 (RFC 850) example : Sunday, 06-Nov-94 08:49:37 GMT
 * - obsolete 2 (ANSI C) example : Sun Nov  6 08:49:37 1994
 * make list of 2 strings elements :
 * - unparsed date
 * - format to use for strptime
 */

int
Client::_header_date_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(Syntax::headers_tab[DATE].name);
	char* strptime_ret;
	size_t i = 0;
	std::string HTTP_date_fmt[3] = {"%a, %d %b %Y %T GMT", "%A, %d-%b-%y %T GMT", "%a %b  %d %T %Y"};
	struct tm timeval;
	std::list<std::string> definitive_value;

	while(i < 3) {
		strptime_ret = strptime(unparsed_header_value.c_str(), HTTP_date_fmt[i].c_str(), &timeval);
		if (!strptime_ret)
			break;
		i++;
	}
	if (strptime_ret != NULL)
		return FAILURE;
	definitive_value.push_back(unparsed_header_value);
	definitive_value.push_back(HTTP_date_fmt[i]);
	request.get_headers().set_value(Syntax::headers_tab[DATE].name, definitive_value);
	return SUCCESS;
}

int
Client::_header_host_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(Syntax::headers_tab[HOST].name);
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
	request.get_headers().set_value(Syntax::headers_tab[HOST].name, definitive_value);
	this->_extract_virtual_server(request, compounds[0]);
	return (SUCCESS);
}

int
Client::_header_referer_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(Syntax::headers_tab[REFERER].name);
	URI_form_t uri_form = Syntax::get_URI_form(unparsed_header_value);

	if (uri_form != ABSOLUTE_URI && uri_form != PARTIAL_URI)
		return FAILURE;
	request.get_headers().set_value(Syntax::headers_tab[REFERER].name,
		std::list<std::string>(1, unparsed_header_value));
	return SUCCESS;
}

int
Client::_header_transfer_encoding_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(Syntax::headers_tab[TRANSFER_ENCODING].name);
	std::list<std::string> encoding_types_list = _parse_coma_q_factor(unparsed_header_value);

	for(std::list<std::string>::iterator it = encoding_types_list.begin();
		it != encoding_types_list.end(); it++)
		if (!Syntax::is_accepted_value(*it, Syntax::encoding_types_tab, TOTAL_ENCODING_TYPES))
			return FAILURE;
	request.get_headers().set_value(Syntax::headers_tab[TRANSFER_ENCODING].name, encoding_types_list);
	return SUCCESS;
}

int
Client::_header_user_agent_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(Syntax::headers_tab[USER_AGENT].name);
	std::vector<std::string> compounds = Syntax::split(unparsed_header_value, " ");
	std::list<std::string> definitive_value;

	for(size_t i = 0; i < compounds.size(); i++)
		definitive_value.push_back(compounds[i]);
	request.get_headers().set_value(Syntax::headers_tab[USER_AGENT].name, definitive_value);
	return SUCCESS;
}

void Client::_send_debug_str(const std::string& str) const {
	std::string to_send = str + "\n";
	size_t size = to_send.size();

	send(_sd, to_send.c_str(), size, 0);
}

int
Client::_headers_parsers(Request &request) {
	const AHTTPMessage::Headers& headers = request.get_headers();

	int (Client::*handler_functions[])(Request &request) = {&Client::_header_accept_charset_parser,
		&Client::_header_accept_language_parser, &Client::_header_authorization_parser,
		&Client::_header_content_length_parser, &Client::_header_content_type_parser, &Client::_header_date_parser, NULL,
		&Client::_header_referer_parser, &Client::_header_transfer_encoding_parser, &Client::_header_user_agent_parser,
	};
	if (!request.get_headers().key_exists(Syntax::headers_tab[HOST].name)
			|| _header_host_parser(request) == FAILURE)
		return FAILURE;
	for(size_t i = 0; i < TOTAL_REQUEST_HEADERS; i++) {
		if (Syntax::request_headers_tab[i].header_index != HOST
			&& headers.key_exists(Syntax::request_headers_tab[i].name)) {
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
		return (_open_file_to_read(request.get_virtual_server()->getErrorPagePath()));
	if (request.get_request_line().get_method() == GET)
		return (_process_GET(exchange));
	return (FAILURE);
}

int
Client::_process_GET(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);

	std::string	path(_build_path_ressource(request));
	struct stat	buf;
	if (-1 == stat(path.c_str(), &buf)) {
		response.get_status_line().set_status_code(NOT_FOUND);
		return (_open_file_to_read(request.get_virtual_server()->getErrorPagePath()));
	}
	return (_open_file_to_read(path));
}

std::string
Client::_build_path_ressource(Request &request) {
	std::string			request_target(request.get_request_line().get_request_target());
	std::string			absolute_path(request_target.substr(0, request_target.find("?")));
	std::list<Location>	locations(request.get_virtual_server()->getLocations());
	std::string			location_root(request.get_virtual_server()->getRoot());
	std::string			location_path("/");

	for (std::list<Location>::iterator it(locations.begin()) ; it != locations.end() ; it++) {
		if (!absolute_path.compare(0, (it->getPath()).size(), it->getPath())) {
			location_root = it->getRoot();
			location_path = it->getPath();
			break ;
		}
	}
	absolute_path.erase(0, location_path.size());
	location_root += "/";
	absolute_path.insert(0, location_root);
	return (absolute_path);
}

int
Client::_open_file_to_read(const std::string &path) {
	_fd = open(path.c_str(), O_RDONLY);
	return (SUCCESS);
}

int
Client::read_file(void) {
	exchange_t	&exchange(_exchanges.front());
	Response	&response(exchange.second);
	char		buffer[_buffer_size + 1];
	int			ret;

	ret = read(_fd, buffer, _buffer_size);
	if (ret == 0) {
		close(_fd);
		_fd = 0;
		response.get_status_line().set_status_code(OK);
		return (_build_output_str(exchange));
	}
	buffer[ret] = '\0';
	response.set_body(response.get_body() + std::string(buffer));
	return (SUCCESS);
}

int
Client::_build_output_str(exchange_t &exchange) {
	Response	&response(exchange.second);

	_output_str.clear();
	_output_str += response.get_status_line().get_http_version();
	_output_str += " ";
	_output_str += Syntax::status_codes_tab[exchange.second.get_status_line().get_status_code()].code_str;
	_output_str += " ";
	_output_str += Syntax::status_codes_tab[exchange.second.get_status_line().get_status_code()].reason_phrase;
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
