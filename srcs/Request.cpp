/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:25:50 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/05 13:47:29 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

const size_t			Request::_limit_request_line_size(1025);

						Request::Request(void) :
							_socket(0) {}

						Request::Request(int socket) :
							_socket(socket) {}

						Request::Request(const Request &x) :
							_socket(x._socket) {}

						Request::~Request(void) {}

Request					&Request::operator=(const Request &x) {
	(void)x;
	return (*this);
}

void					Request::_read_request_line(void) {
	int		ret(0);
	char	c(0);
	char	c_prev(0);
	while ((ret = read(_socket, &c, 1)) > 0
			&& !(c == '\n' && c_prev == '\r'))
	{
		std::cout << c;
		c_prev = c;
	}
	std::cout << std::endl;
}

void					Request::process(void) {
	_read_request_line();
}
