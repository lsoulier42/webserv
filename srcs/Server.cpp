/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/02 15:32:15 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/06 20:07:01 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Request.hpp"

int Server::server_id = 0;

Server::Server() : _id(server_id++),
	_server_sd(0), _reuse_addr(1), _addr_len(sizeof(struct sockaddr_in)) {
	memset((char*)&_sock_addr, 0, _addr_len);
}

Server::Server(const std::string& ipAddr, int port) : _id(server_id++),
	_is_default(true),
	_ip_addr(ipAddr), _port(port), _server_sd(0),
	_reuse_addr(1), _addr_len(sizeof(struct sockaddr_in)) {
	memset((char*)&_sock_addr, 0, _addr_len);
}

Server::Server(const Server& src) {
	*this = src;
}

Server& Server::operator=(const Server& rhs) {
	if (this != &rhs) {
		_id = server_id++;
		_server_name = rhs._server_name;
		_is_default = rhs._is_default;
		_ip_addr = rhs._ip_addr;
		_port = rhs._port;
		_root = rhs._root;
		_autoindex = rhs._autoindex;
		_index = rhs._index;
		_methods = rhs._methods;
		_upload_dir = rhs._upload_dir;
		_server_sd = rhs._server_sd;
		_sock_addr = rhs._sock_addr;
		_reuse_addr = rhs._reuse_addr;
		_addr_len = rhs._addr_len;
	}
	return *this;
}

Server::~Server() {

}

std::string Server::getServerName() const {
	return _server_name;
}

void Server::setServerName(const std::string &serverName) {
	_server_name = serverName;
}

int Server::getId() const {
	return _id;
}

void Server::setId(int id) {
	_id = id;
}

bool Server::isDefault() const {
	return _is_default;
}

void Server::setDefault(bool isDefault) {
	_is_default = isDefault;
}

std::string Server::getIpAddr() const {
	return _ip_addr;
}

void Server::setIpAddr(const std::string& ipAddr) {
	_ip_addr = ipAddr;
}

int Server::getPort() const {
	return _port;
}

void Server::setPort(int port) {
	_port = port;
}

std::string Server::getRoot() const {
	return _root;
}

void Server::setRoot(const std::string &root) {
	_root = root;
}

bool Server::isAutoindex() const {
	return _autoindex;
}

void Server::setAutoindex(bool autoindex) {
	_autoindex = autoindex;
}

std::list<std::string> Server::getIndex() const {
	return _index;
}

void Server::setIndex(const std::list<std::string> &index) {
	_index = index;
}

std::list<std::string> Server::getMethods() const {
	return _methods;
}

void Server::setMethods(const std::list<std::string> &methods) {
	_methods = methods;
}

int Server::getBufferBodySize() const {
	return _buffer_body_size;
}

void Server::setBufferBodySize(int bufferBodySize) {
	_buffer_body_size = bufferBodySize;
}

std::string Server::getUploadDir() const {
	return _upload_dir;
}

void Server::setUploadDir(const std::string &uploadDir) {
	_upload_dir = uploadDir;
}

void Server::setup_default_server() {
	if (!_is_default)
		return ;
	this->_create_socket_descriptor();
	this->_change_socket_options();
	this->_bind_socket();
	this->_set_listen_mode();
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

void Server::_create_socket_descriptor() {
	_server_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_sd == -1) {
		std::cout << "Failed to create socket : ";
		std::cout << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

void Server::_change_socket_options() {
	if (setsockopt(_server_sd, SOL_SOCKET, SO_REUSEADDR,
		&_reuse_addr, sizeof(_reuse_addr)) < 0) {
		std::cout << "Failed to change socket options : ";
		std::cout << std::strerror(errno) << std::endl;
		close(_server_sd);
		exit(EXIT_FAILURE);
	}
}

void Server::_bind_socket() {
	_sock_addr.sin_family = AF_INET;
	_sock_addr.sin_addr.s_addr = inet_addr(_ip_addr.c_str());
	_sock_addr.sin_port = htons(_port);

	if (bind(_server_sd, (struct sockaddr*)&_sock_addr, _addr_len) < 0) {
		std::cout << "Failed to bind to port " << _port;
		std::cout << " : " << std::strerror(errno) << std::endl;
		close(_server_sd);
		exit(EXIT_FAILURE);
	}
}

void Server::_set_listen_mode() {
	if (listen(_server_sd, DEFAULT_BACKLOG) < 0) {
		std::cout << "Failed to listen on socket : ";
		std::cout << std::strerror(errno) << std::endl;
		close(_server_sd);
		exit(EXIT_FAILURE);
	}
}
