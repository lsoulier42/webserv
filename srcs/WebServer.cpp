/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 01:38:16 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/06 22:40:11 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

WebServer::WebServer(const std::vector<Server>& servers) :
	_servers(servers),
	_max_connection(DEFAULT_MAX_CONNECTION),
	_highest_socket(0), _exit(0) {
//	_client_sd.assign(_max_connection, 0);
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
//		std::cout << " - Slot=" << i << std::endl;
		_clients.push_back(Client(connection));
		_config_assoc.insert(std::make_pair(connection, server.getId()));
	}
	/*
	for (int i = 0; i < _max_connection; i++) {
		if (_client_sd[i] == 0) {
			std::cout << "Connection accepted: FD=" << connection;
			std::cout << " - Slot=" << i << std::endl;
			_client_sd[i] = connection;
			_config_assoc.insert(std::make_pair(connection, server.getId()));
			break ;
		}
		if (i == _max_connection - 1) {
			std::string full_response = "Sorry, this server is too busy.\n Try again later! \r\n";
			std::cout << "No room left for new client." << std::endl;
			send(connection, full_response.c_str(), full_response.size(), 0);
			close(connection);
		}
	}
	*/
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

int WebServer::sock_gets(int socket_fd, char *str, size_t count) {
	int read_return;
	char *current_position;
	size_t total_count = 0;
	char last_read = 0;

	current_position = str;
	while (last_read != '\n') {
		read_return = read(socket_fd, &last_read, 1);
		if (read_return <= 0)
			return -1;
		if ((total_count < count) && (last_read != '\n')
			&& (last_read != '\r')) {
			*current_position = last_read;
			current_position++;
			total_count++;
		}
	}
	if (count > 0)
		*current_position = '\0';
	return total_count;
}

/*
void WebServer::handle_data(int socket_id) {
	char buffer[DEFAULT_BUFFER_SIZE];

	if (sock_gets(_client_sd[socket_id], buffer, DEFAULT_BUFFER_SIZE) < 0) {
		std::cout << "Connection lost: FD=" << _client_sd[socket_id];
		std::cout << " - Slot=" << socket_id << std::endl;
		_client_sd[socket_id] = 0;
	} else {
		std::cout << "Received: " << buffer << std::endl;
		std::string received = std::string(buffer);
		if (received == "exit")
			_exit = 1;
		std::stringstream ss;
		ss << "I received this from server_id: "<< _config_assoc[_client_sd[socket_id]] << std::endl;
		send(_client_sd[socket_id], ss.str().c_str(), ss.str().size(), 0);
		std::cout << "Response: " << ss.str() << std::endl;
	}
}
*/

void WebServer::read_socks() {
	for(size_t i = 0; i < _servers.size(); i++) {
		if (_servers[i].isDefault()
			&& FD_ISSET(_servers[i].getServerSd(), &_sockets_list))
			this->accept_connection(_servers[i]);
	}
	for (std::vector<Client>::iterator it(_clients.begin()) ; it != _clients.end() ; it++) {
		if (FD_ISSET(it->get_sd(), &_sockets_list))
			it->process();
//			this->handle_data(i);
	}
}
