/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/11 04:03:12 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

const size_t	Client::_buffer_size(3);

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
	exchange.first.set_status(Request::REQUEST_LINE_RECEIVED);
	exchange.first.get_request_line().set_method(_input_str.substr(0, first_sp));
	exchange.first.get_request_line().set_request_target(_input_str.substr(first_sp + 1, scnd_sp - first_sp - 1));
	exchange.first.get_request_line().set_http_version(_input_str.substr(scnd_sp + 1, (end_rl - scnd_sp - 1)));
	_input_str.erase(0, end_rl + 2);
	if (DEFAULT_METHOD == exchange.first.get_request_line().get_method()) {
		exchange.second.get_status_line().set_status_code(NOT_IMPLEMENTED);
		_closing = true;
		return (FAILURE);
	}
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
	if (_body_expected(exchange.first))
		exchange.first.set_status(Request::HEADERS_RECEIVED);
	else
		exchange.first.set_status(Request::REQUEST_RECEIVED);
	_input_str.erase(0, _input_str.find("\r\n") + 2);
	if (!exchange.first.get_headers().key_exists("Host")) {
		exchange.second.get_status_line().set_status_code(BAD_REQUEST);
		_closing = true;
		return (FAILURE);
	}
	return (SUCCESS);
}

int
Client::_collect_body(exchange_t &exchange) {
	if (_trailer_expected(exchange.first))
		exchange.first.set_status(Request::BODY_RECEIVED);
	else
		exchange.first.set_status(Request::REQUEST_RECEIVED);
	size_t	body_length(0);
	if (exchange.first.get_headers().key_exists("Transfer-Encoding")
			&& exchange.first.get_headers().get_value("Transfer-Encoding").find("chunked") != std::string::npos) {
		body_length = _input_str.find("0\r\n\r\n") + 5;
	} else if (exchange.first.get_headers().key_exists("Content-Length"))
		body_length = static_cast<unsigned long>(std::atol(exchange.first.get_headers().get_value("Content-Length").c_str()));
	exchange.first.set_body(_input_str.substr(0, body_length));
	_input_str.erase(0, body_length);
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
