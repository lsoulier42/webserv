/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:25:50 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/07 14:21:57 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

const size_t					Request::_limit_request_line_size(8000);
const size_t					Request::_limit_header_size(8000);

const Request::_method_entry	Request::_method_tab[] =
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

								Request::Request(void) :
									_status(EMPTY),
									_str() {}

								Request::Request(const Request &x) :
									_status(x._status),
									_str(x._str) {}

								Request::~Request(void) {}

Request							&Request::operator=(const Request &x) {
	_status = x._status;
	_str = x._str;
	return (*this);
}

void							Request::reset(void) {
	_status = EMPTY;
	_str.clear();
}

int								Request::_parse_request(void) {
	switch (_status) {
		case EMPTY :
			if (_str.size() > 0)
				_status = STARTED ;
			break ;
		case STARTED :
			if (std::string::npos != _str.find("\r\n")) {
				std::cout << "request line received" << std::endl;
				_status = REQUEST_LINE_RECEIVED;
				return (_parse_request_line());
			}
			break ;
		case REQUEST_LINE_RECEIVED :
			if (std::string::npos != _str.find("\r\n\r\n")) {
				std::cout << "headers received" << std::endl;
				_status = HEADERS_RECEIVED;
			}
			break ;
		case HEADERS_RECEIVED :
			//TODO:: fonction qui dit si la requete est complete, en fonction de content-length et chunked
			break ;
		default :
			break ;
	}
	return (SUCCESS);
}

int								Request::append(const std::string &data) {
	//TODO:: controle de l'overflow en fonction du status de la request
	_str += data;
	return (_parse_request());
}

int								Request::_parse_request_line(void) {
	size_t		first_sp(0);
	size_t		scnd_sp(0);
	size_t		i(0);
	if (std::string::npos == (first_sp = _str.find_first_of(" ")))
		return (BAD_REQUEST);
	if (std::string::npos == (scnd_sp = _str.find_first_of(" ", first_sp + 1)))
		return (BAD_REQUEST);
	std::string	method_str(_str.substr(0, first_sp));
	while (_method_tab[i]._length) {
		if (_method_tab[i]._str == method_str) {
			_request_line._method = _method_tab[i]._method;
			break ;
		}
		i++;
	}
	if (DEFAULT == _method_tab[i]._method)
		return (NOT_IMPLEMENTED);
	_request_line._request_target = _str.substr(first_sp + 1, scnd_sp - first_sp - 1);
	_request_line._http_version = _str.substr(scnd_sp + 1);
	return (SUCCESS);
}
