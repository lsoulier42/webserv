/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/11 04:27:46 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

const size_t	Client::_buffer_size(900);

Client::Client(void) :
	_sd(),
	_addr(),
	_socket_len(),
	_configs(),
	_input_str(),
	_output_str(),
	_exchanges(),
	_closing(false) {}

Client::Client(int sd, struct sockaddr addr, socklen_t socket_len, const std::list<const Config*> &configs) :
	_sd(sd),
	_addr(addr),
	_socket_len(socket_len),
	_configs(configs),
	_input_str(),
	_output_str(),
	_exchanges(),
	_closing(false) {}

Client::Client(const Client &x) :
	_sd(x._sd),
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
Client::read_socket(void) {
	char	buffer[_buffer_size + 1];
	int		ret;
	if (0 >= (ret = read(_sd, buffer, _buffer_size))) {
		if (0 == ret)
			std::cout << "the client closed the connection." << std::endl;
		else
			std::cout << "error during reading." << std::endl;
		return (FAILURE);
	}
	buffer[ret] = '\0';
	_input_str += (std::string(buffer));
	_input_str_parsing();
	if (!_exchanges.empty() && _exchanges.front().first.get_status() == Request::REQUEST_RECEIVED)
		return (_process(_exchanges.front()));
	return (SUCCESS);
}

bool
Client::_request_line_received(const Request &current_request) const {
	return (current_request.get_status() == Request::START && std::string::npos != _input_str.find("\r\n"));
}

bool
Client::_header_received(const Request &current_request) const {
	return (current_request.get_status() == Request::REQUEST_LINE_RECEIVED
			&& !_headers_received(current_request)
			&& std::string::npos != _input_str.find("\r\n"));
}

bool
Client::_headers_received(const Request &current_request) const {
	return (current_request.get_status() == Request::REQUEST_LINE_RECEIVED
			&& !_input_str.compare(0, 2, "\r\n"));
}

bool
Client::_body_received(const Request &current_request) const {
	return (current_request.get_status() == Request::HEADERS_RECEIVED
			&& ((_transfer_encoding_chunked(current_request)
					&& _input_str.find("0\r\n\r\n") != std::string::npos)
				|| (current_request.get_headers().key_exists(Syntax::method_tab[CONTENT_LENGTH].name)
					&& _input_str.size() >= static_cast<unsigned long>(std::atol(current_request.get_headers().get_value(Syntax::method_tab[CONTENT_LENGTH].name).front().c_str())))));
}

bool
Client::_trailer_received(const Request &current_request) const {
	return (current_request.get_status() == Request::BODY_RECEIVED
			&& !_trailers_received(current_request)
			&& std::string::npos != _input_str.find("\r\n"));
}

//TODO:: pas du tout comme ca qu'on repere la fin des trailers, provisoire, pour test
bool
Client::_trailers_received(const Request &current_request) const {
	return (current_request.get_status() == Request::BODY_RECEIVED
			&& !_input_str.compare(0, 2, "\r\n"));
}

bool
Client::_transfer_encoding_chunked(const Request &current_request) {
	std::list<std::string> transfer_encoding_values;

	if (current_request.get_headers().key_exists(Syntax::method_tab[TRANSFER_ENCODING].name)) {
		transfer_encoding_values = current_request.get_headers().get_value(Syntax::method_tab[TRANSFER_ENCODING].name);
		if (std::find(transfer_encoding_values.begin(), transfer_encoding_values.end(),
			Syntax::encoding_types_tab[CHUNKED].name) != transfer_encoding_values.end())
			return true;
	}
	return false;
}

bool
Client::_body_expected(const Request &current_request) const {
	return (_transfer_encoding_chunked(current_request)
		|| current_request.get_headers().key_exists(Syntax::method_tab[CONTENT_LENGTH].name));
}

