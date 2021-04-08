/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 01:38:16 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/08 21:52:05 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

WebServer::WebServer(const std::vector<Server>& servers) :
	_servers(servers),
	_max_connection(DEFAULT_MAX_CONNECTION),
	_highest_socket(0), _exit(0) {
}

WebServer::~WebServer() {}

void WebServer::setup_servers() {
	for(std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		it->setup_default_server();
		set_non_blocking(it->getServerSd());
		_highest_socket = it->getServerSd();
	}
}

void WebServer::accept_connection(const Server& server) {
	int connection;

	connection = accept(server.getServerSd(),
		server.getSockAddr(), server.getAddrLen());
	if (connection < 0) {
		std::cout << "Failed to grab connection : ";
		std::cout << std::strerror(errno) << std::endl;
		this->close_sockets();
		exit(EXIT_FAILURE);
	}
	set_non_blocking(connection);
	if (connection == -1)
		return ;
	if (_clients.size() == (size_t)_max_connection) {
		std::string full_response = "Sorry, this server is too busy.\n Try again later! \r\n";
		std::cout << "No room left for new client." << std::endl;
		send(connection, full_response.c_str(), full_response.size(), 0);
			close(connection);
	} else {
		std::cout << "Connection accepted: FD=" << connection << std::endl;
		_clients.push_back(Client(connection));
		_config_assoc.insert(std::make_pair(connection, server.getId()));
	}
}

void WebServer::close_sockets() {
	for(std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		if (it->isDefault())
			close(it->getServerSd());
	}
	for(std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
		close(it->get_sd());
}

void WebServer::routine(void) {
	struct timeval timeout;
	int nb_sockets;

	if (_servers.empty()) {
		std::cout << "No server has been set up yet" << std::endl;
		exit(EXIT_FAILURE);
	}

	while(!_exit) {
		this->build_select_list();
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		nb_sockets = select(_highest_socket + 1, &_sockets_list,
			(fd_set*)0, (fd_set*)0, &timeout);
		if (nb_sockets < 0) {
			std::cout << "Select error: ";
			std::cout << std::strerror(errno) << std::endl;
			this->close_sockets();
			exit(EXIT_FAILURE);
		}
		if (nb_sockets == 0) {
			std::cout << ".";
		}
		else
			this->read_socks();
	}
	this->close_sockets();
}

void WebServer::set_non_blocking(int socket_fd) {
	int opts;

	opts = fcntl(socket_fd, F_GETFD);
	if (opts < 0) {
		std::cout << "Fcntl error with F_GETFD : ";
		std::cout << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(socket_fd, F_SETFL, opts) < 0) {
		std::cout << "Fcntl error with F_SETFL : ";
		std::cout << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

void WebServer::build_select_list() {
	FD_ZERO(&_sockets_list);
	for(std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		if (it->isDefault())
			FD_SET(it->getServerSd(), &_sockets_list);
	}
	for(std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		FD_SET(it->get_sd(), &_sockets_list);
		if (it->get_sd() > _highest_socket)
			_highest_socket = it->get_sd();
	}
}

void WebServer::read_socks() {
	for(size_t i = 0; i < _servers.size(); i++) {
		if (_servers[i].isDefault()
			&& FD_ISSET(_servers[i].getServerSd(), &_sockets_list))
			this->accept_connection(_servers[i]);
	}
	for (std::vector<Client>::iterator it(_clients.begin()) ; it != _clients.end() ; ) {
		if (FD_ISSET(it->get_sd(), &_sockets_list) && SUCCESS != it->read_socket()) {
			close(it->get_sd());
			it = _clients.erase(it);
		}
		else
			it++;
	}
}
