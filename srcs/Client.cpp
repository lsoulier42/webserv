/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/08 19:05:29 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

const size_t	Client::_buffer_size(900);

Client::Client(void) :
	_sd(0) {}

Client::Client(int sd) :
	_sd(sd) {}

Client::Client(const Client &x) :
	_sd(x._sd) {}

Client::~Client(void) {}

Client
&Client::operator=(const Client &x) {
	(void)x;
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
	ret = _request.append(std::string(buffer));
	if (CONTINUE == ret)
		return (SUCCESS);
	else
		return (_process(ret));
}

int
Client::_process(int status) {
	std::cout << "-----> REPONSE : " << status << std::endl << std::endl;
	_request.render();
	_request.reset();
	return (SUCCESS);
}
