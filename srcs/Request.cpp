/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:25:50 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/10 16:23:08 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request(void) :
	AHTTPMessage(),
	_status(START),
	_limit_body_size(),
	_request_line() {}

Request::Request(const Request &x) :
	AHTTPMessage(x),
	_status(x._status),
	_limit_body_size(x._limit_body_size),
	_request_line(x._request_line) {}

Request::~Request(void) {}

Request
&Request::operator=(const Request &x) {
	AHTTPMessage::operator=(x);
	_status = x._status;
	_limit_body_size = x._limit_body_size;
	_request_line = x._request_line;
	return (*this);
}

Request::request_status_t
Request::get_status(void) const {
	return (_status);
}

size_t
Request::get_limit_body_size() const {
	return _limit_body_size;
}

Request::RequestLine
&Request::get_request_line(void) {
	return (_request_line);
}

const Request::RequestLine
&Request::get_request_line(void) const {
	return (_request_line);
}

void
Request::set_status(request_status_t status) {
	_status = status;
}

void
Request::set_limit_body_size(size_t size) {
	_limit_body_size = size;
}

//TODO:: char* plutot que std::string pour la reconnaissance du pattern CRLF (RFC 7230 p.20)
/*
int
Request::append(const std::string &data) {
	_str += data;
	return (_request_parsing());
}
*/

void
Request::reset(void) {
	AHTTPMessage::reset();
	_status = START;
	_limit_body_size = 0;
	_request_line.reset();
}

void
Request::render(void) const {
	std::cout << "---REQUEST LINE---" << std::endl;
	_request_line.render();
	std::cout << std::endl << "---HEADERS---" << std::endl;
	get_headers().render();
	std::cout << std::endl << "---BODY---" << std::endl;
	std::cout << get_body() << "$" << std::endl << std::endl;
}

/*
bool
Request::_request_line_received(void) const {
	return (_status == START && std::string::npos != _str.find("\r\n"));
}

bool
Request::_header_received(void) const {
	return (_status == REQUEST_LINE_RECEIVED
			&& !_headers_received()
			&& std::string::npos != _str.find("\r\n"));
}

bool
Request::_headers_received(void) const {
	return (_status == REQUEST_LINE_RECEIVED
			&& !_str.compare(0, 2, "\r\n"));
}

//TODO:: ameliorer : chunked devrait etre le dernier element de Transfer-Encoding
bool
Request::_body_expected(void) const {
	return ((get_headers().key_exists("Transfer-Encoding")
				&& (get_headers().get_value("Transfer-Encoding")).find("chunked") != std::string::npos)
			|| get_headers().key_exists("Content-Length"));
}

//TODO:: ameliorer : chunked devrait etre le dernier element de Transfer-Encoding
bool
Request::_body_received(void) const {
	return (_status == HEADERS_RECEIVED
			&& ((get_headers().key_exists("Transfer-Encoding")
						&& get_headers().get_value("Transfer-Encoding").find("chunked") != std::string::npos
						&& _str.find("0\r\n\r\n") != std::string::npos)
				|| (get_headers().key_exists("Content-Length")
						&& _str.size() >= static_cast<unsigned long>(std::atol(get_headers().get_value("Content-Length").c_str())))));
}

//TODO:: tout
bool
Request::_trailer_expected(void) const {
	return (false);
}

//TODO:: tout
bool
Request::_trailer_received(void) const {
	return (true);
}

int
Request::_request_parsing(void) {
	int	ret(CONTINUE_PARSING);
	while (ret == CONTINUE_PARSING) {
		switch (_status) {
			case START :
				ret = _started_request_parsing();
				break ;
			case REQUEST_LINE_RECEIVED :
				ret = _request_line_received_parsing();
				break ;
			case HEADERS_RECEIVED :
				ret = _headers_received_parsing();
				break ;
			case BODY_RECEIVED :
				ret = _body_received_parsing();
				break ;
			case REQUEST_RECEIVED :
				ret = RECEIVED;
				break ;
		}
	}
	return (ret);
}

int
Request::_started_request_parsing(void) {
	if (_request_line_received()) {
		_status = REQUEST_LINE_RECEIVED;
		return (_collect_request_line_elements());
	}
	return (CONTINUE_READING);
}

int
Request::_request_line_received_parsing(void) {
	if (_headers_received()) {
		if (_body_expected())
			_status = HEADERS_RECEIVED;
		else
			_status = REQUEST_RECEIVED;
		return (_check_headers());
	}
	if (_header_received())
		return (_collect_header());
	return (CONTINUE_READING);
}

int
Request::_headers_received_parsing(void) {
	if (_body_received()) {
		if (_trailer_expected())
			_status = BODY_RECEIVED;
		else
			_status = REQUEST_RECEIVED;
		return (_collect_body());
	}
	return (CONTINUE_READING);
}

int
Request::_body_received_parsing(void) {
	if (_trailer_received()) {
		_status = REQUEST_RECEIVED;
		return (RECEIVED);
	}
	return (CONTINUE_READING);
}

int
Request::_collect_request_line_elements(void) {
	size_t		first_sp(0);
	size_t		scnd_sp(0);
	size_t		end_rl(_str.find("\r\n"));
	if (std::string::npos == (first_sp = _str.find_first_of(" "))
			|| std::string::npos == (scnd_sp = _str.find_first_of(" ", first_sp + 1))) {
		_str.erase(0, end_rl + 2);
		return (BAD_REQUEST);
	}
	_request_line.set_method(_str.substr(0, first_sp));
	_request_line.set_request_target(_str.substr(first_sp + 1, scnd_sp - first_sp - 1));
	_request_line.set_http_version(_str.substr(scnd_sp + 1, (end_rl - scnd_sp - 1)));
	_str.erase(0, end_rl + 2);
	if (DEFAULT_METHOD == _request_line.get_method())
		return (NOT_IMPLEMENTED);
	if (_header_received() || _headers_received())
		return (CONTINUE_PARSING);
	return (CONTINUE_READING);
}

int
Request::_collect_header(void) {
	size_t		col(0);
	size_t		end_header(_str.find("\r\n"));
	if (std::string::npos != (col = _str.find_first_of(":"))) {
		std::string	header_name(_str.substr(0, col));
		std::string	header_value(_str.substr(col + 1, (end_header - col - 1)));
		get_headers().insert(header_name, header_value);
	}
	_str.erase(0, end_header + 2);
	if (_header_received() || _headers_received())
		return (CONTINUE_PARSING);
	return (CONTINUE_READING);
}

//TODO:: ameliorer
int
Request::_check_headers(void) {
	_str.erase(0, _str.find("\r\n") + 2);
	if (!get_headers().key_exists("Host"))
		return (BAD_REQUEST);
	if (_status == REQUEST_RECEIVED)
		return (RECEIVED);
	if (_body_received())
		return (CONTINUE_PARSING);
	return (CONTINUE_READING);
}

//TODO:: s'assurer que chunked est le dernier element du champ Transfer-Encoding
//TODO:: decoder le body si besoin
int
Request::_collect_body(void) {
	size_t	body_length(0);
	if (get_headers().key_exists("Transfer-Encoding")
			&& get_headers().get_value("Transfer-Encoding").find("chunked") != std::string::npos) {
		body_length = _str.find("0\r\n\r\n") + 5;
	} else if (get_headers().key_exists("Content-Length"))
		body_length = static_cast<unsigned long>(std::atol(get_headers().get_value("Content-Length").c_str()));
	set_body(_str.substr(0, body_length));
	_str.erase(0, body_length);
	if (_status == REQUEST_RECEIVED)
		return (RECEIVED);
	if (_trailer_received())
		return (CONTINUE_PARSING);
	return (CONTINUE_READING);
}
*/

