/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:25:50 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/08 00:55:28 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

const size_t	Request::_limit_request_line_size(8000);
const size_t	Request::_limit_header_size(8000);
const size_t	Request::_limit_body_size(8000);

Request::Request(void) :
	_status(EMPTY),
	_str(),
	_request_line(),
	_headers(),
	_body() {}

Request::Request(const Request &x) :
	_status(x._status),
	_str(x._str),
	_request_line(x._request_line),
	_headers(x._headers),
	_body(x._body) {}

Request::~Request(void) {}

Request
&Request::operator=(const Request &x) {
	_status = x._status;
	_str = x._str;
	_request_line = x._request_line;
	_headers = x._headers;
	_body = x._body;
	return (*this);
}

void
Request::reset(void) {
	_status = EMPTY;
	_str.clear();
	_request_line._reset();
	_headers._reset();
	_body.clear();
}

//TODO:: ameliorer : chunked doit etre en dernier
bool
Request::_body_expected(void) const {
	if ((_headers._key_exists("Transfer-Encoding")
				&& (_headers._at("Transfer-Encoding")).find("chunked") != std::string::npos))
	{
		std::cout << "body expected because transfer encoding" << std::endl;
		return (true);
	}
	if (_headers._key_exists("Content-Length"))
	{
		std::cout << "body expected because content-len" << std::endl;
		return (true);
	}
	return (false);
	/*
	return ((_headers._key_exists("Transfer-Encoding")
				&& (_headers._at("Transfer-Encoding")).find("chunked") != std::string::npos)
			|| _headers._key_exists("Content-Length"));
			*/
}

//TODO:: ameliorer : chunked doit etre en dernier
bool
Request::_body_received(void) const {
	return ((_headers._key_exists("Transfer-Encoding")
				&& _headers._at("Transfer-Encoding").find("chunked") != std::string::npos
				&& _str.find("0\r\n\r\n") != std::string::npos)
			|| (_headers._key_exists("Content-Length")
				&& _str.size() >= static_cast<unsigned long>(std::atol(_headers._at("Content-Length").c_str()))));
}

bool
Request::_trailer_expected(void) const {
	return (false);
}

bool
Request::_trailer_received(void) const {
	return (true);
}

int
Request::_parse_request(void) {
	switch (_status) {
		case EMPTY :
			if (_str.size() > 0)
				_status = STARTED ;
			return (CONTINUE);
		case STARTED :
			if (std::string::npos != _str.find("\r\n")) {
				std::cout << "request line received" << std::endl;
				_status = REQUEST_LINE_RECEIVED;
				return (_parse_request_line());
			}
			return (CONTINUE);
		case REQUEST_LINE_RECEIVED :
			if (_str == "\r\n") {
				std::cout << "all headers received" << std::endl;
				_headers._render();
				if (_body_expected()) {
					std::cout << "body expected" << std::endl;
					_status = HEADERS_RECEIVED;
				} else {
					std::cout << "request received" << std::endl;
					_status = REQUEST_RECEIVED;
				}
				return (_parse_headers());
			}
			if (std::string::npos != _str.find("\r\n"))
				_parse_header();
			return (CONTINUE);
		case HEADERS_RECEIVED :
			if (_body_received()) {
				_body = _str;
				_str.clear();
				if (!_trailer_expected()) {
					_status = REQUEST_RECEIVED;
					return (RECEIVED);
				} else
					_status = BODY_RECEIVED;
			}
			return (CONTINUE);
		case BODY_RECEIVED :
			if (_trailer_received()) {
				_status = REQUEST_RECEIVED;
				return (RECEIVED);
			}
			return (CONTINUE);
		default :
			return (CONTINUE);
	}
}

int
Request::append(const std::string &data) {
	//TODO:: controle de l'overflow en fonction du status de la request
	_str += data;
	return (_parse_request());
}

void
Request::_parse_header(void) {
	size_t		col(0);
	if (std::string::npos == (col = _str.find_first_of(":"))) {
		_str.erase(0, _str.find("\r\n") + 2);
		return ;
	}
	std::string	header_name(_str.substr(0, col));
	std::string	header_value(_str.substr(col + 1, (_str.rfind("\r") - col - 1)));
	_headers._push(header_name, header_value);
	_str.erase(0, _str.find("\r\n") + 2);
}

//TODO:: ameliorer
int
Request::_parse_headers(void) {
	if (!_headers._key_exists("Host"))
		return (BAD_REQUEST);
	_str.erase(0, _str.find("\r\n") + 2);
	if (_status == REQUEST_RECEIVED)
		return (RECEIVED);
	else
		return (CONTINUE);
}

