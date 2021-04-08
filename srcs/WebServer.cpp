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
std::string WebServer::methods_array[TOTAL_METHODS] = {"GET", "HEAD",
	"POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"};

WebServer::WebServer() : _max_connection(DEFAULT_MAX_CONNECTION),
	_highest_socket(0), _exit(false) {
	_client_sd.assign(_max_connection, 0);
}

WebServer::~WebServer() {}

void WebServer::setup_servers() {
	std::set<std::pair<std::string, int> > default_servers;
	std::pair<std::string, int> ip_port;
	Server new_server;

	for(std::vector<Config>::iterator it = _configs.begin(); it != _configs.end(); it++) {
		ip_port = std::make_pair(it->getIpAddr(), it->getPort());
		if (default_servers.find(ip_port) == default_servers.end()) {
			default_servers.insert(ip_port);
			new_server.setConfig(&(*it));
			_servers.push_back(new_server);
		}
	}
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
		const Config* configRef = _config_assoc.find(_client_sd[socket_id])->second;
		ss << "I received this from server with port: ";
		ss << configRef->getPort() << std::endl;
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
		std::cerr << "Error during config file parsing." << std::endl;
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
	std::string usage("Usage: `server {'");

	while(_config_file) {
		std::getline(_config_file, line_buffer);
		line_buffer = WebServer::trim_comments(line_buffer);
		line_buffer = WebServer::trim_whitespaces(line_buffer);
		tokens = WebServer::split_whitespaces(line_buffer);
		if (line_buffer.empty() || tokens.empty())
			continue;
		if (tokens[0] != "server") {
			std::cerr << "Unknown instruction `" << line_buffer << "' in main context." << std::endl;
			std::cerr << usage << std::endl;
			return 0;
		}
		if (tokens.size() == 1 || (tokens.size() == 2 && tokens[1] != "{")) {
			std::cerr << "Missing opening bracket for `server' instruction." << std::endl;
			std::cerr << usage << std::endl;
			return 0;
		}
		if (tokens.size() > 2) {
			std::cerr << "Too much arguments for `server' instruction: " << line_buffer << std::endl;
			std::cerr << usage << std::endl;
			return 0;
		}
		if (!check_server_bloc())
			return 0;
	}
	if (_configs.empty()) {
		std::cerr << "Config file is empty." << std::endl;
		return 0;
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
		new_line = new_line.substr(start);
	end = new_line.find_last_not_of(whitespaces);
	if (end != std::string::npos)
		new_line = new_line.substr(0, end + 1);
	return new_line;
}

std::vector<std::string> WebServer::split_whitespaces(const std::string& line_buffer) {
	return split(line_buffer, " \n\r\t\f\v");
}

std::vector<std::string> WebServer::split(const std::string& line_buffer, const std::string& charset) {
	std::vector<std::string> result;
	std::string token;
	size_t ws_pos, progress_pos = 0, wd_len;;

	while(progress_pos < line_buffer.size()) {
		ws_pos = line_buffer.find_first_of(charset, progress_pos);
		if (ws_pos == std::string::npos)
			break;
		wd_len = ws_pos - progress_pos;
		token = line_buffer.substr(progress_pos, wd_len);
		progress_pos += wd_len + 1;
		if(!token.empty())
			result.push_back(token);
	}
	if (progress_pos < line_buffer.size())
		result.push_back(line_buffer.substr(progress_pos));
	return result;
}

int WebServer::trim_semicolon(std::vector<std::string>& tokens) {
	std::string last_token = tokens.back();
	std::string::iterator ite = tokens.back().end();
	char last_char = *(--ite);

	if (last_token == ";") {
		tokens.pop_back();
		return 1;
	}
	if (last_char != ';')
		return 0;
	tokens.back().erase(ite);
	return 1;
}

int WebServer::check_server_bloc() {
	std::string line_buffer;
	std::vector<std::string> tokens;
	Config new_config;
	bool closing_bracket = false;
	bool instructions_complete[TOTAL_SERVER_INSTRUCTIONS] = {false};
	std::string instructions[TOTAL_SERVER_INSTRUCTIONS] = {"listen", "server_name",
		"error_page", "client_max_body_size", "location", "methods",
		"root", "autoindex", "index", "upload_dir", "cgi"};
	int (WebServer::*instructions_functions[TOTAL_SERVER_INSTRUCTIONS])(const std::vector<std::string>&,
		Config&) = { &WebServer::parse_listen, &WebServer::parse_server_name, &WebServer::parse_error_page,
		&WebServer::parse_client_max_body_size, &WebServer::parse_location, &WebServer::parse_methods_config,
		&WebServer::parse_root_config, &WebServer::parse_autoindex_config, &WebServer::parse_index_config,
		&WebServer::parse_upload_dir, &WebServer::parse_cgi_config };

	while(_config_file) {
		std::getline(_config_file, line_buffer);
		line_buffer = WebServer::trim_comments(line_buffer);
		line_buffer = WebServer::trim_whitespaces(line_buffer);
		tokens = WebServer::split_whitespaces(line_buffer);
		if (line_buffer.empty() || tokens.empty())
			continue;
		if (tokens[0] == "}") {
			if (tokens.size() > 1) {
				std::cerr << "Closing bracket is not the single instruction on the line." << std::endl;
				return 0;
			}
			closing_bracket = true;
			break;
		}
		for(int i = 0; i < TOTAL_SERVER_INSTRUCTIONS; i++) {
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
			if (i == TOTAL_SERVER_INSTRUCTIONS - 1) {
				std::cerr << "Unknown instruction `" << tokens[0] << "' in server context." << std::endl;
				return 0;
			}
		}
	}
	if (!closing_bracket) {
		std::cerr << "Found `" << line_buffer << "' when expected closing bracket `}' in server bloc." << std::endl;
		return 0;
	}
	if (!instructions_complete[LISTEN]) {
		std::cerr << "`listen' instruction not found in server bloc." << std::endl;
		return 0;
	}
	if (!instructions_complete[ROOT]) {
		std::cerr << "`root' instruction not found in server bloc." << std::endl;
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
	std::string usage("Usage: 'listen <ip:port>;'\nip format : [0-255].[0-255].[0-255].[0-255] - port number: [1-65535]");

	if (tokens.size() == 1) {
		std::cerr << "`listen' instruction needs an argument." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	if (tokens.size() > 2) {
		std::cerr << "Too much arguments for `listen' instruction." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	colon_pos = tokens[1].find(':');
	if (colon_pos == std::string::npos) {
		std::cerr << "Invalid ip:port format." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	ip_addr = tokens[1].substr(0, colon_pos);
	if (!check_ip_format(ip_addr)) {
		std::cerr << "Invalid IP format: `" << ip_addr << "'" << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	port = tokens[1].substr(colon_pos + 1, tokens[1].size() - colon_pos - 1);
	if (!is_num(port.c_str())) {
		std::cerr << "Port is not a number: `" << port << "'" << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	port_nb = std::strtol(port.c_str(), NULL, 10);
	if (port_nb <= 0 || port_nb > 65535) {
		std::cerr << "Invalid port number: `" << port << "'" << std::endl;
		std::cerr << usage << std::endl;
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
	std::list<std::string> server_names;
	std::string usage("Usage: 'server_name <string1> <string2> [...];'");

	if (tokens.size() == 1) {
		std::cerr << "`server_name' instruction needs an argument." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	for(std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		server_names.push_back(*it);
	}
	config.setServerName(server_names);
	return 1;
}

int WebServer::parse_error_page(const std::vector<std::string>& tokens, Config& config) {
	std::list<int> error_codes;
	std::string error_path;
	int error_code;
	std::string usage("Usage: 'error_page <400 401 504....> [/absolute/path];'");

	if (tokens.size() < 3) {
		std::cerr << "`error_page' instruction needs at least 2 arguments." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	for (std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		if (*it != tokens.back()) {
			if (!is_num(it->c_str())) {
				std::cerr << "`'error_page' error codes must be numerical." << std::endl;
				std::cerr << usage << std::endl;
				return 0;
			}
			error_code = std::strtol(it->c_str(), NULL, 10);
			if (!is_error_code(error_code)) {
				std::cerr << "Argument `" << *it << "' is not a valid error code." <<std::endl;
				std::cerr << usage << std::endl;
				return 0;
			}
			error_codes.push_back(error_code);
		}
		else {
			if (!check_path(*it)) {
				std::cerr << "Path error in `error_page' instruction." << std::endl;
				std::cerr << usage << std::endl;
				return 0;
			}
			error_path = *it;
		}
	}
	config.setErrorPageCodes(error_codes);
	config.setErrorPagePath(error_path);
	return 1;
}

int WebServer::check_path(const std::string &path) {
	std::ifstream test(path.c_str());
	if (!test) {
		std::cerr << "The path `" << path << "' can't be opened: ";
		std::cerr << strerror(errno) << std::endl;
		return 0;
	}
	test.close();
	return 1;
}

int WebServer::parse_client_max_body_size(const std::vector<std::string>& tokens, Config& config) {
	int client_max_body_size;
	std::string usage("Usage: 'client_max_body_size <positive, non null, numerical value>;'");

	if (tokens.size() == 1) {
		std::cerr << "`client_max_body_size' instruction needs one argument." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	if (tokens.size() > 2) {
		std::cerr << "`Too much arguments for `client_max_body_size' instruction." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	if (!is_num(tokens.back().c_str())) {
		std::cerr << "`client_max_body_size' argument must be numerical." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	client_max_body_size = std::strtol(tokens.back().c_str(), NULL, 10);
	if (client_max_body_size == 0) {
		std::cerr << "`client_max_body_size' argument can't be 0." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	config.setClientMaxBodySize(client_max_body_size);
	return 1;
}

int WebServer::parse_location(const std::vector<std::string>& tokens, Config& config) {
	std::string usage("Usage: 'location [/random/path/name] {'");
	std::string path, bracket, line_buffer;
	std::vector<std::string> location_tokens;
	Location new_location;
	bool closing_bracket = false;
	bool instructions_complete[TOTAL_LOCATION_INSTRUCTIONS] = {false};
	std::string instructions[TOTAL_LOCATION_INSTRUCTIONS] = {"methods",
		"root", "autoindex", "index", "cgi"};
	int (WebServer::*instructions_functions[TOTAL_LOCATION_INSTRUCTIONS])(const std::vector<std::string>&,
		Location&) = { &WebServer::parse_methods_location, &WebServer::parse_root_location,
		&WebServer::parse_autoindex_location, &WebServer::parse_index_location,
		&WebServer::parse_cgi_location};

	if(tokens.size() != 3) {
		std::cerr << "`location' instruction needs one argument and an opening bracket." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	path = tokens[1];
	bracket = tokens[2];
	if (path[0] != '/') {
		std::cerr << "Wrong path format in `location' instruction." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	if (bracket != "{") {
		std::cerr << "`location' instruction needs to end with an opening bracket." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	new_location.setPath(path);
	while(_config_file) {
		std::getline(_config_file, line_buffer);
		line_buffer = WebServer::trim_comments(line_buffer);
		line_buffer = WebServer::trim_whitespaces(line_buffer);
		location_tokens = WebServer::split_whitespaces(line_buffer);
		if (line_buffer.empty() || location_tokens.empty())
			continue;
		if (location_tokens[0] == "}") {
			if (location_tokens.size() > 1) {
				std::cerr << "Closing bracket is not the single instruction on the line." << std::endl;
				return 0;
			}
			closing_bracket = true;
			break;
		}
		for(int i = 0; i < TOTAL_LOCATION_INSTRUCTIONS; i++) {
			if (instructions[i] == location_tokens[0]) {
				if (instructions_complete[i]) {
					std::cerr << "Instruction `" << instructions[i] << "' is present more than once" << std::endl;
					return 0;
				}
				if (!trim_semicolon(location_tokens)) {
					std::cerr << "Instruction `" << instructions[i] << "' needs an ending semicolon." << std::endl;
					return 0;
				}
				if (!(this->*instructions_functions[i])(location_tokens, new_location)) {
					std::cerr << "Parsing function error." << std::endl;
					return 0;
				}
				instructions_complete[i] = true;
				break;
			}
			if (i == TOTAL_LOCATION_INSTRUCTIONS - 1) {
				std::cerr << "Unknown instruction `" << location_tokens[0] << "' in location context." << std::endl;
				return 0;
			}
		}
	}
	if (!closing_bracket) {
		std::cerr << "Found `" << line_buffer << "' when expected closing bracket `}' in location bloc." << std::endl;
		return 0;
	}
	config.addLocation(new_location);
	return 1;
}

int WebServer::method_index(const std::string& method) {
	for(int i = 0; i < TOTAL_METHODS; i++) {
		if (method == methods_array[i])
			return i;
	}
	return -1;
}

int WebServer::parse_methods(const std::vector<std::string>& tokens, AConfig& config) {
	std::list<std::string> methods;
	bool methods_complete[TOTAL_METHODS] = {false};
	int method_idx;
	std::string usage("Usage: 'methods GET and/or PUT and/or HEAD [...];'");

	if (tokens.size() == 1) {
		std::cerr << "`methods' instruction needs at least one argument." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	for (std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		method_idx = method_index(*it);
		if (method_idx == -1) {
			std::cerr << "Argument `" << *it << "' is not a valid method." << std::endl;
			std::cerr << usage << std::endl;
			return 0;
		}
		if (methods_complete[method_idx]) {
			std::cerr << "Method `" << *it << "' is present more than once." << std::endl;
			std::cerr << usage << std::endl;
			return 0;
		}
		methods_complete[method_idx] = true;
		methods.push_back(*it);
	}
	config.setMethods(methods);
	return 1;
}

int WebServer::parse_methods_config(const std::vector<std::string>& tokens, Config& config) {
	return parse_methods(tokens, config);
}

int WebServer::parse_methods_location(const std::vector<std::string>& tokens, Location& location) {
	return parse_methods(tokens, location);
}

int WebServer::parse_root(const std::vector<std::string>& tokens, AConfig& config) {
	std::string usage("Usage: 'root [/absolute/path];'");
	std::string path;

	if (tokens.size() == 1) {
		std::cerr << "`root' instruction needs one argument." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	if (tokens.size() > 2) {
		std::cerr << "Too much arguments for `root' instruction." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	path = tokens[1];
	if (path[0] != '/') {
		std::cerr << "Argument `" << path << "' is not an absolute path." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	if (!check_path(path)) {
		std::cerr << "Path error in `root' instruction." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	config.setRoot(path);
	return 1;
}

int WebServer::parse_root_config(const std::vector<std::string>& tokens, Config& config) {
	return parse_root(tokens, config);
}

int WebServer::parse_root_location(const std::vector<std::string>& tokens, Location& location) {
	return parse_root(tokens, location);
}

int WebServer::parse_autoindex(const std::vector<std::string>& tokens, AConfig& config) {
	std::string usage("Usage: 'autoindex <on/off>;'");
	std::string response;

	if (tokens.size() == 1) {
		std::cerr << "`autoindex' instruction needs one argument." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	if (tokens.size() > 2) {
		std::cerr << "Too much arguments for `autoindex' instruction." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	response = tokens[1];
	if (response != "on" && response != "off") {
		std::cerr << "Argument `" << response << "' is invalid for `autoindex' instruction." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	config.setAutoindex(response == "on");
	return 1;
}

int WebServer::parse_autoindex_config(const std::vector<std::string>& tokens, Config& config) {
	return parse_autoindex(tokens, config);
}

int WebServer::parse_autoindex_location(const std::vector<std::string>& tokens, Location& location) {
	return parse_autoindex(tokens, location);
}

int WebServer::parse_index(const std::vector<std::string>& tokens, AConfig& config) {
	std::string usage("Usage: 'index <page1>.html <page2>.php [...];'");
	std::list<std::string> index_pages;

	if (tokens.size() == 1) {
		std::cerr << "`index' instruction needs at least one argument." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	for (std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		index_pages.push_back(*it);
	}
	config.setIndex(index_pages);
	return 1;
}

int WebServer::parse_index_config(const std::vector<std::string>& tokens, Config& config) {
	return parse_index(tokens, config);
}

int WebServer::parse_index_location(const std::vector<std::string>& tokens, Location& location) {
	return parse_index(tokens, location);
}

int WebServer::parse_upload_dir(const std::vector<std::string>& tokens, Config& config) {
	std::string usage("Usage: 'upload_dir [/absolute/path];'");
	std::string path;

	if (tokens.size() == 1) {
		std::cerr << std::cerr << "`upload_dir' instruction needs one argument." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	if (tokens.size() > 2) {
		std::cerr << "Too much arguments for `upload_dir' instruction." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	path = tokens[1];
	if (path[0] != '/') {
		std::cerr << "Argument `" << path << "' is not an absolute path." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	if (!check_path(path)) {
		std::cerr << "Path error in `upload_dir' instruction." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	config.setUploadDir(path);
	return 1;
}

int WebServer::parse_cgi(const std::vector<std::string>& tokens, AConfig& config) {
	std::string usage("Usage: 'cgi [*.<ext>] [/absolute/path];'");
	std::string cgi_file_ext, cgi_path;

	if (tokens.size() != 3) {
		std::cerr << "`cgi' instruction needs 3 arguments." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	cgi_file_ext = tokens[1];
	cgi_path = tokens[2];
	if (cgi_file_ext.size() < 3
		|| (cgi_file_ext.size() > 2 && (cgi_file_ext[0] != '*' || cgi_file_ext[1] != '.'))) {
		std::cerr << "Argument `" << cgi_file_ext << "' is not a valid format for cgi file extension." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	if (cgi_path[0] != '/') {
		std::cerr << "Argument `" << cgi_path << "' is not an absolute path." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	if (!check_path(cgi_path)) {
		std::cerr << "Path error in `cgi' instruction." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	config.setCgiExtension(cgi_file_ext.substr(2));
	config.setCgiPath(cgi_path);
	return 1;
}

int WebServer::parse_cgi_config(const std::vector<std::string>& tokens, Config& config) {
	return parse_cgi(tokens, config);
}

int WebServer::parse_cgi_location(const std::vector<std::string>& tokens, Location& location) {
	return parse_cgi(tokens, location);
}

bool WebServer::is_informational_code(int code) {
	return code == 100 || code == 101;
}

bool WebServer::is_successful_code(int code) {
	return code >= 200 && code <= 206;
}

bool WebServer::is_redirection_code(int code) {
	return (code >= 300 && code <= 305) || code == 307;
}

bool WebServer::is_client_error_code(int code) {
	return (code >= 400 && code <= 417) || code == 426;
}

bool WebServer::is_server_error_code(int code) {
	return code >= 500 && code <= 505;
}

bool WebServer::is_error_code(int code) {
	return is_server_error_code(code) || is_client_error_code(code);
}
