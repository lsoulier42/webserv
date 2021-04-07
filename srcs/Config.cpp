/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 17:04:05 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/06 17:04:06 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config() {

}

Config::Config(const Config& src) {
	*this = src;
}

Config& Config::operator=(const Config& rhs) {
	if (this != &rhs) {
		_server_name = rhs._server_name;
		_ip_addr = rhs._ip_addr;
		_port = rhs._port;
		_root = rhs._root;
		_autoindex = rhs._autoindex;
		_index = rhs._index;
		_methods = rhs._methods;
		_client_max_body_size = rhs._client_max_body_size;
		_upload_dir = rhs._upload_dir;
	}
	return *this;
}

Config::~Config() {

}

std::list<std::string> Config::getServerName() const {
	return _server_name;
}

void Config::setServerName(const std::list<std::string> &serverName) {
	_server_name = serverName;
}

std::string Config::getIpAddr() const {
	return _ip_addr;
}

void Config::setIpAddr(const std::string& ipAddr) {
	_ip_addr = ipAddr;
}

int Config::getPort() const {
	return _port;
}

void Config::setPort(int port) {
	_port = port;
}

std::string Config::getRoot() const {
	return _root;
}

void Config::setRoot(const std::string &root) {
	_root = root;
}

bool Config::isAutoindex() const {
	return _autoindex;
}

void Config::setAutoindex(bool autoindex) {
	_autoindex = autoindex;
}

std::list<std::string> Config::getIndex() const {
	return _index;
}

void Config::setIndex(const std::list<std::string> &index) {
	_index = index;
}

std::list<std::string> Config::getMethods() const {
	return _methods;
}

void Config::setMethods(const std::list<std::string> &methods) {
	_methods = methods;
}

int Config::getClientMaxBodySize() const {
	return _client_max_body_size;
}

void Config::setClientMaxBodySize(int clientMaxBodySize) {
	_client_max_body_size = clientMaxBodySize;
}

std::string Config::getUploadDir() const {
	return _upload_dir;
}

void Config::setUploadDir(const std::string &uploadDir) {
	_upload_dir = uploadDir;
}
