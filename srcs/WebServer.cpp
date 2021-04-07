/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 01:38:16 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/06 01:38:17 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

bool WebServer::verbose = false;

WebServer::WebServer() : _max_connection(DEFAULT_MAX_CONNECTION),
	_highest_socket(0), _exit(false) {
	_client_sd.assign(_max_connection, 0);
	_servers.assign(1, Server());
}

WebServer::~WebServer() {}

void WebServer::setup_servers() {
	Config test;

	test.setIpAddr("0.0.0.0");
	test.setPort(80);
	for(std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		it->setup_default_server(test);
		set_non_blocking(it->getServerSd());
		_highest_socket = it->getServerSd();
	}
}

void WebServer::accept_connection(const Server& server) {
	int connection;

	connection = accept(server.getServerSd(),
		server.getSockAddr(), server.getAddrLen());
	if (connection < 0) {
		std::cerr << "Failed to grab connection : ";
		std::cerr << std::strerror(errno) << std::endl;
		this->close_sockets();
		exit(EXIT_FAILURE);
	}
	set_non_blocking(connection);
	if (connection == -1)
		return ;
	for (int i = 0; i < _max_connection; i++) {
		if (_client_sd[i] == 0) {
			std::cout << "Connection accepted: FD=" << connection;
			std::cout << " - Slot=" << i << std::endl;
			_client_sd[i] = connection;
			_config_assoc.insert(std::make_pair(connection, server.getConfig()));
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

void WebServer::close_sockets() {
	for(std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		close(it->getServerSd());
	}
	for(std::vector<int>::iterator it = _client_sd.begin(); it != _client_sd.end(); it++)
		close(*it);
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
			std::cerr << "Select error: ";
			std::cerr << std::strerror(errno) << std::endl;
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
		std::cerr << "Fcntl error with F_GETFD : ";
		std::cerr << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(socket_fd, F_SETFL, opts) < 0) {
		std::cerr << "Fcntl error with F_SETFL : ";
		std::cerr << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

void WebServer::build_select_list() {
	FD_ZERO(&_sockets_list);
	for(std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		FD_SET(it->getServerSd(), &_sockets_list);
	}
	for(std::vector<int>::iterator it = _client_sd.begin(); it != _client_sd.end(); it++) {
		FD_SET(*it, &_sockets_list);
		if (*it > _highest_socket)
			_highest_socket = *it;
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
			_exit = true;
		std::stringstream ss;
		const Config& configRef = _config_assoc.find(_client_sd[socket_id])->second;
		ss << "I received this from server with port: ";
		ss << configRef.getPort() << std::endl;
		send(_client_sd[socket_id], ss.str().c_str(), ss.str().size(), 0);
		std::cout << "Response: " << ss.str() << std::endl;
	}
}

void WebServer::read_socks() {
	for(size_t i = 0; i < _servers.size(); i++) {
		if (FD_ISSET(_servers[i].getServerSd(), &_sockets_list))
			this->accept_connection(_servers[i]);
	}
	for (int i = 0; i < _max_connection; i++) {
		if (FD_ISSET(_client_sd[i], &_sockets_list))
			this->handle_data(i);
	}
}

int WebServer::parsing(const std::string &filepath) {
	if (!this->check_config_file(filepath))
		return 0;
	if (!this->check_main_bloc()) {
		std::cerr << "Error during parsing." << std::endl;
		_config_file.close();
		return 0;
	}
	_config_file.close();
	return 1;
}

int WebServer::check_config_file(const std::string &filepath) {
	size_t ext_pos;

	if (filepath.empty()) {
		std::cerr << "The file `" << filepath << " is empty" << std::endl;
		return 0;
	}
	ext_pos = filepath.find(".conf");
	if (ext_pos == std::string::npos
		|| ext_pos != filepath.size() - 5) {
		std::cerr << "The file `" << filepath << "' has a bad extension." << std::endl;
		return 0;
	}
	_config_file.open(filepath.c_str(), std::ios_base::in);
	if (!_config_file) {
		std::cerr << "The file `" << filepath << "' can't be opened: ";
		std::cerr << strerror(errno) << std::endl;
		return 0;
	}
	return (1);
}

int WebServer::check_main_bloc() {
	std::string line_buffer;
	std::vector<std::string> tokens;

	while(_config_file) {
		std::getline(_config_file, line_buffer);
		line_buffer = WebServer::trim_comments(line_buffer);
		line_buffer = WebServer::trim_whitespaces(line_buffer);
		if (line_buffer.empty())
			continue;
		tokens = WebServer::split_whitespaces(line_buffer);
		if (tokens.size() != 2
			|| (tokens.size() == 2
			&& (tokens[0] != "server" || tokens[1] != "{"))) {
			std::cerr << "Unknown instruction `" << line_buffer << "' in main context." << std::endl;
			return 0;
		}
		if (!check_server_bloc()) {
			std::cerr << "Server context parsing error." << std::endl;
			return 0;
		}
	}
	return 1;
}

std::string WebServer::trim_comments(const std::string &line_buffer) {
	std::string new_line;
	size_t hash_char_pos;

	new_line = line_buffer;
	hash_char_pos = new_line.find('#');
	if (hash_char_pos == std::string::npos)
		return new_line;
	return new_line.substr(0, hash_char_pos);
}

std::string WebServer::trim_whitespaces(const std::string& line_buffer) {
	std::string whitespaces;
	std::string new_line;
	size_t start, end;

	whitespaces = " \n\r\t\f\v";
	new_line = line_buffer;
	start = new_line.find_first_not_of(whitespaces);
	if (start != std::string::npos)
		new_line.substr(start);
	end = new_line.find_last_not_of(whitespaces);
	if (end != std::string::npos)
		new_line.substr(end);
	return new_line;
}

std::vector<std::string> WebServer::split_whitespaces(const std::string& line_buffer) {
	return split(line_buffer, " \n\r\t\f\v");
}

std::vector<std::string> WebServer::split(const std::string& line_buffer, const std::string& charset) {
	std::vector<std::string> result;
	std::string token;
	size_t ws_pos, progress_pos = 0;

	while(progress_pos < line_buffer.size()) {
		ws_pos = line_buffer.find(charset, progress_pos);
		if (ws_pos == std::string::npos)
			break;
		token = line_buffer.substr(progress_pos, ws_pos);
		result.push_back(token);
		progress_pos += ws_pos + 1;
	}
	if (progress_pos < line_buffer.size())
		result.push_back(line_buffer.substr(progress_pos));
	return result;
}

int WebServer::trim_semicolon(std::vector<std::string>& tokens) {
	std::string last_token = tokens.back();
	std::string::iterator ite = last_token.end();
	char last_char = *(--ite);

	if (last_token == ";") {
		tokens.pop_back();
		return 1;
	}
	if (last_char != ';')
		return 0;
	last_token.erase(ite);
	return 1;
}

int WebServer::check_server_bloc() {
	std::string line_buffer;
	std::vector<std::string> tokens;
	Config new_config;
	bool closing_bracket = false;
	bool instructions_complete[TOTAL_INSTRUCTIONS] = {false};
	std::string instructions[TOTAL_INSTRUCTIONS] = {"listen", "server_name",
		"error_page", "client_max_body_size", "location", "methods",
		"root", "autoindex", "index", "upload_dir", "cgi"};
	int (WebServer::*instructions_functions[TOTAL_INSTRUCTIONS])(const std::vector<std::string>&,
		Config&) = { &WebServer::parse_listen, &WebServer::parse_server_name, &WebServer::parse_error_page,
		&WebServer::parse_client_max_body_size, &WebServer::parse_location, &WebServer::parse_methods,
		&WebServer::parse_root, &WebServer::parse_autoindex, &WebServer::parse_index,
		&WebServer::parse_upload_dir, &WebServer::parse_cgi };

	while(_config_file) {
		std::getline(_config_file, line_buffer);
		line_buffer = WebServer::trim_comments(line_buffer);
		line_buffer = WebServer::trim_whitespaces(line_buffer);
		if (line_buffer.empty())
			continue;
		if (line_buffer == "}") {
			closing_bracket = true;
			break;
		}
		tokens = WebServer::split_whitespaces(line_buffer);
		for(int i = 0; i < TOTAL_INSTRUCTIONS; i++) {
			if (instructions[i] == tokens[0]) {
				if (instructions_complete[i] && i != LOCATION) {
					std::cerr << "Instruction `" << instructions[i] << "' is present more than once" << std::endl;
					return 0;
				}
				if (i != LOCATION && !trim_semicolon(tokens)) {
					std::cerr << "Instruction `" << instructions[i] << "' needs an ending semicolon." << std::endl;
					return 0;
				}
				if (!(this->*instructions_functions[i])(tokens, new_config)) {
					std::cerr << "Parsing function error." << std::endl;
					return 0;
				}
				instructions_complete[i] = true;
				break;
			}
			if (i == TOTAL_INSTRUCTIONS - 1) {
				std::cerr << "Unknown instruction `" << tokens[0] << "'." << std::endl;
				return 0;
			}
		}
	}
	if (!closing_bracket) {
		std::cerr << "Found `" << line_buffer << "' when expected closing bracket `}'." << std::endl;
		return 0;
	}
	if (!instructions_complete[LISTEN]) {
		std::cerr << "`listen' instruction not found." << std::endl;
		return 0;
	}
	if (!instructions_complete[ROOT]) {
		std::cerr << "`root' instruction not found." << std::endl;
		return 0;
	}
	_configs.push_back(new_config);
	return (1);
}

int WebServer::parse_listen(const std::vector<std::string>& tokens, Config& config) {
	std::string ip_addr;
	std::string port;
	int port_nb;
	size_t colon_pos;

	if (tokens.size() == 1) {
		std::cerr << "`listen' instruction needs an argument." << std::endl;
		return 0;
	}
	if (tokens.size() > 2) {
		std::cerr << "Too much arguments for `listen' instruction." << std::endl;
		return 0;
	}
	colon_pos = tokens[1].find(':');
	if (colon_pos == std::string::npos) {
		std::cerr << "Invalid argument format: should be <IP>:<port>;" << std::endl;
		return 0;
	}
	ip_addr = tokens[1].substr(0, colon_pos);
	if (!check_ip_format(ip_addr)) {
		std::cerr << "Invalid IP format: " << ip_addr << std::endl;
		return 0;
	}
	port = tokens[1].substr(colon_pos + 1, tokens[1].size() - colon_pos - 1);
	if (!is_num(port.c_str())) {
		std::cerr << "Port is not a number: " << port << std::endl;
		return 0;
	}
	port_nb = std::strtol(port.c_str(), NULL, 10);
	if (port_nb <= 0 || port_nb > 65535) {
		std::cerr << "Invalid port number: " << port << std::endl;
		return 0;
	}
	config.setIpAddr(ip_addr);
	config.setPort(port_nb);
	return 1;
}

bool WebServer::is_num(const char* str) {
	for(int i = 0; str[i]; i++) {
		if (!std::isdigit(str[i]))
			return false;
	}
	return true;
}

int WebServer::check_ip_format(const std::string& ip) {
 	std::vector<std::string> nums = split(ip, ".");
 	int num;

 	if (nums.size() != 4)
 		return 0;
 	for(int i = 0; i < 4; i++) {
 		if (!is_num(nums[i].c_str()))
 			return 0;
 		num = std::strtol(nums[i].c_str(), NULL, 10);
 		if (num < 0 || num > 255)
 			return 0;
 	}
 	return 1;
}

int WebServer::parse_server_name(const std::vector<std::string>& tokens, Config& config) {
	if (tokens.size() == 1) {
		std::cerr << "`server_name' instruction needs an argument." << std::endl;
		return 0;
	}
	std::list<std::string> server_names;
	for(std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		server_names.push_back(*it);
	}
	config.setServerName(server_names);
	return 1;
}

int WebServer::parse_error_page(const std::vector<std::string>& tokens, Config& config) {
	if (tokens.size() < 3) {
		std::cerr << "`error_page' instruction needs at least 2 arguments." << std::endl;
		return 0;
	}
	std::list<int> error_codes;
	std::string error_path;
	for (std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		if (*it != tokens.back()) {
			if (!is_num(it->c_str())) {
				std::cerr << "`'error_page' error codes must be numerical" << std::endl;
				return 0;
			}
			//TODO: is error_code (check if error_code exist)
			error_codes.push_back(std::strtol(it->c_str(), NULL, 10));
		}
		else {
			//TODO: check open ? check format ?
			error_path = *it;
		}
	}
	config.setErrorPageCodes(error_codes);
	config.setErrorPagePath(error_path);
	return 1;
}

int WebServer::parse_client_max_body_size(const std::vector<std::string>& tokens, Config& config) {
	if (tokens.size() == 1) {
		std::cerr << "`client_max_body_size' instruction needs one argument." << std::endl;
		return 0;
	}
	if (tokens.size() > 2) {
		std::cerr << "`Too much arguments for `client_max_body_size' instruction." << std::endl;
		return 0;
	}
	if (!is_num(tokens.back().c_str())) {
		std::cerr << "`client_max_body_size' argument must be numerical." << std::endl;
		return 0;
	}
	int client_max_body_size = std::strtol(tokens.back().c_str(), NULL, 10);
	if (client_max_body_size == 0) {
		std::cerr << "`client_max_body_size' argument can't be 0." << std::endl;
		return 0;
	}
	config.setClientMaxBodySize(client_max_body_size);
	return 1;
}

int WebServer::parse_location(const std::vector<std::string>& tokens, Config& config) {
	//TODO
}

int WebServer::parse_methods(const std::vector<std::string>& tokens, Config& config) {

}

int WebServer::parse_root(const std::vector<std::string>& tokens, Config& config) {

}

int WebServer::parse_autoindex(const std::vector<std::string>& tokens, Config& config) {

}

int WebServer::parse_index(const std::vector<std::string>& tokens, Config& config) {

}

int WebServer::parse_upload_dir(const std::vector<std::string>& tokens, Config& config) {

}

int WebServer::parse_cgi(const std::vector<std::string>& tokens, Config& config) {

}


