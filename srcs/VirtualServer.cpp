/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: louise <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/15 19:32:37 by louise            #+#    #+#             */
/*   Updated: 2021/04/15 19:32:42 by louise           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualServer.hpp"

VirtualServer::VirtualServer() : _client_max_body_size(DEFAULT_MAX_BODY_SIZE) {

}

VirtualServer::VirtualServer(const VirtualServer& src) {
	*this = src;
}

VirtualServer&
VirtualServer::operator=(const VirtualServer& rhs) {
	if (this != &rhs) {
		_server_names = rhs._server_names;
		_ip_addr = rhs._ip_addr;
		_port = rhs._port;
		_client_max_body_size = rhs._client_max_body_size;
		_upload_dir = rhs._upload_dir;
		_error_page_codes = rhs._error_page_codes;
		_error_page_path = rhs._error_page_path;
		_locations = rhs._locations;
	}
	return *this;
}

VirtualServer::~VirtualServer() {

}

std::string
VirtualServer::get_ip_addr() const {
	return _ip_addr;
}

void
VirtualServer::set_ip_addr(const std::string& ipAddr) {
	_ip_addr = ipAddr;
}

int
VirtualServer::get_port() const {
	return _port;
}

void
VirtualServer::set_port(int port) {
	_port = port;
}

std::list<std::string>
VirtualServer::get_server_names() const {
	return _server_names;
}

void
VirtualServer::set_server_names(const std::list<std::string> &serverName) {
	_server_names = serverName;
}

std::list<status_code_t>
VirtualServer::get_error_page_codes() const {
	return _error_page_codes;
}

void
VirtualServer::set_error_page_codes(const std::list<status_code_t>& errorPageCodes) {
	_error_page_codes = errorPageCodes;
}
const std::string&
VirtualServer::get_error_page_path() const {
	return _error_page_path;
}

void
VirtualServer::set_error_page_path(const std::string& errorPagePath) {
	_error_page_path = errorPagePath;
}

unsigned long
VirtualServer::get_client_max_body_size() const {
	return _client_max_body_size;
}

void
VirtualServer::set_client_max_body_size(unsigned long clientMaxBodySize) {
	_client_max_body_size = clientMaxBodySize;
}

const std::list<Location>&
VirtualServer::get_locations() const {
	return _locations;
}

void
VirtualServer::add_location(const Location &location) {
	_locations.push_back(location);
}

std::string
VirtualServer::get_upload_dir() const {
	return _upload_dir;
}

void
VirtualServer::set_upload_dir(const std::string &uploadDir) {
	_upload_dir = uploadDir;
}

std::list<const VirtualServer*>
VirtualServer::build_virtual_server_list(const std::list<VirtualServer>& virtual_servers,
	const VirtualServer* default_server) {
	std::list<const VirtualServer*> return_list;
	for(std::list<VirtualServer>::const_iterator it = virtual_servers.begin(); it != virtual_servers.end(); it++) {
		if (default_server->get_ip_addr() == it->get_ip_addr()
			&& default_server->get_port() == it->get_port())
			return_list.push_back(it.operator->());
	}
	return return_list;
}
