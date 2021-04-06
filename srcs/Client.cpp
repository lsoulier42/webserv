/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/06 22:34:18 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

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

void							Client::process(void) {
	std::cout << "i am processing" << std::endl;
}

int								Client::get_sd(void) const {
	return (_sd);
}
