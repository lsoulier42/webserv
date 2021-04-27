/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chris <chris@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 01:38:16 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/22 20:10:02 by chris            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"
#include "ResponseHandling.hpp"

WebServer::WebServer() :
	_max_connection(DEFAULT_MAX_CONNECTION),
	_highest_socket(0) {

}

WebServer::~WebServer() {

}

void
WebServer::setup_servers() {
	std::set<std::pair<std::string, int> > unique_pairs;
	std::pair<std::string, int> ip_port;
	Server new_server;

	for(std::list<VirtualServer>::iterator it = _virtual_servers.begin(); it != _virtual_servers.end(); it++) {
		ip_port = std::make_pair(it->get_ip_addr(), it->get_port());
		if (unique_pairs.find(ip_port) == unique_pairs.end()) {
			unique_pairs.insert(ip_port);
			new_server.set_virtual_server(&(*it));
			_servers.push_back(new_server);
		}
	}
	for(std::list<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		it->setup_default_server();
		set_non_blocking(it->get_server_sd());
		_highest_socket = it->get_server_sd();
	}
}

void
WebServer::_accept_connection(const Server& server) {
	int connection;
	bool max_client_reached;
	struct sockaddr client_addr = *server.get_sock_addr();
	socklen_t client_socket_len = *server.get_addr_len();

	connection = accept(server.get_server_sd(),
		&client_addr, &client_socket_len);
	if (connection < 0 && !sig_value) {
		std::cerr << "Failed to grab connection : ";
		std::cerr << std::strerror(errno) << std::endl;
		this->_close_sockets();
		exit(EXIT_FAILURE);
	}
	set_non_blocking(connection);
	if (connection == -1)
		return ;
	max_client_reached = _clients.size() >= _max_connection;
	_clients.push_back(Client(connection, client_addr, client_socket_len,
		VirtualServer::build_virtual_server_list(_virtual_servers, server.get_virtual_server()), max_client_reached));
	DEBUG_COUT("Connection successfully established with " << _clients.front().get_ip_addr());
}