bool
Client::_trailer_expected(const Request &current_request) const {
	return ((current_request.get_headers().key_exists("Trailer")));
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
		if (_trailers_received(current_exchange.first))
			current_exchange.first.set_status(Request::REQUEST_RECEIVED);
		if (current_exchange.first.get_status() != Request::REQUEST_RECEIVED)
			return ;
	}
}

int
Client::_collect_request_line_elements(exchange_t &exchange) {
	size_t		first_sp(0);
	size_t		scnd_sp(0);
	size_t		end_rl(_input_str.find("\r\n"));
	if (std::string::npos == (first_sp = _input_str.find_first_of(' '))
			|| std::string::npos == (scnd_sp = _input_str.find_first_of(' ', first_sp + 1))) {
		_input_str.erase(0, end_rl + 2);
		exchange.first.set_status(Request::REQUEST_RECEIVED);
		exchange.second.get_status_line().set_status_code(BAD_REQUEST);
		_closing = true;
		return (FAILURE);
	}
	exchange.first.get_request_line().set_method(_input_str.substr(0, first_sp));
	exchange.first.get_request_line().set_request_target(_input_str.substr(first_sp + 1, scnd_sp - first_sp - 1));
	exchange.first.get_request_line().set_http_version(_input_str.substr(scnd_sp + 1, (end_rl - scnd_sp - 1)));
	_input_str.erase(0, end_rl + 2);
	if (DEFAULT_METHOD == exchange.first.get_request_line().get_method()) {
		exchange.first.set_status(Request::REQUEST_RECEIVED);
		exchange.second.get_status_line().set_status_code(NOT_IMPLEMENTED);
		_closing = true;
		return (FAILURE);
	}
	exchange.first.set_status(Request::REQUEST_LINE_RECEIVED);
	return (SUCCESS);
}

int
Client::_collect_header(exchange_t &exchange) {
	size_t							col(0);
	size_t							end_header(_input_str.find("\r\n"));
	AHTTPMessage::Headers::header_t	current_header;

	if (std::string::npos != (col = _input_str.find_first_of(':'))) {
		current_header.name = _input_str.substr(0, col);
		current_header.unparsed_value = _input_str.substr(col + 1, (end_header - col - 1));
		exchange.first.get_headers().insert(current_header);
	}
	_input_str.erase(0, end_header + 2);
	return (SUCCESS);
}

