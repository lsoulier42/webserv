/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/07 14:20:59 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

const size_t					Client::_buffer_size(3);

								Client::Client(void) :
									_sd(0) {}

								Client::Client(int sd) :
									_sd(sd) {}

								Client::Client(const Client &x) :
									_sd(x._sd) {}

								Client::~Client(void) {}

Client							&Client::operator=(const Client &x) {
	(void)x;
	return (*this);
}

int								Client::read_socket(void) {
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
	if (SUCCESS != (ret = _request.append(std::string(buffer)))) {
		std::cout << "error : " << ret << std::endl;
		_request.reset();
	}
	return (SUCCESS);
}

int								Client::get_sd(void) const {
	return (_sd);
}
