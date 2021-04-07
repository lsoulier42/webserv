/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:25:50 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/07 19:05:21 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

const size_t	Request::_limit_request_line_size(8000);
const size_t	Request::_limit_header_size(8000);

Request::Request(void) :
	_status(EMPTY),
	_str(),
	_request_line() {}

Request::Request(const Request &x) :
	_status(x._status),
	_str(x._str),
	_request_line(x._request_line) {}

Request::~Request(void) {}

Request
&Request::operator=(const Request &x) {
	_status = x._status;
	_str = x._str;
	_request_line = x._request_line;
	return (*this);
}

void
Request::reset(void) {
	_status = EMPTY;
	_str.clear();
	_request_line._reset();
}

int
Request::_parse_request(void) {
	switch (_status) {
		case EMPTY :
			if (_str.size() > 0)
				_status = STARTED ;
			return (SUCCESS);
		case STARTED :
			if (std::string::npos != _str.find("\r\n")) {
				std::cout << "request line received" << std::endl;
				_status = REQUEST_LINE_RECEIVED;
				return (_parse_request_line());
			}
			return (SUCCESS);
		case REQUEST_LINE_RECEIVED :
			if (_str == "\r\n") {
				std::cout << "all headers received" << std::endl;
				_status = HEADERS_RECEIVED;
				return (SUCCESS);
			}
			if (std::string::npos != _str.find("\r\n"))
				_parse_header();
			return (SUCCESS);
		case HEADERS_RECEIVED :
			//TODO:: fonction qui dit si la requete est complete, en fonction de content-length et chunked
			return (SUCCESS);
		default :
			return (SUCCESS);
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
	std::cout << header_name << std::endl;
	std::cout << _hash(header_name.c_str()) % 18 << std::endl;
	_str.erase(0, _str.find("\r\n") + 2);
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
	_request_line._http_version = _str.substr(scnd_sp + 1);
	_str.erase(0, _str.find("\r\n") + 2);
	return (SUCCESS);
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

Request::_headers_

unsigned long
Request::_headers_t::_hash(const char *buffer) const {
	unsigned long	hash(5381);
	int				c;
	size_t			i(0);
	while ((c = header[i++]))
		hash = ((hash << 5) + hash) + c;
	return (hash);
}