int
Client::_check_headers(exchange_t &exchange) {
	_input_str.erase(0, _input_str.find("\r\n") + 2);
	if (!_headers_handlers(exchange)) {
		_closing = true;
		exchange.first.set_status(Request::REQUEST_RECEIVED);
		return (FAILURE);
	}
	if (_body_expected(exchange.first))
		exchange.first.set_status(Request::HEADERS_RECEIVED);
	else
		exchange.first.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

int
Client::_collect_body(exchange_t &exchange) {
	size_t	body_length(0);
	Request& current_request = exchange.first;

	if (_transfer_encoding_chunked(current_request))
		body_length = _input_str.find("0\r\n\r\n") + 5;
	else if (current_request.get_headers().key_exists("Content-Length"))
		body_length = static_cast<unsigned long>(std::atol(current_request.get_headers().get_value("Content-Length").front().c_str()));
	current_request.set_body(_input_str.substr(0, body_length));
	_input_str.erase(0, body_length);
	if (_trailer_expected(current_request))
		current_request.set_status(Request::BODY_RECEIVED);
	else
		current_request.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

//TODO:: remplir l'objet Response. return SUCESS si on reussit a aller jusqu'au bout, PENDING si on est bloque par la lecture d'un fichier
int
Client::_fill_response(exchange_t &exchange) {
	bool	need_syscall_read(false);
	AHTTPMessage::Headers::header_t fake_response;

	fake_response.name = "Truc";
	fake_response.unparsed_value = "Bidule chose chouette";
	fake_response.value = std::list<std::string>(1, "bidule");

	exchange.first.render();
	exchange.second.get_status_line().set_status_code(OK);
	exchange.second.get_status_line().set_http_version("HTTP/1.1");
	exchange.second.get_headers().insert(fake_response);
	exchange.second.set_body("Bip bop boup\r\n");
	if (need_syscall_read)
		return (_open_file_to_read());
	return (_build_output_str(exchange));
}

//on ouvre le fichier
//on actualise la valeur d'un attribut _fd
//on ajoute le fd aux fd monitores par select
int
Client::_open_file_to_read(void) {
	return (SUCCESS);
}


//fonction appelee lorsque select nous autorise a read le fd
//on lit le fichier et lorsqu'on l'a termine
//on peut passer a la suite du process de la requete
int
Client::_read_file(void) {
	bool	end_file(true);
	exchange_t	&exchange(_exchanges.front());
	if (end_file)
		return (_build_output_str(exchange));
	return (SUCCESS);
}

//TODO:: transformer l'objet Response en une char* a envoyer avec write sur le socket dans la fonction _write_socket()
int
Client::_build_output_str(exchange_t &exchange) {
	_output_str.clear();
	_output_str += exchange.second.get_body();
	return (_write_socket(exchange));
}

//TODO:: envoyer la char* _output_str sur le socket
int
Client::_write_socket(exchange_t &exchange) {
	(void)exchange;
	write(_sd, _output_str.c_str(), _output_str.size());
	//ici, return FAILURE si besoin
	_exchanges.pop_front();
	if (!_exchanges.empty() && _exchanges.front().first.get_status() == Request::REQUEST_RECEIVED)
		return (_process(_exchanges.front()));
	return (SUCCESS);
}

int
Client::_process(exchange_t &exchange) {
	exchange.first.set_status(Request::REQUEST_PROCESSED);
	return (_fill_response(exchange));
}
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
			cit != server_names.end(); it++) {
			if (*cit == host_value) {
				virtual_server = *it;
				break;
			}
		}
	}
	virtual_server = virtual_server ? virtual_server : _configs.front();
	current_request.set_virtual_server(virtual_server);
}

std::list<std::string>
Client::_parse_coma_q_factor(const std::string& unparsed_value) {
	std::list<std::string> new_list;
	std::multimap<float, std::string> value_weight;
	std::vector<std::string> elements_split;

	elements_split = Syntax::split(unparsed_value, ", ");
	value_weight = _split_header_weight(elements_split);
	for(std::multimap<float, std::string>::const_reverse_iterator rit = value_weight.rbegin();
		rit != value_weight.rend(); rit++)
		new_list.push_back(rit->second);
	return new_list;
}

std::multimap<float, std::string>
Client::_split_header_weight(const std::vector<std::string>& elements_split) {
	std::multimap<float, std::string> value_weight;
	std::vector<std::string> weight_split;
	std::string effective_value;
	std::string q_weight;
	float effective_weight;

	for(std::vector<std::string>::const_iterator it = elements_split.begin();
		it != elements_split.end(); it++) {
		weight_split = Syntax::split(*it, ";");
		effective_value = !weight_split.empty() && weight_split.size() != 1 ?
				weight_split[0] : *it;
		effective_weight = !weight_split.empty() && weight_split.size() != 1 ?
				std::strtol(q_weight.substr(2).c_str(), NULL, 10) : 1;
		value_weight.insert(std::make_pair(effective_weight, effective_value));
	}
	return value_weight;
}

/* _header_accept_charset_handler:
 * 4 most common accepted : utf-8, iso-8859-1, unicode-1-1, US-ASCII
 * asterisk form accepted
 * case sensitive
 * make a list of accepted charsets
 *
 */