int
Request::_parse_request_line(void) {
	size_t		first_sp(0);
	size_t		scnd_sp(0);
	size_t		i(0);
	if (std::string::npos == (first_sp = _str.find_first_of(" ")))
		return (BAD_REQUEST);
	if (std::string::npos == (scnd_sp = _str.find_first_of(" ", first_sp + 1)))
		return (BAD_REQUEST);
	std::string	method_str(_str.substr(0, first_sp));
	while (_request_line_t::_method_tab[i]._length) {
		if (_request_line_t::_method_tab[i]._str == method_str) {
			_request_line._method = _request_line_t::_method_tab[i]._method;
			break ;
		}
		i++;
	}
	if (_request_line_t::DEFAULT == _request_line_t::_method_tab[i]._method)
		return (NOT_IMPLEMENTED);
	_request_line._request_target = _str.substr(first_sp + 1, scnd_sp - first_sp - 1);
	_request_line._http_version = _str.substr(scnd_sp + 1, (_str.rfind("\r") - scnd_sp - 1));
	std::cout << "request target : " << _request_line._request_target << "$" << std::endl;
	std::cout << "http : " << _request_line._http_version << "$" << std::endl;
	_str.erase(0, _str.find("\r\n") + 2);
	return (CONTINUE);
}

const Request::_request_line_t::_method_entry_t	Request::_request_line_t::_method_tab[] =
{
	{GET, "GET", 3},
	{HEAD, "HEAD", 4},
	{POST, "POST", 4},
	{PUT, "PUT", 3},
	{DELETE, "DELETE", 6},
	{CONNECT, "CONNECT", 7},
	{OPTIONS, "OPTIONS", 7},
	{TRACE, "TRACE", 5},
	{DEFAULT, "", 0}
};

Request::_request_line_t::_request_line_t(void) :
	_method(DEFAULT),
	_request_target(),
	_http_version() {}

Request::_request_line_t::_request_line_t(const _request_line_t &x) :
	_method(x._method),
	_request_target(x._request_target),
	_http_version(x._http_version) {}

Request::_request_line_t
&Request::_request_line_t::operator=(const _request_line_t &x) {
	_method = x._method;
	_request_target = x._request_target;
	_http_version = x._http_version;
	return (*this);
}

void
Request::_request_line_t::_reset(void) {
	_method = DEFAULT;
	_request_target.clear();
	_http_version.clear();
}

const size_t	Request::_headers_t::_headers_tab_size(30);

Request::_headers_t::_headers_t(void) :
	_tab(_headers_tab_size) {}

Request::_headers_t::_headers_t(const _headers_t &x) :
	_tab(_headers_tab_size) {
	for (size_t i(0) ; i < _headers_tab_size ; i++)
		if (x._tab[i])
			_tab[i] = new std::list<_header_entry_t>(*(x._tab[i]));
}

Request::_headers_t::~_headers_t(void) {
	_reset();
}

Request::_headers_t
&Request::_headers_t::operator=(const _headers_t &x) {
	for (size_t i(0) ; i < _headers_tab_size ; i++) {
		if (_tab[i]) {
			delete _tab[i];
			_tab[i] = 0;
		}
		if (x._tab[i])
			_tab[i] = new std::list<_header_entry_t>(*(x._tab[i]));
	}
	return (*this);
}

void
Request::_headers_t::_reset(void) {
	for (size_t i(0) ; i < _headers_tab_size ; i++)
		if (_tab[i]) {
			delete _tab[i];
			_tab[i] = 0;
		}
}

void
Request::_headers_t::_push(const std::string &header_name, const std::string &header_value) {
	_header_entry_t	new_entry;
	new_entry._key = header_name;
	new_entry._value = header_value;
	unsigned long	index(_hash(header_name.c_str()));
	if (!_tab[index])
		_tab[index] = new std::list<_header_entry_t>;
	(_tab[index])->push_front(new_entry);
}

bool
Request::_headers_t::_key_exists(const std::string &key) const {
	return (_tab[_hash(key.c_str())]);
}

std::string
&Request::_headers_t::_at(const std::string &key) {
	std::list<_header_entry_t>	*entry_list(_tab[_hash(key.c_str())]);
	if (entry_list) {
		for (std::list<_header_entry_t>::iterator it(entry_list->begin()) ; it != entry_list->end() ; it++)
			if (it->_key == key)
				return (it->_value);
	}
	throw (std::out_of_range("Request::_headers::_at : out of range exception"));
}

const std::string
&Request::_headers_t::_at(const std::string &key) const {
	std::list<_header_entry_t>	*entry_list(_tab[_hash(key.c_str())]);
	if (entry_list) {
		for (std::list<_header_entry_t>::const_iterator it(entry_list->begin()) ; it != entry_list->end() ; it++)
			if (it->_key == key)
				return (it->_value);
	}
	throw (std::out_of_range("Request::_headers::_at : out of range exception"));
}

unsigned long
Request::_headers_t::_hash(const char *buffer) const {
	unsigned long	hash(5381);
	char			c;
	size_t			i(0);
	while ((c = buffer[i++]))
		hash = ((hash << 5) + hash) + c;
	return (hash % _headers_tab_size);
}

void
Request::_headers_t::_render(void) const {
	for (size_t i(0) ; i < _headers_tab_size ; i++) {
		if (_tab[i]) {
			for (std::list<_header_entry_t>::iterator it(_tab[i]->begin()) ; it != _tab[i]->end() ; it++) {
				std::cout << "KEY : " << it->_key << "$" << std::endl;
				std::cout << "VALUE : " << it->_value << "$" << std::endl;
			}
		}
	}
}
