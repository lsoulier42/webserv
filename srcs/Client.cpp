/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/08 21:37:31 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

const size_t	Client::_buffer_size(900);

Client::Client(void) :
	_sd(0),
	_request() {}

Client::Client(int sd) :
	_sd(sd),
	_request() {}

Client::Client(const Client &x) :
	_sd(x._sd),
	_request(x._request) {}

Client::~Client(void) {}

Client
&Client::operator=(const Client &x) {
	_request = x._request;
	return (*this);
}

int
Client::get_sd(void) const {
	return (_sd);
}

struct sockaddr
Client::get_addr(void) const {
	return _addr;
}

void
Client::set_addr(const struct sockaddr& addr) {
	_addr = addr;
}

socklen_t
Client::get_socket_len(void) const {
	return _socket_len;
}

void
Client::set_socket_len(const socklen_t& socket_len) {
	_socket_len = socket_len;
}

std::list<const Config*>
Client::get_configs() const {
	return _configs;
}

void
Client::set_configs(const std::list<const Config*>& configs) {
	_configs = configs;
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
	if (CONTINUE_READING == ret)
		return (SUCCESS);
	else
		return (_process(ret));
}

/*
 * ici, on traite la requete
 * une fois traitee, la requete doit etre reset
 * return (autre chose que SUCCESS) entrainera la fermeture de la connexion et la suppression de l'objet Client dans Webserver::read_socks()
 */
int
Client::_process(int status) {
	std::cout << "-----> REPONSE : " << status << std::endl << std::endl;
	_request.render();
	_request.reset();
	return (SUCCESS);
}

/* This function get the right configuration of virtual host
 * based on header host and server_name
 * http://nginx.org/en/docs/http/request_processing.html
 *
 */

const Config*
Client::extractVirtualServer() const {
	std::string host_requested;
	std::vector<std::string> host_elements;
	std::string effective_host;
	std::list<std::string> server_names;
	const Config* virtual_server_found = NULL;

	if (_request.get_headers().key_exists("Host")) {
		host_requested = _request.get_headers().get_value("Host");
		host_elements = split(host_requested, ":");
		effective_host = host_elements[0];
	}
	for(std::list<const Config*>::const_iterator it = _configs.begin();
		it != _configs.end(); it++) {
		server_names = (*it)->getServerNames();
		for (std::list<std::string>::const_iterator cit = server_names.begin();
			cit != server_names.end(); it++) {
			if (*cit == effective_host) {
				virtual_server_found = *it;
				break;
			}
		}
		if (virtual_server_found)
			break;
	}
	return virtual_server_found ? virtual_server_found : _configs.front();
}
