/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/02 15:32:15 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/02 15:32:18 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() : _socket_fd(-1), _new_socket(-1) {
	_addrlen = sizeof(_sock_addr);
	memset((char*)&_sock_addr, 0, _addrlen);
}

Server::Server(const Server &src) { *this = src; }

Server& Server::operator=(const Server &rhs) {
	if (this != &rhs) {
		_socket_fd = rhs._socket_fd;
		_addrlen = rhs._addrlen;
		_sock_addr = rhs._sock_addr;
		_new_socket = rhs._new_socket;
	}
	return *this;
}

Server::~Server() {}

void Server::create_socket() {
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd == -1) {
		std::cout << "Failed to create socket : ";
		std::cout << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}
void Server::bind_socket(int port) {
	_sock_addr.sin_family = AF_INET;
	_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sock_addr.sin_port = htons(port);

	if (bind(_socket_fd, (struct sockaddr*)&_sock_addr, _addrlen) < 0) {
		std::cout << "Failed to bind to port " << port;
		std::cout << " : " << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

void Server::listen_mode() {
	if (listen(_socket_fd, 10) < 0) {
		std::cout << "Failed to listen on socket : ";
		std::cout << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

void Server::accept_connection() {
	_new_socket = accept(_socket_fd, (struct sockaddr*)&_sock_addr,
		(socklen_t*)&_addrlen);
	if (_new_socket < 0) {
		std::cout << "Failed to grab connection : ";
		std::cout << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

int Server::server_routine() {
	char buffer[1000];
	int read_return;
	std::string response = "Good talking to you\n";

	this->accept_connection();
	read_return = read(_new_socket, buffer, 1000);
	buffer[read_return] = '\0';
	std::cout << "The message was: " << buffer;
	send(_new_socket, response.c_str(), response.size(), 0);
	close(_new_socket);

	return (1);
}

void Server::connect(void) {
	this->create_socket();
	this->bind_socket(BINDING_PORT);
	this->listen_mode();
	while(server_routine())
		;
	close(_socket_fd);
}
