/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:25:50 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/20 11:03:22 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "Client.hpp"

#include "Request.hpp"
#include "Client.hpp"

int Request::_indexes = 0;

Request::Request(void) :
	AHTTPMessage(),
	_id(_indexes++),
	_status(START),
	_request_line(),
	_virtual_server(),
	_location(),
	_client_addr() {}

Request::Request(const Client &client) :
	AHTTPMessage(),
	_id(_indexes++),
	_status(START),
	_request_line(),
	_virtual_server(client._virtual_servers.front()),
	_location(&(client._virtual_servers.front()->get_locations().back())),
	_client_addr(client._addr) {}

Request::Request(const Request &x) :
	AHTTPMessage(x),
	_id(_indexes++),
	_status(x._status),
	_request_line(x._request_line),
	_virtual_server(x._virtual_server),
	_location(x._location),
	_client_addr(x._client_addr) {}

Request::~Request(void) {}

Request
&Request::operator=(const Request &x) {
	AHTTPMessage::operator=(x);
	_id = _indexes++;
	_status = x._status;
	_request_line = x._request_line;
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

const VirtualServer* Request::get_virtual_server() const {
	return _virtual_server;
}

const struct sockaddr
&Request::get_client_addr(void) const {
	return (_client_addr);
}

void
Request::set_status(request_status_t status) {
	_status = status;
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

const Location*
Request::get_location() const {
	return _location;
}

void
Request::set_location(const Location* location) {
	_location = location;
}

ByteArray&
Request::get_raw(void) {
	return _raw;
}

const ByteArray&
Request::get_raw(void) const {
	return _raw;
}

void
Request::set_raw(const ByteArray& raw) {
	_raw = raw;
}

char*
Request::get_ip_addr() const {
	struct sockaddr not_const = _client_addr;
	struct sockaddr_in *client_addr_cast;

	client_addr_cast = reinterpret_cast<struct sockaddr_in*>(&not_const);
	return (inet_ntoa(client_addr_cast->sin_addr));
}

std::string
Request::get_ident() const {
	std::stringstream ss;

	ss << get_ip_addr() << "[request no:" << _id << "]";
	return (ss.str());
}

