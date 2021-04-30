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

size_t WebServer::write_buffer_size = 65000;

WebServer::WebServer() :
	_config_file(),
	_servers(),
	_virtual_servers(),
	_max_connection(DEFAULT_MAX_CONNECTION),
	_clients(),
	_sockets_list(),
	_highest_socket(0) {

}

WebServer::~WebServer() {

}

void
WebServer::setup_servers() {
	std::set<std::pair<std::string, int> >	unique_pairs;
	std::pair<std::string, int> 			ip_port;
	Server 									new_server;

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
		_highest_socket = it->get_server_sd();
	}
}

void
WebServer::_accept_connection(const Server& server) {
	int				connection;
	bool 			max_client_reached;
	struct sockaddr	client_addr = *server.get_sock_addr();
	socklen_t 		client_socket_len = *server.get_addr_len();

	connection = accept(server.get_server_sd(),
		&client_addr, &client_socket_len);
	if (connection < 0 && !sig_value) {
		std::cerr << "Failed to grab connection : ";
		std::cerr << std::strerror(errno) << std::endl;
		this->_close_sockets();
		exit(EXIT_FAILURE);
	}
	if (fcntl(connection, F_SETFL, O_NONBLOCK) < 0) {
		DEBUG_COUT("Fcntl error with F_SETFL : " << std::strerror(errno));
		exit(EXIT_FAILURE);
	}
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
	struct timeval	timeout;
	int 			nb_sockets;

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
		std::list<Client::exchange_t>& exchanges = it->get_exchanges();

		for (std::list<Client::exchange_t>::iterator exchanges_it = exchanges.begin();
			exchanges_it != exchanges.end(); exchanges_it++) {
			int tmp_fd = exchanges_it->first.get_tmp_fd();
			if (tmp_fd != 0) {
				FD_SET(tmp_fd, &_sockets_list[WRITE]);
				if (tmp_fd > _highest_socket)
					_highest_socket = tmp_fd;
			}
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
			continue;
		}
		std::list<Client::exchange_t> &exchanges = it->get_exchanges();
		if (!exchanges.empty() && exchanges.front().first.get_status() == Request::REQUEST_RECEIVED)
			it->process(exchanges.front());
		if (FD_ISSET(it->get_fd(), &_sockets_list[READ]))
			it->read_target_resource();
		if (FD_ISSET(it->get_cgi_output_fd(), &_sockets_list[READ]))
			it->read_cgi_output();
		it++;
	}
}

void
WebServer::_write_socks() {
	for (std::list<Client>::iterator it(_clients.begin()) ; it != _clients.end() ; ) {
		std::list<Client::exchange_t>& exchanges = it->get_exchanges();

		for (std::list<Client::exchange_t>::iterator exchanges_it = exchanges.begin();
			 exchanges_it != exchanges.end(); exchanges_it++) {
			int tmp_fd = exchanges_it->first.get_tmp_fd();
			if (FD_ISSET(tmp_fd, &_sockets_list[WRITE]))
				exchanges_it->first.write_tmp_file();
		}
		if (FD_ISSET(it->get_sd(), &_sockets_list[WRITE]) && it->write_socket() == FAILURE) {
			close(it->get_sd());
			it = _clients.erase(it);
			continue;
		}
		it++;
	}
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

int handle_buffer_size(char **argv, int& i) {
	size_t arg_buffer_size;

	if (!argv[i + 1]) {
		std::cout << "Error : -bf needs an argument" << std::endl;
	}
	if(!Syntax::str_is_num(argv[i + 1])) {
		std::cout << "Error : -bf argument must be numerical and positive" << std::endl;
		return (FAILURE);
	}
	arg_buffer_size = strtol(argv[++i], NULL, 10);
	if (arg_buffer_size == 0) {
		std::cout << "Error : -bf argument can't be 0" << std::endl;
		return (FAILURE);
	}
	if (arg_buffer_size > MAX_BUFFER_SIZE) {
		std::cout << "Error : -bf argument can't be more than 1Mo" << std::endl;
		return (FAILURE);
	}
	WebServer::write_buffer_size = arg_buffer_size;
	return (SUCCESS);
}

int check_args(int argc, char **argv, std::string& filepath) {
	if (argc >= 2 && argc <= 5) {
		for (int i = 1; i < argc; i++) {
			std::string argument(argv[i]);
			if (argument == "-v") {
				DEBUG_START(true);
			}
			else if (argument == "-bf") {
				if (handle_buffer_size(argv, i) == FAILURE)
					return (FAILURE);
			}
			else
				filepath = argument;
		}
	}
	return (SUCCESS);
}

int main(int argc, char **argv) {
	WebServer	webserv;
	std::string	filepath;

	if (signal(SIGINT, &WebServer::sigint_handler) == SIG_ERR)
		return (EXIT_FAILURE);
	if (check_args(argc, argv, filepath) == FAILURE)
		return (EXIT_FAILURE);
	if (mkdir("./tmp", 0777) == 0)
		DEBUG_COUT("tmp directory has been created successfully");
	if (filepath.empty())
		filepath = "conf/default.conf";
	if (!webserv.parsing(filepath))
		return EXIT_FAILURE;
	DEBUG_COUT("Config file parsing went well");
	DEBUG_COUT("Write buffer size has been set to : " << WebServer::write_buffer_size);
	DEBUG_COUT(PROGRAM_VERSION << " has started.");
	webserv.setup_servers();
	webserv.routine();
	return (EXIT_SUCCESS);
}
