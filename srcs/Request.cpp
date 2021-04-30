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
#include "WebServer.hpp"

int Request::_indexes = 0;

Request::Request(void) :
	AHTTPMessage(),
	_id(_indexes++),
	_status(START),
	_request_line(),
	_virtual_server(),
	_location(),
	_client_addr(),
	_chunked_body(false),
	_tmp_fd(0),
	_tmp_filename(),
	_body_size_expected(0),
	_body_size_received(0),
	_body_received(false),
	_tmp_file_size(0),
	_body_written(false) {}

Request::Request(const Client &client) :
	AHTTPMessage(),
	_id(_indexes++),
	_status(START),
	_request_line(),
	_virtual_server(client._virtual_servers.front()),
	_location(&(client._virtual_servers.front()->get_locations().back())),
	_client_addr(client._addr),
	_chunked_body(false),
	_tmp_fd(0),
	_tmp_filename(),
	_body_size_expected(0),
	_body_size_received(0),
	_body_received(false),
	_tmp_file_size(0),
	_body_written(false) {}

Request::Request(const Request &x) :
	AHTTPMessage(x),
	_id(x._id),
	_status(x._status),
	_request_line(x._request_line),
	_virtual_server(x._virtual_server),
	_location(x._location),
	_client_addr(x._client_addr),
	_chunked_body(x._chunked_body),
	_tmp_fd(x._tmp_fd),
	_tmp_filename(x._tmp_filename),
	_body_size_expected(x._body_size_expected),
	_body_size_received(x._body_size_received),
	_body_received(x._body_received),
	_tmp_file_size(x._tmp_file_size),
	_body_written(x._body_written) {}

Request::~Request(void) {}

Request
&Request::operator=(const Request &x) {
	AHTTPMessage::operator=(x);
	if (this != &x) {
		_id = x._id;
		_status = x._status;
		_request_line = x._request_line;
		_virtual_server = x._virtual_server;
		_location = x._location;
		_chunked_body = x._chunked_body;
		_tmp_fd = x._tmp_fd;
		_tmp_filename = x._tmp_filename;
		_body_size_expected = x._body_size_expected;
		_body_size_received = x._body_size_received;
		_body_received = x._body_received;
		_tmp_file_size = x._tmp_file_size;
		_body_written = x._body_written;
	}
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

bool
Request::is_chunked() const {
	return _chunked_body;
}

void
Request::set_chunked() {
	_chunked_body = true;
}

int
Request::get_id() const {
	return (_id);
}

size_t&
Request::get_body_size_received(void) {
	return (_body_size_received);
}

size_t
Request::get_body_size_received(void) const {
	return (_body_size_received);
}

bool
Request::body_is_received() const {
	return (_body_received);
}

void
Request::set_body_received() {
	_body_received = true;
}

size_t
Request::get_body_size_expected(void) const {
	return (_body_size_expected);
}

void
Request::set_body_size_expected(size_t body_size) {
	_body_size_expected = body_size;
}

std::string
Request::get_tmp_filename(void) const {
	return (_tmp_filename);
}

void
Request::set_tmp_filename(const std::string& tmp_filename) {
	_tmp_filename = tmp_filename;
}

int
Request::get_tmp_fd(void) const {
	return (_tmp_fd);
}

void
Request::set_tmp_fd(int fd) {
	_tmp_fd = fd;
}

size_t&
Request::get_tmp_file_size() {
	return (_tmp_file_size);
}

size_t
Request::get_tmp_file_size() const {
	return (_tmp_file_size);
}

bool
Request::body_is_writen() const {
	return (_body_written);
}

void
Request::set_body_written() {
	_body_written = true;
}

int
Request::write_tmp_file(void) {
	ByteArray	&body = get_body();
	size_t		body_size = body.size();
	size_t		to_write = std::min(body_size, WebServer::write_buffer_size);
	ssize_t 	write_return;

	if (_tmp_fd == 0)
		return (SUCCESS);
	write_return = write(_tmp_fd, body.c_str(), to_write);
	if (write_return < 0) {
		DEBUG_COUT("Error during writing target resource: " << std::strerror(errno) << "(" << get_ident() << ")");
		close(_tmp_fd);
		_tmp_fd = 0;
		return (FAILURE);
	}
	body.pop_front(write_return);
	_tmp_file_size += write_return;
	if (body_is_received() && _tmp_file_size == _body_size_received) {
		_body_written = true;
		close(_tmp_fd);
		_tmp_fd = 0;
	}
	return (SUCCESS);
}