int
Client::_header_accept_charset_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	Response::StatusLine status_line = exchange.second.get_status_line();
	std::list<std::string> charsets_list = _parse_coma_q_factor(unparsed_header_value);

	for(std::list<std::string>::const_iterator it = charsets_list.begin(); it != charsets_list.end(); it++) {
		if (!Syntax::is_accepted_value<Syntax::accepted_charsets_entry_t>(*it,
			Syntax::charsets_tab, TOTAL_ACCEPTED_CHARSETS)) {
			status_line.set_status_code(NOT_ACCEPTABLE);
			return 0;
		}
	}
	exchange.first.get_headers().set_value(Syntax::headers_tab[ACCEPT_CHARSETS].name, charsets_list);
	return 1;
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
		&& (language.size() < 2 || language.size() > 3))
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

/* _header_accept_language_handler:
 * only check if language tag is legit
 * need to check the accepted languages of the representation, else error 406 - NOT ACCEPTABLE
 * make a list of strings of accepted languages
 *
 */

int
Client::_header_accept_language_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	Response::StatusLine status_line = exchange.second.get_status_line();
	std::list<std::string> languages_list = _parse_coma_q_factor(unparsed_header_value);

	for(std::list<std::string>::const_iterator it = languages_list.begin(); it != languages_list.end(); it++) {
		if (!_is_valid_language_tag(*it)) {
			status_line.set_status_code(BAD_REQUEST);
			return 0;
		}
	}
	exchange.first.get_headers().set_value(Syntax::headers_tab[ACCEPT_LANGUAGE].name, languages_list);
	return 1;
}

/* _header_allow_handler:
 * not a request header
 *
 */

int
Client::_header_allow_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	(void)exchange;
	(void)unparsed_header_value;
	return 1;
}

/* _header_authorization_handler:
 * 1 check :
 * - has 2 tokens (size and credentials
 * case sensitive
 * make a list of two strings elements : type and credentials
 *
 */

int
Client::_header_authorization_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	Response::StatusLine status_line = exchange.second.get_status_line();
	std::vector<std::string> compounds = Syntax::split(unparsed_header_value, " ");
	std::list<std::string> definitive_value;

	if (compounds.size() != 2) {
		status_line.set_status_code(BAD_REQUEST);
		return 0;
	}
	definitive_value.push_back(compounds[0]);
	definitive_value.push_back(compounds[1]);
	exchange.first.get_headers().set_value(Syntax::headers_tab[AUTHORIZATION].name, definitive_value);
	return 1;
}

/* _header_content_length_handler (request header for POST and PUT only):
 * checks :
 * - error if transfer encoding header is already present
 * - error if not numerical :
 * -- error if multiple values because of whitespaces
 * -- error if negative value because of negative sign
 * - error if length > max_client_body_size from virtual server config
 * make a list of string of one element : content_length value
 *
 */

int
Client::_header_content_length_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	std::string content_length_str = unparsed_header_value;
	unsigned long content_length;
	Response::StatusLine status_line = exchange.second.get_status_line();
	std::list<std::string> definitive_value;

	if(exchange.first.get_headers().key_exists(Syntax::headers_tab[TRANSFER_ENCODING].name)){
		status_line.set_status_code(BAD_REQUEST);
		return 0;
	}
	if (!Syntax::str_is_num(content_length_str)) {
		status_line.set_status_code(BAD_REQUEST);
		return 0;
	}
	content_length = std::strtol(content_length_str.c_str(), NULL, 10);
	if (content_length > exchange.first.get_virtual_server()->getClientMaxBodySize()) {
		status_line.set_status_code(PAYLOAD_TOO_LARGE);
		return (0);
	}
	definitive_value.push_back(unparsed_header_value);
	exchange.first.get_headers().set_value(Syntax::headers_tab[CONTENT_LENGTH].name, definitive_value);
	return (1);
}

/* _header_content_language_handler (response header):
 * not a request header
 *
 */

int
Client::_header_content_language_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	(void)exchange;
	(void)unparsed_header_value;
	return 1;
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

/* _header_content_location_handler (response header):
 * not a request header
 *
 */

int
Client::_header_content_location_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	(void)exchange;
	(void)unparsed_header_value;
	return 1;
}

