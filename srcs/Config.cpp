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

Config::Config() : AConfig() {
	_port = 0;
	_client_max_body_size = 8192;
}

Config::Config(const Config& src) {
	*this = src;
}

Config& Config::operator=(const Config& rhs) {
	if (this != &rhs) {
		this->_root = rhs._root;
		this->_autoindex = rhs._autoindex;
		this->_index = rhs._index;
		this->_methods = rhs._methods;
		this->_cgi_path = rhs._cgi_path;
		this->_cgi_extension = rhs._cgi_extension;
		_server_name = rhs._server_name;
		_ip_addr = rhs._ip_addr;
		_port = rhs._port;
		_client_max_body_size = rhs._client_max_body_size;
		_upload_dir = rhs._upload_dir;
		_locations = rhs._locations;
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

std::list<int> Config::getErrorPageCodes() const {
	return _error_page_codes;
}

void Config::setErrorPageCodes(const std::list<int>& errorPageCodes) {
	_error_page_codes = errorPageCodes;
}
std::string Config::getErrorPagePath() const {
	return _error_page_path;
}

void Config::setErrorPagePath(const std::string& errorPagePath) {
	_error_page_path = errorPagePath;
}

void Config::showConfig() const {
	std::list<std::string>::const_iterator it;
	std::string next;

	std::cout << std::endl << "Server configuration: " << std::endl;
	if (!_server_name.empty()) {
		std::cout << "server_name: `";
		it = this->_server_name.begin();
		while (it != this->_server_name.end()) {
			std::cout << *it++;
			next = it != this->_server_name.end() ? ", " : "'\n";
			std::cout << next;
		}
	}
	std::cout << "ip address: `" << _ip_addr << "'" << std::endl;
	std::cout << "port: `" << _port << "'" << std::endl;
	std::cout << "client_max_body_size: `" << _client_max_body_size << "'" << std::endl;
	std::cout << "upload_dir: `" << _upload_dir << "'" << std::endl;
	this->showCommonConfig();
	for(std::list<Location>::const_iterator it = _locations.begin(); it != _locations.end(); it++)
		it->showConfig();
	std::cout << std::endl;
}

std::list<Location> Config::getLocations() const {
	return _locations;
}

void Config::addLocation(const Location &location) {
	_locations.push_back(location);
}