/*
 * REQUEST_LINE_T RELATED FUNCTIONS
 */

Request::RequestLine::RequestLine(void) :
	AStartLine(),
	_method(DEFAULT_METHOD),
	_request_target() {}

Request::RequestLine::RequestLine(const RequestLine &x) :
	AStartLine(x),
	_method(x._method),
	_request_target(x._request_target) {}

Request::RequestLine::~RequestLine(void) {}

Request::RequestLine
&Request::RequestLine::operator=(const RequestLine &x) {
	AStartLine::operator=(x);
	_method = x._method;
	_request_target = x._request_target;
	return (*this);
}

method_t
Request::RequestLine::get_method(void) const {
	return (_method);
}

const std::string
&Request::RequestLine::get_request_target(void) const {
	return (_request_target);
}

void
Request::RequestLine::set_method(const std::string &method_str) {
	size_t	i(0);
	while (i < DEFAULT_METHOD) {
		if (Syntax::method_tab[i].name == method_str) {
			_method = Syntax::method_tab[i].method_index;
			return ;
		}
		i++;
	}
	_method = DEFAULT_METHOD;
}

void
Request::RequestLine::set_request_target(const std::string &request_target) {
	_request_target = request_target;
}

void
Request::RequestLine::reset(void) {
	AStartLine::reset();
	_method = DEFAULT_METHOD;
	_request_target.clear();
}

void
Request::RequestLine::render(void) const {
	size_t	i(0);
	while (i < DEFAULT_METHOD) {
		if (Syntax::method_tab[i].method_index == _method) {
			std::cout << "METHOD : " << Syntax::method_tab[i].name << std::endl;
			break ;
		}
		i++;
	}
	std::cout << "REQUEST TARGET : " << _request_target << "$" << std::endl;
	std::cout << "HTTP VERSION : " << get_http_version() << "$" << std::endl;
}