std::list<std::string>
Client::_parse_content_type_header_value(const std::string& unparsed_value) {
	std::string mime_type;
	std::string parameter;
	size_t semicolon_pos;
	std::list<std::string> value_list;

	semicolon_pos = unparsed_value.find_first_of(';');
	mime_type = unparsed_value.substr(0, semicolon_pos);
	value_list.push_back(mime_type);
	if (semicolon_pos != std::string::npos) {
		parameter = unparsed_value.substr(semicolon_pos + 1);
		value_list.push_back(parameter);
	}
	return value_list;
}

/* _content_type_handler (for POST and PUT method only) :
 * supported media types : text/plain, text/html, text/css, text/csv, image/bmp
 * image/gif, image/jpeg, image/png, application/octet-stream (default), application/javascript,
 * application/pdf, application/xml
 *
 */

int
Client::_header_content_type_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	Response::StatusLine status_line = exchange.second.get_status_line();
	std::list<std::string> content_type_list = _parse_content_type_header_value(unparsed_header_value);
	std::string mime_type = content_type_list.front();

	if (!Syntax::is_accepted_value<Syntax::mime_type_entry_t>(mime_type,
		Syntax::mime_types_tab, TOTAL_MIME_TYPES)) {
		status_line.set_status_code(UNSUPPORTED_MEDIA_TYPE);
		return 0;
	}
	exchange.first.get_headers().set_value(Syntax::headers_tab[CONTENT_TYPE].name, content_type_list);
	return 1;
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
Client::_header_date_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	Response::StatusLine status_line = exchange.second.get_status_line();
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
	if (strptime_ret != NULL) {
		status_line.set_status_code(BAD_REQUEST);
		return 0;
	}
	definitive_value.push_back(unparsed_header_value);
	definitive_value.push_back(HTTP_date_fmt[i]);
	exchange.first.get_headers().set_value(Syntax::headers_tab[DATE].name, definitive_value);
	return 1;
}

/* _header_host_handler:
 * 2 checks :
 * - invalid if whitespaces (INVALID_URI_FORM)
 * - invalid if multiple semicolon in format
 * make list of 1 or 2 strings elements :
 * - host name
 * - port number (if exist)
 * use _extract_virtual_server to choose the right configuration for the request
 *
 */

int
Client::_header_host_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	Response::StatusLine status_line = exchange.second.get_status_line();
	std::vector<std::string> compounds;
	std::list<std::string> definitive_value;

	if (Syntax::get_URI_form(unparsed_header_value) == INVALID_URI_FORM) {
		status_line.set_status_code(BAD_REQUEST);
		return 0;
	}
	compounds = Syntax::split(unparsed_header_value, ":");
	if (compounds.size() > 2) {
		status_line.set_status_code(BAD_REQUEST);
		return 0;
	}
	definitive_value.push_back(compounds[0]);
	this->_extract_virtual_server(exchange.first, compounds[0]);
	if (compounds.size() == 2)
		definitive_value.push_back(compounds[1]);
	exchange.first.get_headers().set_value(Syntax::headers_tab[HOST].name, definitive_value);
	return 1;
}

/* _last_modified_handler (response header) : same as _date_handler
 * not a request header
 *
 */

int
Client::_header_last_modified_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	(void)exchange;
	(void)unparsed_header_value;
	return 1;
}

/* _header_location_handler:
 * not a request header
 *
 */

int
Client::_header_location_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	(void)exchange;
	(void)unparsed_header_value;
	return 1;
}

/* _header_referer_handler:
 * 1 check : if value is an absolute or partial URI
 * make a list of one string element : the URI
 *
 */

int
Client::_header_referer_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	Response::StatusLine status_line = exchange.second.get_status_line();
	URI_form_t uri_form = Syntax::get_URI_form(unparsed_header_value);

	if (uri_form != ABSOLUTE_URI && uri_form != PARTIAL_URI) {
		status_line.set_status_code(BAD_REQUEST);
		return 0;
	}
	exchange.first.get_headers().set_value(Syntax::headers_tab[REFERER].name,
		std::list<std::string>(1, unparsed_header_value));
	return 1;
}

