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
			&& ((current_request.get_headers().key_exists("Transfer-Encoding")
						&& current_request.get_headers().get_value("Transfer-Encoding").find("chunked") != std::string::npos
						&& _input_str.find("0\r\n\r\n") != std::string::npos)
				|| (current_request.get_headers().key_exists("Content-Length")
						&& _input_str.size() >= static_cast<unsigned long>(std::atol(current_request.get_headers().get_value("Content-Length").c_str())))));
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
Client::_body_expected(const Request &current_request) const {
	return ((current_request.get_headers().key_exists("Transfer-Encoding")
				&& (current_request.get_headers().get_value("Transfer-Encoding")).find("chunked") != std::string::npos)
			|| current_request.get_headers().key_exists("Content-Length"));
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
	if (std::string::npos == (first_sp = _input_str.find_first_of(" "))
			|| std::string::npos == (scnd_sp = _input_str.find_first_of(" ", first_sp + 1))) {
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
	size_t		col(0);
	size_t		end_header(_input_str.find("\r\n"));
	if (std::string::npos != (col = _input_str.find_first_of(":"))) {
		std::string	header_name(_input_str.substr(0, col));
		std::string	header_value(_input_str.substr(col + 1, (end_header - col - 1)));
		exchange.first.get_headers().insert(std::make_pair(header_name, header_value));
	}
	_input_str.erase(0, end_header + 2);
	return (SUCCESS);
}

int
Client::_check_headers(exchange_t &exchange) {
	_input_str.erase(0, _input_str.find("\r\n") + 2);
	if (!exchange.first.get_headers().key_exists("Host")) {
		exchange.first.set_status(Request::REQUEST_RECEIVED);
		exchange.second.get_status_line().set_status_code(BAD_REQUEST);
		_closing = true;
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
	if (exchange.first.get_headers().key_exists("Transfer-Encoding")
			&& exchange.first.get_headers().get_value("Transfer-Encoding").find("chunked") != std::string::npos) {
		body_length = _input_str.find("0\r\n\r\n") + 5;
	} else if (exchange.first.get_headers().key_exists("Content-Length"))
		body_length = static_cast<unsigned long>(std::atol(exchange.first.get_headers().get_value("Content-Length").c_str()));
	exchange.first.set_body(_input_str.substr(0, body_length));
	_input_str.erase(0, body_length);
	if (_trailer_expected(exchange.first))
		exchange.first.set_status(Request::BODY_RECEIVED);
	else
		exchange.first.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

//TODO:: remplir l'objet Response. return SUCESS si on reussit a aller jusqu'au bout, PENDING si on est bloque par la lecture d'un fichier
int
Client::_fill_response(exchange_t &exchange) {
	bool	need_syscall_read(false);
	exchange.first.render();
	exchange.second.get_status_line().set_status_code(OK);
	exchange.second.get_status_line().set_http_version("HTTP/1.1");
	exchange.second.get_headers().insert(std::make_pair("Truc", "Bidule chose chouette"));
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
/* This function get the right configuration of virtual host
 * based on header host and server_name
 * http://nginx.org/en/docs/http/request_processing.html
 *
 */

void
Client::_extract_virtual_server(exchange_t& exchange) {
	std::string host_requested;
	std::vector<std::string> host_elements;
	std::string effective_host;
	std::list<std::string> server_names;
	const Config* virtual_server = NULL;

	if (exchange.first.get_headers().key_exists("Host")) {
		host_requested = exchange.first.get_headers().get_value("Host");
		host_elements = Syntax::split(host_requested, ":");
		effective_host = host_elements[0];
	}
	for(std::list<const Config*>::const_iterator it = _configs.begin();
		it != _configs.end(); it++) {
		server_names = (*it)->getServerNames();
		for (std::list<std::string>::const_iterator cit = server_names.begin();
			cit != server_names.end(); it++) {
			if (*cit == effective_host) {
				virtual_server = *it;
				break;
			}
		}
		if (virtual_server)
			break;
	}
	virtual_server = virtual_server ? virtual_server : _configs.front();
	exchange.first.set_virtual_server(virtual_server);
}

int Client::_accept_charset_handler(exchange_t& exchange, const std::list<std::string>& charsets_list) {
	Response::StatusLine status_line = exchange.second.get_status_line();

	for(std::list<std::string>::const_iterator it = charsets_list.begin(); it != charsets_list.end(); it++) {
		if (!Syntax::is_accepted_value<Syntax::accepted_charsets_entry_t>(*it,
			Syntax::charsets_tab, TOTAL_ACCEPTED_CHARSETS)) {
			status_line.set_status_code(NOT_ACCEPTABLE);
			return 0;
		}
		//TODO: handler
	}
	return 1;
}

int Client::_accept_language_handler(exchange_t& exchange, const std::list<std::string>& languages_list) {
	Response::StatusLine status_line = exchange.second.get_status_line();

	for(std::list<std::string>::const_iterator it = languages_list.begin(); it != languages_list.end(); it++) {
		if (!Syntax::is_accepted_value<Syntax::accepted_languages_entry_t>(*it,
				Syntax::languages_tab, TOTAL_ACCEPTED_CHARSETS)) {
			status_line.set_status_code(NOT_ACCEPTABLE);
			return 0;
		}
		//TODO: handler
	}
	return 1;
}

bool Client::_is_allowed_method(std::list<std::string> allowed_methods, const std::string& method) {
	for (std::list<std::string>::iterator it = allowed_methods.begin(); it != allowed_methods.end(); it++) {
		if (*it == method)
			return true;
	}
	return false;
}

int Client::_allow_handler(exchange_t& exchange, const std::list<std::string>& methods_list) {
	Response::StatusLine status_line = exchange.second.get_status_line();
	std::list<std::string> allowed_methods;

	for(std::list<std::string>::const_iterator it = methods_list.begin(); it != methods_list.end(); it++) {
		if (!Syntax::is_accepted_value<Syntax::method_tab_entry_t>(*it,
			Syntax::method_tab, DEFAULT_METHOD)) {
			status_line.set_status_code(BAD_REQUEST);
			return 0;
		}
		allowed_methods = exchange.first.get_virtual_server()->getMethods();
		if (!_is_allowed_method(allowed_methods, *it)) {
			status_line.set_status_code(METHOD_NOT_ALLOWED);
			return 0;
		}
		//TODO: handler
	}
	return 1;
}

bool Client::_check_credentials(const std::string& credential) {
	(void)credential;
	//TODO: find out how authorization fields are made
	return true;
}

int Client::_authorization_handler(exchange_t& exchange, const std::list<std::string>& credentials_list) {
	std::string credential;
	Response::StatusLine status_line = exchange.second.get_status_line();

	if (credentials_list.size() != 1) {
		status_line.set_status_code(BAD_REQUEST);
		return 0;
	}
	credential = credentials_list.front();
	if (!_check_credentials(credential)) {
		status_line.set_status_code(UNAUTHORIZED);
		return 0;
	}
	//TODO: handler
	return 1;
}

bool Client::_transfer_encoding_is_set(const exchange_t& exchange) {
	const AHTTPMessage::Headers& headers = exchange.first.get_headers();

	return headers.key_exists(Syntax::headers_tab[TRANSFER_ENCODING].name);
}

int Client::_content_length_handler(exchange_t& exchange, const std::list<std::string>& content_length_list) {
	std::string content_length_str;
	unsigned long content_length;
	Response::StatusLine status_line = exchange.second.get_status_line();

	if(_transfer_encoding_is_set(exchange)) {
		status_line.set_status_code(BAD_REQUEST);
		return 0;
	}
	if (content_length_list.size() != 1) {
		status_line.set_status_code(BAD_REQUEST);
		return 0;
	}
	content_length_str = content_length_list.front();
	if (!Syntax::is_num(content_length_str.c_str())) {
		status_line.set_status_code(BAD_REQUEST);
		return 0;
	}
	content_length = std::strtol(content_length_str.c_str(), NULL, 10);
	if (content_length > exchange.first.get_virtual_server()->getClientMaxBodySize()) {
		status_line.set_status_code(PAYLOAD_TOO_LARGE);
		return (0);
	}
	//TODO: handler
	return (1);
}

int Client::_headers_handler(exchange_t& exchange) {
	const AHTTPMessage::Headers& headers = exchange.first.get_headers();
	std::list<std::string> value_list;
	Response::StatusLine status_line = exchange.second.get_status_line();
	bool header_complete[TOTAL_HEADER_NAMES] = {false};
	int (Client::*handler_functions[])(exchange_t&, const std::list<std::string>&) = {&Client::_accept_charset_handler,
		&Client::_accept_language_handler, &Client::_allow_handler, &Client::_authorization_handler, &Client::_content_length_handler};
	/* TODO::
	 * check champs multiples++++++++++++++++++
	 *
	 */
	for(size_t i = 0; i < TOTAL_HEADER_NAMES; i++) {
		if (headers.key_exists(Syntax::headers_tab[i].name)) {
			value_list = Syntax::parse_header_value(headers.get_value(Syntax::headers_tab[i].name));
			if (value_list.empty()) {
				status_line.set_status_code(BAD_REQUEST);
				_closing = true;
				exchange.first.set_status(Request::REQUEST_RECEIVED);
				return 0;
			}
			if (!(this->*handler_functions[i])(exchange, value_list)) {
				_closing = true;
				exchange.first.set_status(Request::REQUEST_RECEIVED);
				return 0;
			}
			header_complete[i] = true;
		}
	}
	return 1;
}