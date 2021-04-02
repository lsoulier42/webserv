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

Server::Server() : _server_fd(-1), _addrlen(sizeof(_sock_addr)),
	_reuse_addr(1), _high_sock(-1), _exit(0) {
	_max_connection = DEFAULT_MAX_CONNECTION;
	_connect_list = _alloc.allocate(_max_connection);
	for (int i = 0; i < _max_connection; i++)
		_alloc.construct(_connect_list + i, 0);
	memset((char*)&_sock_addr, 0, _addrlen);
}

Server::~Server() {
	for (int i = 0; i < _max_connection; i++)
		_alloc.destroy(_connect_list + i);
	_alloc.deallocate(_connect_list, _max_connection);
}

void Server::setup_server_socket() {
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd == -1) {
		std::cout << "Failed to create socket : ";
		std::cout << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR,
		&_reuse_addr, sizeof(_reuse_addr));
	set_non_blocking(_server_fd);
	_high_sock = _server_fd;
	this->bind_socket(DEFAULT_PORT);
	this->listen_mode();
}
void Server::bind_socket(int port) {
	_sock_addr.sin_family = AF_INET;
	_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sock_addr.sin_port = htons(port);

	if (bind(_server_fd, (struct sockaddr*)&_sock_addr, _addrlen) < 0) {
		std::cout << "Failed to bind to port " << port;
		std::cout << " : " << std::strerror(errno) << std::endl;
		close(_server_fd);
		exit(EXIT_FAILURE);
	}
}

void Server::listen_mode() {
	if (listen(_server_fd, DEFAULT_BACKLOG) < 0) {
		std::cout << "Failed to listen on socket : ";
		std::cout << std::strerror(errno) << std::endl;
		close(_server_fd);
		exit(EXIT_FAILURE);
	}
}

void Server::accept_connection() {
	int connection;

	connection = accept(_server_fd, (struct sockaddr*)&_sock_addr,
		(socklen_t*)&_addrlen);
	if (connection < 0) {
		std::cout << "Failed to grab connection : ";
		std::cout << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	set_non_blocking(connection);
	if (connection == -1)
		return ;
	for (int i = 0; i < _max_connection; i++) {
		if (_connect_list[i] == 0) {
			std::cout << "Connection accepted: FD=" << connection;
			std::cout << " - Slot=" << i << std::endl;
			_alloc.construct(_connect_list + i, connection);
			break ;
		}
		if (i == _max_connection - 1) {
			std::string full_response = "Sorry, this server is too busy.\n Try again later! \r\n";
			std::cout << "No room left for new client." << std::endl;
			send(connection, full_response.c_str(), full_response.size(), 0);
			close(connection);
		}
	}
}

void Server::connect(void) {
	struct timeval timeout;
	int read_socks;

	this->setup_server_socket();
	while(!_exit) {
		this->build_select_list();
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		read_socks = select(_high_sock + 1, &_socks,
			(fd_set*)0, (fd_set*)0, &timeout);
		if (read_socks < 0) {
			std::cout << "Select error: ";
			std::cout << std::strerror(errno) << std::endl;
			close(_server_fd);
			exit(EXIT_FAILURE);
		}
		if (read_socks == 0) {
			std::cout << ".";
		}
		else
			this->read_socks();
	}
	close(_server_fd);
}

void Server::set_non_blocking(int socket_fd) {
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

void Server::build_select_list() {
	FD_ZERO(&_socks);
	FD_SET(_server_fd, &_socks);
	for(int i = 0; i < _max_connection; i++) {
		if (_connect_list[i] != 0) {
			FD_SET(_connect_list[i], &_socks);
			if (_connect_list[i] > _high_sock)
				_high_sock = _connect_list[i];
		}
	}
}

int Server::sock_gets(int socket_fd, char *str, size_t count) {
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

void Server::handle_data(int socket_id) {
	char buffer[DEFAULT_BUFFER_SIZE];

	if (sock_gets(_connect_list[socket_id], buffer, DEFAULT_BUFFER_SIZE) < 0) {
		std::cout << "Connection lost: FD=" << _connect_list[socket_id];
		std::cout << " - Slot=" << socket_id << std::endl;
		_alloc.construct(_connect_list + socket_id, 0);
	} else {
		std::cout << "Received: " << buffer << std::endl;
		std::string received = std::string(buffer);
		if (received == "exit")
			_exit = 1;
		for(int i = 0; buffer[i]; i++)
			buffer[i] = (char)toupper(buffer[i]);
		send(_connect_list[socket_id], buffer, strlen(buffer), 0);
		std::cout << "Response: " << buffer << std::endl;
	}
}

void Server::read_socks() {
	if (FD_ISSET(_server_fd, &_socks))
		this->accept_connection();
	for (int i = 0; i < _max_connection; i++) {
		if (FD_ISSET(_connect_list[i], &_socks))
			this->handle_data(i);
	}
}
