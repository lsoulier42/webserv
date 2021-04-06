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

Server::Server() : _server_sd(0), _reuse_addr(1),
	_addr_len(sizeof(struct sockaddr_in)) {
	memset((char*)&_sock_addr, 0, _addr_len);
}

Server::Server(const Server &src) { *this = src; }

Server& Server::operator=(const Server &rhs) {
	if (this != &rhs) {
		_config = rhs._config;
		_server_sd = rhs._server_sd;
		_addr_len = rhs._addr_len;
		_reuse_addr = rhs._reuse_addr;
		_sock_addr = rhs._sock_addr;
	}
	return *this;
}

Server::~Server() {

}

int Server::getServerSd() const {
	return _server_sd;
}

struct sockaddr* Server::getSockAddr() const {
	return (struct sockaddr*)&_sock_addr;
}

socklen_t* Server::getAddrLen() const {
	return (socklen_t*)&_addr_len;
}

const Config& Server::getConfig() const{
	return this->_config;
}

void Server::setConfig(const Config &config) {
	_config = config;
}

void Server::_create_socket_descriptor() {
	_server_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_sd == -1) {
		std::cout << "Failed to create socket : ";
		std::cout << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	if (WebServer::verbose) {
		std::cout << "Success, server sd created is : " << _server_sd << std::endl;
	}
}

void Server::setup_default_server(const Config& config) {
	this->setConfig(config);
	this->_create_socket_descriptor();
	this->_change_socket_options();
	this->_bind_socket();
	this->_set_listen_mode();
}

void Server::_change_socket_options() {
	if (setsockopt(_server_sd, SOL_SOCKET, SO_REUSEADDR,
		&_reuse_addr, sizeof(_reuse_addr)) < 0) {
		std::cout << "Failed to change socket options : ";
		std::cout << std::strerror(errno) << std::endl;
		close(_server_sd);
		exit(EXIT_FAILURE);
	}
	if (WebServer::verbose) {
		std::cout << "Socket options changes succesfully on sd: " << _server_sd << std::endl;
	}
}

void Server::_bind_socket() {
	int port = _config.getPort();

	_sock_addr.sin_family = AF_INET;
	_sock_addr.sin_addr.s_addr = inet_addr(_config.getIpAddr().c_str());
	_sock_addr.sin_port = htons(port);

	if (bind(_server_sd, (struct sockaddr*)&_sock_addr, _addr_len) < 0) {
		std::cout << "Failed to bind to port " << port;
		std::cout << " : " << std::strerror(errno) << std::endl;
		close(_server_sd);
		exit(EXIT_FAILURE);
	}
	if (WebServer::verbose) {
		std::cout << "Sd: " << _server_sd << " successfully bind on port: " << port << std::endl;
	}
}

void Server::_set_listen_mode() const {
	if (listen(_server_sd, DEFAULT_BACKLOG) < 0) {
		std::cout << "Failed to listen on socket : ";
		std::cout << std::strerror(errno) << std::endl;
		close(_server_sd);
		exit(EXIT_FAILURE);
	}
	if (WebServer::verbose) {
		std::cout << "Sd: " << _server_sd << " successfully set in listen mode." << std::endl;
	}
}
