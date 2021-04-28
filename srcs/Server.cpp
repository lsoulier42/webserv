/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cchenot <cchenot@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/02 15:32:15 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/21 20:32:02 by cchenot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Request.hpp"

Server::Server() : _virtual_server(NULL), _server_sd(0), _reuse_addr(1),
	_addr_len(sizeof(struct sockaddr_in)) {
	memset((char*)&_sock_addr, 0, _addr_len);
}

Server::Server(const Server &src) { *this = src; }

Server&
Server::operator=(const Server &rhs) {
	if (this != &rhs) {
		_virtual_server = rhs._virtual_server;
		_server_sd = rhs._server_sd;
		_addr_len = rhs._addr_len;
		_reuse_addr = rhs._reuse_addr;
		_sock_addr = rhs._sock_addr;
	}
	return *this;
}

Server::~Server() {

}

int
Server::get_server_sd() const {
	return _server_sd;
}

struct sockaddr*
Server::get_sock_addr() const {
	return (struct sockaddr*)(&_sock_addr);
}

socklen_t*
Server::get_addr_len() const {
	return (socklen_t*)(&_addr_len);
}

const VirtualServer*
Server::get_virtual_server() const{
	return this->_virtual_server;
}

void
Server::set_virtual_server(VirtualServer* virtual_server) {
	_virtual_server = virtual_server;
}

void
Server::setup_default_server() {
	this->_create_socket_descriptor();
	this->_change_socket_options();
	this->_bind_socket();
	this->_set_listen_mode();
}

void
Server::_create_socket_descriptor() {
	_server_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_sd == -1) {
		std::cerr << "Failed to create socket : ";
		std::cerr << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	if (fcntl(_server_sd, F_SETFL, O_NONBLOCK) < 0) {
		DEBUG_COUT("Fcntl error with F_SETFL : " << std::strerror(errno));
		exit(EXIT_FAILURE);
	}
	DEBUG_COUT("Server socket successfully created at file descriptor " << _server_sd);
}

void
Server::_change_socket_options() {
	if (setsockopt(_server_sd, SOL_SOCKET, SO_REUSEADDR,
		&_reuse_addr, sizeof(_reuse_addr)) < 0) {
		std::cerr << "Failed to change socket options : ";
		std::cerr << std::strerror(errno) << std::endl;
		close(_server_sd);
		exit(EXIT_FAILURE);
	}
}

void
Server::_bind_socket() {
	int port = _virtual_server->get_port();

	_sock_addr.sin_family = AF_INET;
	_sock_addr.sin_addr.s_addr = inet_addr(_virtual_server->get_ip_addr().c_str());
	_sock_addr.sin_port = htons(port);

	if (bind(_server_sd, (struct sockaddr*)&_sock_addr, _addr_len) < 0) {
		std::cerr << "Failed to bind to port " << port;
		std::cerr << " : " << std::strerror(errno) << std::endl;
		close(_server_sd);
		exit(EXIT_FAILURE);
	}
	DEBUG_COUT("Server with file descriptor " <<  _server_sd << " has been successfully bind on port: " << port);
}

void
Server::_set_listen_mode() const {
	if (listen(_server_sd, DEFAULT_BACKLOG) < 0) {
		std::cerr << "Failed to listen on socket : ";
		std::cerr << std::strerror(errno) << std::endl;
		close(_server_sd);
		exit(EXIT_FAILURE);
	}
}