void
WebServer::_close_sockets() {
	for(std::list<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		close(it->get_server_sd());
	}
	for(std::list<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
		close(it->get_sd());
}

void
WebServer::routine(void) {
	struct timeval timeout;
	int nb_sockets;

	if (_servers.empty()) {
		DEBUG_COUT("No server has been set up yet");
		exit(EXIT_FAILURE);
	}
	while(!sig_value) {
		this->_build_select_list();
		timeout.tv_sec = 0;
		timeout.tv_usec = 10;
		nb_sockets = select(_highest_socket + 1, &_sockets_list[READ],
			&_sockets_list[WRITE], (fd_set*)0, &timeout);
		if (nb_sockets < 0 && !sig_value) {
			std::cerr << "Select error: ";
			std::cerr << std::strerror(errno) << std::endl;
			this->_close_sockets();
			exit(EXIT_FAILURE);
		}
		if (nb_sockets != 0 && !sig_value) {
			this->_read_socks();
			this->_write_socks();
		}
	}
	DEBUG_COUT(PROGRAM_VERSION << " is exiting majestically.");
	this->_close_sockets();
}

void
WebServer::set_non_blocking(int file_descriptor) {
	int opts;

	opts = fcntl(file_descriptor, F_GETFD);
	if (opts < 0) {
		DEBUG_COUT("Fcntl error with F_GETFD : " << std::strerror(errno));
		exit(EXIT_FAILURE);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(file_descriptor, F_SETFL, opts) < 0) {
		DEBUG_COUT("Fcntl error with F_SETFL : " << std::strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void
WebServer::_build_select_list() {
	FD_ZERO(&_sockets_list[READ]);
	FD_ZERO(&_sockets_list[WRITE]);
	for(std::list<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		FD_SET(it->get_server_sd(), &_sockets_list[READ]);
	}
	for(std::list<Client>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		FD_SET(it->get_sd(), &_sockets_list[READ]);
		FD_SET(it->get_sd(), &_sockets_list[WRITE]);
		if (it->get_sd() > _highest_socket)
			_highest_socket = it->get_sd();
		if (it->get_fd() != 0 && it->get_fd()) {
			FD_SET(it->get_fd(), &_sockets_list[READ]);
			if (it->get_fd() > _highest_socket)
				_highest_socket = it->get_fd();
		}
		if (it->get_cgi_output_fd() != 0) {
			FD_SET(it->get_cgi_output_fd(), &_sockets_list[READ]);
			if (it->get_cgi_output_fd() > _highest_socket)
				_highest_socket = it->get_cgi_output_fd();
		}
		if (it->get_cgi_input_fd() != 0) {
			FD_SET(it->get_cgi_input_fd(), &_sockets_list[WRITE]);
			if (it->get_cgi_input_fd() > _highest_socket)
				_highest_socket = it->get_cgi_input_fd();
		}
		if (it->get_file_write_fd() > 0) {
			FD_SET(it->get_file_write_fd(), &_sockets_list[WRITE]);
			if (it->get_file_write_fd() > _highest_socket)
				_highest_socket = it->get_file_write_fd();
		}
  }
}

void
WebServer::_read_socks() {
	for(std::list<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		if (FD_ISSET(it->get_server_sd(), &_sockets_list[READ]))
			this->_accept_connection(*it);
	}
	if (sig_value != 0)
		return;
	for (std::list<Client>::iterator it(_clients.begin()) ; it != _clients.end() ; ) {
		if (FD_ISSET(it->get_sd(), &_sockets_list[READ]) && it->read_socket() == FAILURE) {
			close(it->get_sd());
			it = _clients.erase(it);
		}
		else
			it++;
	}
	for (std::list<Client>::iterator it(_clients.begin()) ; it!= _clients.end() ; it++) {
		if (FD_ISSET(it->get_fd(), &_sockets_list[READ]))
			it->read_target_resource();
	}
	for (std::list<Client>::iterator it(_clients.begin()) ; it!= _clients.end() ; it++)
		if (FD_ISSET(it->get_cgi_output_fd(), &_sockets_list[READ]))
			it->read_cgi_output();
}

void
WebServer::_write_socks() {
	for (std::list<Client>::iterator it(_clients.begin()) ; it != _clients.end() ;) {
		if (FD_ISSET(it->get_sd(), &_sockets_list[WRITE]) && it->write_socket() == FAILURE) {
			close(it->get_sd());
			it = _clients.erase(it);
		}
		else
			it++;
	}
	for (std::list<Client>::iterator it(_clients.begin()) ; it != _clients.end(); it++)
		if (FD_ISSET(it->get_file_write_fd(), &_sockets_list[WRITE])) {
			std::map<std::string, int>::iterator file = _locked_files.find(it->get_target_path());
			if (file == _locked_files.end())
				_locked_files.insert(std::make_pair(it->get_target_path(), it->get_file_write_fd()));
			if (file == _locked_files.end()
				|| (file != _locked_files.end() && file->second == it->get_file_write_fd())) {
				it->write_target_resource();
				if (it->get_file_write_fd() == 0)
					_locked_files.erase(it->get_target_path());
			}
		}
	for (std::list<Client>::iterator it(_clients.begin()) ; it!= _clients.end() ; it++)
		if (FD_ISSET(it->get_cgi_input_fd(), &_sockets_list[WRITE]))
			it->write_cgi_input();
}

int
WebServer::parsing(const std::string &filepath) {
	if (!ConfigParsing::check_config_file(filepath, _config_file))
		return (FAILURE);
	if (!ConfigParsing::check_main_bloc(_config_file, _virtual_servers)) {
		DEBUG_COUT("Error during config file parsing.");
		_config_file.close();
		return (FAILURE);
	}
	_config_file.close();
	return (SUCCESS);
}


int
WebServer::sig_value = 0;

void
WebServer::sigint_handler(int signum) {
	sig_value = signum;
}

int main(int argc, char **argv) {
	WebServer webserv;
	std::string filepath;

	if (signal(SIGINT, &WebServer::sigint_handler) == SIG_ERR)
		return (EXIT_FAILURE);
	if (argc >= 2 && argc <= 3) {
		for (int i = 1; i < argc; i++) {
			std::string argument(argv[i]);
			if (argument == "-v") {
				DEBUG_START(true);
			}
			else
				filepath = argument;
		}
	}
	DEBUG_COUT(PROGRAM_VERSION << " has started.");
	if (filepath.empty())
		filepath = "conf/default.conf";
	if (!webserv.parsing(filepath))
		return EXIT_FAILURE;
	DEBUG_COUT("Config file parsing went well");
	webserv.setup_servers();
	webserv.routine();
	return (EXIT_SUCCESS);
}
