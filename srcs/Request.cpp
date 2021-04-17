/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:25:50 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/16 01:40:53 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request(void) :
	AHTTPMessage(),
	_status(START),
	_request_line(),
	_compromising(false),
	_virtual_server(),
	_location() {}

Request::Request(const VirtualServer *virtual_server) :
	AHTTPMessage(),
	_status(START),
	_request_line(),
	_compromising(false),
	_virtual_server(virtual_server),
	_location(&(virtual_server->get_locations().back())) {}

Request::Request(const Request &x) :
	AHTTPMessage(x),
	_status(x._status),
	_request_line(x._request_line),
	_compromising(x._compromising),
	_virtual_server(x._virtual_server),
	_location(x._location) {}

Request::~Request(void) {}

Request
&Request::operator=(const Request &x) {
	AHTTPMessage::operator=(x);
	_status = x._status;
	_request_line = x._request_line;
	_compromising = x._compromising;
	_virtual_server = x._virtual_server;
	_location = x._location;
	return (*this);
}

Request::request_status_t
Request::get_status(void) const {
	return (_status);
}

Request::RequestLine
&Request::get_request_line(void) {
	return (_request_line);
}

const Request::RequestLine
&Request::get_request_line(void) const {
	return (_request_line);
}

bool
Request::get_compromising(void) const {
	return (_compromising);
}

const VirtualServer* Request::get_virtual_server() const {
	return _virtual_server;
}

void
Request::set_status(request_status_t status) {
	_status = status;
}

void
Request::set_compromising(bool compromising) {
	_compromising = compromising;
}

void Request::set_virtual_server(const VirtualServer* virtual_server) {
	_virtual_server = virtual_server;
}

void
Request::reset(void) {
	AHTTPMessage::reset();
	_status = START;
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

const Location*
Request::get_location() const {
	return _location;
}

void
Request::set_location(const Location* location) {
	_location = location;
}
