/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParsing.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chris <chris@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/15 20:59:19 by louise            #+#    #+#             */
/*   Updated: 2021/04/22 16:40:54 by chris            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParsing.hpp"

int
ConfigParsing::check_config_file(const std::string &filepath, std::ifstream& config_file) {
	size_t ext_pos;

	if (filepath.empty()) {
		std::cerr << "The file `" << filepath << " is empty" << std::endl;
		return FAILURE;
	}
	ext_pos = filepath.find(".conf");
	if (ext_pos == std::string::npos
		|| ext_pos != filepath.size() - 5) {
		std::cerr << "The file `" << filepath << "' has a bad extension." << std::endl;
		return FAILURE;
	}
	config_file.open(filepath.c_str(), std::ios_base::in);
	if (!config_file) {
		std::cerr << "The file `" << filepath << "' can't be opened: ";
		std::cerr << strerror(errno) << std::endl;
		return FAILURE;
	}
	return SUCCESS;
}

int
ConfigParsing::check_main_bloc(std::ifstream& config_file, std::list<VirtualServer>& virtual_servers) {
	std::vector<std::string> tokens;
	std::string usage("Usage: `server {'");

	while(config_file) {
		tokens = format_instruction_line(config_file);
		if (tokens.empty())
			continue;
		if (tokens[INSTRUCTION_TOKEN] != "server")
			return unknown_instruction(tokens[INSTRUCTION_TOKEN], "main");
		if (tokens.size() == 1 || (tokens.size() == 2 && tokens[1] != "{"))
			return(bracket_not_found(tokens[INSTRUCTION_TOKEN], "main", OPENING_BRACKET));
		if (tokens.size() > 2)
			return (invalid_number_arguments(1, tokens.size() - 1, "server", usage));
		if (!check_server_bloc(config_file, virtual_servers))
			return FAILURE;
	}
	if (virtual_servers.empty()) {
		std::cerr << "Config file is empty." << std::endl;
		return FAILURE;
	}
	return SUCCESS;
}

std::vector<std::string>
ConfigParsing::format_instruction_line(std::ifstream& config_file) {
	std::string line_buffer;

	std::getline(config_file, line_buffer);
	line_buffer = Syntax::trim_comments(line_buffer);
	line_buffer = Syntax::trim_whitespaces(line_buffer);
	return Syntax::split(line_buffer, WHITESPACES);
}

int
ConfigParsing::server_instruction_handler(std::vector<std::string>& tokens,
	VirtualServer& virtual_server, std::set<server_instruction_t>& instructions_filled) {
	int (*instructions_functions[])(const std::vector<std::string>&, VirtualServer&) = {
		&parse_listen, &parse_server_name, &parse_error_page,
		&parse_client_max_body_size, &parse_upload_dir};

	for(size_t i = LISTEN; i < TOTAL_SERVER_INSTRUCTIONS - 1; i++) {
		if (Syntax::server_instructions_tab[i].name == tokens[INSTRUCTION_TOKEN]) {
			if (Syntax::server_instructions_tab[i].instruction_index != LOCATION_INSTRUCTION
				&& instructions_filled.find(Syntax::server_instructions_tab[i].instruction_index) != instructions_filled.end())
				return (multiple_instructions_found(Syntax::server_instructions_tab[i].name));
			if (!Syntax::trim_semicolon(tokens))
				return semicolon_not_found(Syntax::server_instructions_tab[i].name);
			if (!(*instructions_functions[i])(tokens, virtual_server))
				return FAILURE;
			instructions_filled.insert(Syntax::server_instructions_tab[i].instruction_index);
			break;
		}
	}
	return SUCCESS;
}

int
ConfigParsing::location_instructions_handler(std::vector<std::string>& tokens,
	Location& location, std::set<location_instruction_t>& instructions_filled) {
	int (*instructions_functions[TOTAL_LOCATION_INSTRUCTIONS])(const std::vector<std::string>&, Location&) =
		{ &parse_root, &parse_methods, &parse_index, &parse_cgi, &parse_autoindex };

	for(int i = 0; i < TOTAL_LOCATION_INSTRUCTIONS; i++) {
		if (Syntax::location_instructions_tab[i].name == tokens[INSTRUCTION_TOKEN]) {
			if (instructions_filled.find(Syntax::location_instructions_tab[i].instruction_index) != instructions_filled.end())
				return (multiple_instructions_found(Syntax::location_instructions_tab[i].name));
			if (!Syntax::trim_semicolon(tokens))
				return semicolon_not_found(Syntax::location_instructions_tab[i].name);
			if (!(*instructions_functions[i])(tokens, location))
				return FAILURE;
			instructions_filled.insert(Syntax::location_instructions_tab[i].instruction_index);
			break;
		}
	}
	std::cout << "THERE" << location.get_cgi_extension() << std::endl;
	return SUCCESS;
}

int
ConfigParsing::check_server_bloc(std::ifstream& config_file, std::list<VirtualServer>& virtual_servers) {
	std::vector<std::string> tokens;
	std::set<server_instruction_t> instructions_filled;
	std::set<location_instruction_t> default_location_filled;
	VirtualServer new_virtual_server;
	Location default_location;
	bool closing_bracket = false;

	while(config_file) {
		tokens = format_instruction_line(config_file);
		if (tokens.empty())
			continue;
		if (tokens[INSTRUCTION_TOKEN] == "}") {
			if (tokens.size() > 1)
				return(closing_bracket_not_alone(tokens[INSTRUCTION_TOKEN], "server"));
			closing_bracket = true;
			break;
		}
		if (tokens[INSTRUCTION_TOKEN] == "location") {
			if (check_location_bloc(config_file, tokens, new_virtual_server) == FAILURE)
				return FAILURE;
		}
		else if (is_server_instruction(tokens[INSTRUCTION_TOKEN])) {
			if (server_instruction_handler(tokens, new_virtual_server, instructions_filled) == FAILURE)
				return FAILURE;
		} else if (is_location_instruction(tokens[INSTRUCTION_TOKEN])) {
			if (location_instructions_handler(tokens, default_location, default_location_filled) == FAILURE)
				return FAILURE;
		}
		else
			return unknown_instruction(tokens[INSTRUCTION_TOKEN], "server");
	}
	new_virtual_server.add_location(default_location);
	if (!closing_bracket)
		return(bracket_not_found(tokens[INSTRUCTION_TOKEN], "server", CLOSING_BRACKET));
	if (instructions_filled.find(LISTEN) == instructions_filled.end())
		return (mandatory_instruction_not_found("listen"));
	if (default_location_filled.find(ROOT) == default_location_filled.end())
		return (mandatory_instruction_not_found("root"));
	virtual_servers.push_back(new_virtual_server);
	return (SUCCESS);
}

int
ConfigParsing::check_location_bloc(std::ifstream& config_file,
	const std::vector<std::string>& tokens, VirtualServer& virtual_server) {
	std::string usage("Usage: 'location [/random/path/name] {'");
	std::string path, bracket;
	std::vector<std::string> location_tokens;
	Location new_location;
	bool closing_bracket = false;
	std::set<location_instruction_t> instructions_filled;

	if(tokens.size() != 3)
		return (invalid_number_arguments(2, tokens.size() - 1, tokens[INSTRUCTION_TOKEN], usage));
	path = tokens[1];
	bracket = tokens[2];
	if (path[0] != '/')
		return(wrong_path_format(tokens[INSTRUCTION_TOKEN], usage));
	if (bracket != "{")
		return(bracket_not_found(bracket, "location", OPENING_BRACKET));
	new_location.set_path(path);
	while(config_file) {
		location_tokens = format_instruction_line(config_file);
		if (location_tokens.empty())
			continue;
		if (location_tokens[INSTRUCTION_TOKEN] == "}") {
			if (location_tokens.size() > 1)
				return(closing_bracket_not_alone(tokens[0], tokens[0]));
			closing_bracket = true;
			break;
		}
		if (is_location_instruction(location_tokens[INSTRUCTION_TOKEN])) {
			if (location_instructions_handler(location_tokens, new_location, instructions_filled) == FAILURE)
				return FAILURE;
		}
		else
			return unknown_instruction(tokens[INSTRUCTION_TOKEN], "location");
	}
	if (!closing_bracket)
		return(bracket_not_found(location_tokens[INSTRUCTION_TOKEN], "server", OPENING_BRACKET));
	std::cout << "OVER THERE" << new_location.get_cgi_extension() << std::endl;
	virtual_server.add_location(new_location);
	return SUCCESS;
}

int
ConfigParsing::parse_listen(const std::vector<std::string>& tokens, VirtualServer& virtual_server) {
	std::string ip_addr = "0.0.0.0";
	std::string port;
	int port_nb;
	size_t colon_pos;
	std::string usage("Usage: 'listen <[optional_ip:]port>;'");

	usage += "ip format : [0-255].[0-255].[0-255].[0-255] - port number: [1-65535]";
	if (tokens.size() != 2)
		return (invalid_number_arguments(2, tokens.size() - 1, tokens[0], usage));
	colon_pos = tokens[1].find(':');
	if (colon_pos != std::string::npos) {
		ip_addr = tokens[1].substr(0, colon_pos);
		if (!Syntax::check_ip_format(ip_addr)) {
			std::cerr << "Invalid IP format: `" << ip_addr << "'" << std::endl;
			std::cerr << usage << std::endl;
			return FAILURE;
		}
		port = tokens[1].substr(colon_pos + 1);
	}
	else
		port = tokens[1];
	if (!Syntax::str_is_num(port))
		return (argument_not_numerical(port, tokens[0], usage));
	port_nb = std::strtol(port.c_str(), NULL, 10);
	if (port_nb <= 0 || port_nb > 65535) {
		std::cerr << "Invalid port number: `" << port << "'" << std::endl;
		std::cerr << usage << std::endl;
		return FAILURE;
	}
	virtual_server.set_ip_addr(ip_addr);
	virtual_server.set_port(port_nb);
	virtual_server.set_port_str(port);
	return SUCCESS;
}

int
ConfigParsing::parse_server_name(const std::vector<std::string>& tokens, VirtualServer& virtual_server) {
	std::list<std::string> server_names;
	std::string usage("Usage: 'server_name <string1> <string2> [...];'");

	if (tokens.size() < 2)
		return (instruction_need_at_least(1, tokens[0], usage));
	for(std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		server_names.push_back(*it);
	}
	virtual_server.set_server_names(server_names);
	return SUCCESS;
}

int
ConfigParsing::parse_error_page(const std::vector<std::string>& tokens, VirtualServer& virtual_server) {
	std::list<status_code_t> error_codes;
	std::string error_path;
	int error_code_int;
	size_t	i(0);
	std::string usage("Usage: 'error_page <400 401 504....> [/absolute/or/relative/path];'");

	if (tokens.size() < 3)
		return (instruction_need_at_least(2, tokens[0], usage));
	for (std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		if (*it != tokens.back()) {
			if (!Syntax::str_is_num(*it))
				return (argument_not_numerical(*it, tokens[0], usage));
			error_code_int = std::strtol(it->c_str(), NULL, 10);
			if (!Syntax::is_error_code(error_code_int)) {
				std::cerr << "Argument `" << *it << "' is not a valid error code." <<std::endl;
				std::cerr << usage << std::endl;
				return FAILURE;
			}
			while (Syntax::status_codes_tab[i].code_index != TOTAL_STATUS_CODE
				&& Syntax::status_codes_tab[i].code_int != error_code_int)
				i++;
			error_codes.push_back(Syntax::status_codes_tab[i].code_index);
		}
		else {
			if (!check_instruction_path(*it, tokens[0], usage))
				return FAILURE;
			error_path = *it;
		}
	}
	virtual_server.set_error_page_codes(error_codes);
	virtual_server.set_error_page_path(error_path);
	return SUCCESS;
}

int
ConfigParsing::parse_client_max_body_size(const std::vector<std::string>& tokens, VirtualServer& virtual_server) {
	unsigned long client_max_body_size;
	std::string usage("Usage: 'client_max_body_size <positive, non null, numerical value>;'");

	if (tokens.size() != 2)
		return (invalid_number_arguments(1, tokens.size() - 1, tokens[0], usage));
	if (!Syntax::str_is_num(tokens.back()))
		return (argument_not_numerical(tokens.back(), tokens[0], usage));
	client_max_body_size = std::strtol(tokens.back().c_str(), NULL, 10);
	if (client_max_body_size == 0) {
		std::cerr << "`client_max_body_size' argument can't be null." << std::endl;
		std::cerr << usage << std::endl;
		return FAILURE;
	}
	virtual_server.set_client_max_body_size(client_max_body_size);
	return SUCCESS;
}
int
ConfigParsing::parse_upload_dir(const std::vector<std::string>& tokens, VirtualServer& virtual_server) {
	std::string usage("Usage: 'upload_dir [/absolute/or/relative/path];'");
	std::string path;

	if (tokens.size() != 2)
		return (invalid_number_arguments(1, tokens.size() - 1, tokens[0], usage));
	path = tokens[1];
	if (!check_instruction_path(path, tokens[0], usage))
		return FAILURE;
	virtual_server.set_upload_dir(path);
	return SUCCESS;
}

int
ConfigParsing::parse_root(const std::vector<std::string>& tokens, Location& location) {
	std::string usage("Usage: 'root [/absolute/or/relative/path];'");
	std::string path;

	if (tokens.size() != 2)
		return (invalid_number_arguments(1, tokens.size() - 1, tokens[0], usage));
	path = tokens[1];
	if (!check_instruction_path(path, tokens[0], usage))
		return FAILURE;
	location.set_root(path);
	return SUCCESS;
}

int
ConfigParsing::parse_methods(const std::vector<std::string>& tokens, Location& location) {
	std::list<std::string> methods;
	bool methods_complete[DEFAULT_METHOD] = {false};
	int method_idx;
	std::string usage("Usage: 'methods GET and/or PUT and/or HEAD [...];'");

	if (tokens.size() < 2)
		return (instruction_need_at_least(1, tokens[0], usage));
	for (std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		method_idx = Syntax::fetch_method_index(*it);
		if (method_idx == -1) {
			std::cerr << "Argument `" << *it << "' is not a valid method." << std::endl;
			std::cerr << usage << std::endl;
			return FAILURE;
		}
		if (methods_complete[method_idx]) {
			std::cerr << "Method `" << *it << "' is present more than once." << std::endl;
			std::cerr << usage << std::endl;
			return FAILURE;
		}
		methods_complete[method_idx] = true;
		methods.push_back(*it);
	}
	location.set_methods(methods);
	return SUCCESS;
}

int
ConfigParsing::parse_index(const std::vector<std::string>& tokens, Location& location) {
	std::string usage("Usage: 'index <page1>.html <page2>.php [...];'");
	std::list<std::string> index_pages;

	if (tokens.size() < 2)
		return (instruction_need_at_least(1, tokens[0], usage));
	for (std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		index_pages.push_back(*it);
	}
	location.set_index(index_pages);
	return SUCCESS;
}

int
ConfigParsing::parse_cgi(const std::vector<std::string>& tokens, Location& location) {
	std::cout << "HERE" << std::endl;
	std::string usage("Usage: 'cgi [*.<ext>] [/absolute/or/relative/path];'");
	std::string cgi_file_ext, cgi_path;

	if (tokens.size() != 3)
		return (invalid_number_arguments(2, tokens.size() - 1, tokens[0], usage));
	cgi_file_ext = tokens[1];
	cgi_path = tokens[2];
	if (cgi_file_ext.size() < 3
		|| (cgi_file_ext.size() > 2 && (cgi_file_ext[0] != '*' || cgi_file_ext[1] != '.'))) {
		std::cerr << "Argument `" << cgi_file_ext;
		std::cerr << "' is not a valid format for cgi file extension." << std::endl;
		std::cerr << usage << std::endl;
		return FAILURE;
	}
	if (!check_instruction_path(cgi_path, tokens[0], usage))
		return FAILURE;
	location.set_cgi_extension(cgi_file_ext.substr(1));
	location.set_cgi_path(cgi_path);
	return SUCCESS;
}

int
ConfigParsing::parse_autoindex(const std::vector<std::string>& tokens, Location& location) {
	std::string usage("Usage: 'autoindex <on/off>;'");
	std::string response;

	if (tokens.size() != 2)
		return (invalid_number_arguments(1, tokens.size() - 1, tokens[0], usage));
	response = tokens[1];
	if (response != "on" && response != "off") {
		std::cerr << "Argument `" << response << "' is invalid";
		std::cerr<< " for `autoindex' instruction." << std::endl;
		std::cerr << usage << std::endl;
		return FAILURE;
	}
	location.set_autoindex(response == "on");
	return SUCCESS;
}

int
ConfigParsing::mandatory_instruction_not_found(const std::string& instruction) {
	std::cerr << "`" << instruction <<"' instruction not found in server bloc." << std::endl;
	return FAILURE;
}

int
ConfigParsing::unknown_instruction(const std::string& token, const std::string& context) {
	std::cerr << "Unknown instruction `" << token;
	std::cerr<< " in " << context << " context." << std::endl;
	return FAILURE;
}

int
ConfigParsing::semicolon_not_found(const std::string& instruction) {
	std::cerr << "Instruction `" << instruction << "' needs an ending semicolon." << std::endl;
	return FAILURE;
}

int
ConfigParsing::multiple_instructions_found(const std::string& instruction) {
	std::cerr << "Instruction `" << instruction << "' is present more than once" << std::endl;
	return FAILURE;
}

int
ConfigParsing::closing_bracket_not_alone(const std::string& line_buffer, const std::string& context) {
	std::cerr << "Closing bracket is not the single instruction on the line";
	std::cerr<< " in " << context << " context, and found with: " << line_buffer << std::endl;
	return FAILURE;
}

int
ConfigParsing::invalid_number_arguments(int expected, int found, const std::string& instruction, const std::string& usage) {
	std::cerr << "`" << instruction << "' instruction needs " << expected <<" argument(s) ";
	std::cerr << "and " << found << " were found." << std::endl;
	std::cerr << usage << std::endl;
	return FAILURE;
}

int
ConfigParsing::bracket_not_found(const std::string& line_buffer, const std::string& context, t_bracket_type type) {
	std::string type_str = type == OPENING_BRACKET ? "opening" : "closing";

	std::cerr << "Found `" << line_buffer << "' when expected " << type_str << " bracket";
	std::cerr<< " in " << context << " context." << std::endl;
	return FAILURE;
}

int
ConfigParsing::wrong_path_format(const std::string& instruction, const std::string& usage) {
	std::cerr << "Wrong path format in `" << instruction << "' instruction." << std::endl;
	std::cerr << usage << std::endl;
	return FAILURE;
}

int
ConfigParsing::check_instruction_path(const std::string &path, const std::string& instruction, const std::string& usage) {
	if (Syntax::get_path_type(path) == INVALID_PATH) {
		std::cerr << "Error in instruction `" << instruction << "'" << std::endl;
		std::cerr << "The path `" << path << "' can't be opened: ";
		std::cerr << strerror(errno) << std::endl;
		std::cerr << usage << std::endl;
		return FAILURE;
	}
	return SUCCESS;
}

int
ConfigParsing::argument_not_numerical(const std::string& token, const std::string& instruction, const std::string& usage) {
	std::cerr << "This argument: `" << token << "' in instruction`" << instruction;
	std::cerr << " must be numerical." << std::endl;
	std::cerr << usage << std::endl;
	return FAILURE;
}

int
ConfigParsing::instruction_need_at_least(int expected, const std::string& instruction, const std::string& usage) {
	std::cerr << "`" << instruction << "' instruction needs at least ";
	std::cerr << expected << " argument(s)." << std::endl;
	std::cerr << usage << std::endl;
	return FAILURE;
}

bool
ConfigParsing::is_server_instruction(const std::string& token) {
	for(size_t i = 0; i < TOTAL_SERVER_INSTRUCTIONS; i++) {
		if (Syntax::server_instructions_tab[i].name == token)
			return true;
	}
	return false;
}

bool
ConfigParsing::is_location_instruction(const std::string& token) {
	for(size_t i = 0; i < TOTAL_LOCATION_INSTRUCTIONS; i++) {
		if (Syntax::location_instructions_tab[i].name == token)
			return true;
	}
	return false;
}