/* _header_retry_after_handler:
 * not a request headerd
 *
 */

int
Client::_header_retry_after_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	(void)exchange;
	(void)unparsed_header_value;
	return 1;
}

/* _header_server_handler:
 * not a request header
 *
 */

int
Client::_header_server_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	(void)exchange;
	(void)unparsed_header_value;
	return 1;
}

/* _header_transfer_encoding_handler:
 * check if one of the following types :
 * chunked, compress, deflate, gzip, identity
 * make a list of strings of encoding types
 *
 */


int
Client::_header_transfer_encoding_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	Response::StatusLine status_line = exchange.second.get_status_line();
	std::list<std::string> encoding_types_list = _parse_coma_q_factor(unparsed_header_value);

	for(std::list<std::string>::iterator it = encoding_types_list.begin();
		it != encoding_types_list.end(); it++) {
		if (!Syntax::is_accepted_value(*it, Syntax::encoding_types_tab, TOTAL_ENCODING_TYPES)) {
			status_line.set_status_code(BAD_REQUEST);
			return 0;
		}
	}
	exchange.first.get_headers().set_value(Syntax::method_tab[TRANSFER_ENCODING].name, encoding_types_list);
	return 1;
}

/* _header_user_agent_handler
 * no check
 * make list of space separated products (preserving the /product_version)
 *
 */

int
Client::_header_user_agent_handler(exchange_t& exchange, const std::string &unparsed_header_value) {
	std::vector<std::string> compounds = Syntax::split(unparsed_header_value, " ");
	std::list<std::string> definitive_value;

	for(size_t i = 0; i < compounds.size(); i++)
		definitive_value.push_back(compounds[i]);
	exchange.first.get_headers().set_value(Syntax::headers_tab[USER_AGENT].name, definitive_value);
	return 1;
}

/* _header_www_authenticate:
 * not a request header
 *
 */

int
Client::_header_www_authenticate(exchange_t& exchange, const std::string &unparsed_header_value) {
	(void)exchange;
	(void)unparsed_header_value;
	return 1;
}

int
Client::_headers_handlers(exchange_t& exchange) {
	const AHTTPMessage::Headers& headers = exchange.first.get_headers();
	Response::StatusLine status_line = exchange.second.get_status_line();
	int (Client::*handler_functions[])(exchange_t&, const std::string&) = {&Client::_header_accept_charset_handler,
		&Client::_header_accept_language_handler, &Client::_header_allow_handler, &Client::_header_authorization_handler,
		&Client::_header_content_length_handler, &Client::_header_content_language_handler, &Client::_header_content_location_handler,
		&Client::_header_content_type_handler, &Client::_header_date_handler, NULL, &Client::_header_last_modified_handler,
		&Client::_header_location_handler, &Client::_header_referer_handler, &Client::_header_retry_after_handler,
		&Client::_header_server_handler, &Client::_header_transfer_encoding_handler, &Client::_header_user_agent_handler,
		&Client::_header_www_authenticate};

	if (!exchange.first.get_headers().key_exists(Syntax::headers_tab[HOST].name)) {
		exchange.second.get_status_line().set_status_code(BAD_REQUEST);
		return 0;
	}

	if (!_header_host_handler(exchange, headers.get_unparsed_value(Syntax::headers_tab[HOST].name)))
		return 0;
	for(size_t i = 0; i < TOTAL_HEADER_NAMES; i++) {
		if (i != HOST && headers.key_exists(Syntax::headers_tab[i].name)) {
			if (!(this->*handler_functions[i])(exchange, headers.get_unparsed_value(Syntax::headers_tab[i].name)))
				return 0;
		}
	}
	return 1;
}
