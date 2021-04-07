/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/07 12:14:15 by mdereuse         ###   ########.fr       */
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

int								Client::process(void) {
	char	buffer[_buffer_size + 1];
	int		ret;
	if (0 >= (ret = read(_sd, buffer, _buffer_size)))
		//TODO:: differentes erreurs en fonction de ret
		return (FAILURE);
	buffer[ret] = '\0';
	_request.append(std::string(buffer));
	return (SUCCESS);
}

int								Client::get_sd(void) const {
	return (_sd);
}
