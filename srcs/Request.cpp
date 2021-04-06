/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:25:50 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/05 20:19:00 by mdereuse         ###   ########.fr       */
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
									_socket(0) {}

								Request::Request(int socket) :
									_socket(socket) {}

								Request::Request(const Request &x) :
									_socket(x._socket) {}

								Request::~Request(void) {}

Request							&Request::operator=(const Request &x) {
	(void)x;
	return (*this);
}

int								Request::_read_request_line(char *buffer) {
	int		ret(0);
	char	c(0);
	char	c_prev(0);
	size_t	i(0);
	while (i < _limit_request_line_size
			&& (ret = read(_socket, &c, 1)) > 0
			&& !(c == '\n' && c_prev == '\r')) {
		buffer[i++] = c;
		c_prev = c;
	}
	if (i == _limit_request_line_size)
		return (BAD_REQUEST);
	if (ret <= 0)
		return (INTERRUPT);
	buffer[i - 1] = '\0';
	return (SUCCESS);
}

//TODO:: verification prealable concernant la longueur max des methodes implementees
int								Request::_parse_request_line(char *buffer) {
	std::string	str(buffer);
	size_t		first_sp(0);
	size_t		scnd_sp(0);
	size_t		i(0);
	if (std::string::npos == (first_sp = str.find_first_of(" ")))
		return (BAD_REQUEST);
	if (std::string::npos == (scnd_sp = str.find_first_of(" ", first_sp + 1)))
		return (BAD_REQUEST);
	std::string	method_str(str.substr(0, first_sp));
	while (_method_tab[i]._length) {
		if (_method_tab[i]._str == method_str) {
			_request_line._method = _method_tab[i]._method;
			break ;
		}
		i++;
	}
	if (DEFAULT == _method_tab[i]._method)
		return (NOT_IMPLEMENTED);
	_request_line._request_target = str.substr(first_sp + 1, scnd_sp - first_sp - 1);
	_request_line._http_version = str.substr(scnd_sp + 1);
	return (SUCCESS);
}

int								Request::_process_headers(void) {
	while (1) {
		int		ret(0);
		size_t	i(0);
		char	c(0);
		char	c_prev(0);
		char	buffer[_limit_header_size + 1];
		while ((ret = read(_socket, &c, 1)) > 0
				&& !(c == '\n' && c_prev == '\r')) {
			buffer[i++] = c;
			c_prev = c;
		}
		if (ret <= 0)
			return (INTERRUPT);
		buffer[i - 1] = '\0';
		if (!buffer[0])
			break ;
		std::cout << std::string(buffer) << std::endl;
	}
	return (SUCCESS);
}

int								Request::_process_request_line(void) {
	int		ret(0);
	char	buf[_limit_request_line_size + 1];
	if (SUCCESS != (ret = _read_request_line(buf))
			|| SUCCESS != (ret = _parse_request_line(buf)))
		return (ret);
	return (SUCCESS);
}

//TODO:: gerer le retour de process_request_line
void							Request::process(void) {
	char	buffer[3000];
	size_t	ret;
	ret = recv(_socket, buffer, 3000, 0);
	buffer[ret] = '\0';
	std::cout << std::string(buffer) << std::endl;
	std::cout << "end" << std::endl;
	/*
	_process_request_line();
	std::cout << "request : " << _request_line._request_target;
	std::cout << "$ version : " << _request_line._http_version;
	std::cout << "$" << std::endl;
	_process_headers();
	std::cout << "TRUC" << std::endl;
	*/
}
