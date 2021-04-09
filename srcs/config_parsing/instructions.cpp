/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   instructions.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 13:09:12 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/08 13:09:13 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parsing.hpp"

int parse_listen(const std::vector<std::string>& tokens, Config& config) {
	std::string ip_addr;
	std::string port;
	int port_nb;
	size_t colon_pos;
	std::string usage("Usage: 'listen <ip:port>;'");

	usage += "ip format : [0-255].[0-255].[0-255].[0-255] - port number: [1-65535]";
	if (tokens.size() != 2)
		return (invalid_number_arguments(2, tokens.size() - 1, tokens[0], usage));
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
	if (!is_num(port.c_str()))
		return (argument_not_numerical(port, tokens[0], usage));
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

int parse_server_name(const std::vector<std::string>& tokens, Config& config) {
	std::list<std::string> server_names;
	std::string usage("Usage: 'server_name <string1> <string2> [...];'");

	if (tokens.size() < 2)
		return (instruction_need_at_least(1, tokens[0], usage));
	for(std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		server_names.push_back(*it);
	}
	config.setServerName(server_names);
	return 1;
}

int parse_error_page(const std::vector<std::string>& tokens, Config& config) {
	std::list<int> error_codes;
	std::string error_path;
	int error_code;
	std::string usage("Usage: 'error_page <400 401 504....> [/absolute/path];'");

	if (tokens.size() < 3)
		return (instruction_need_at_least(2, tokens[0], usage));
	for (std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		if (*it != tokens.back()) {
			if (!is_num(it->c_str()))
				return (argument_not_numerical(*it, tokens[0], usage));
			error_code = std::strtol(it->c_str(), NULL, 10);
			if (!Syntax::is_error_code(error_code)) {
				std::cerr << "Argument `" << *it << "' is not a valid error code." <<std::endl;
				std::cerr << usage << std::endl;
				return 0;
			}
			error_codes.push_back(error_code);
		}
		else {
			if (!check_path(*it, tokens[0], usage))
				return 0;
			error_path = *it;
		}
	}
	config.setErrorPageCodes(error_codes);
	config.setErrorPagePath(error_path);
	return 1;
}

int parse_client_max_body_size(const std::vector<std::string>& tokens, Config& config) {
	int client_max_body_size;
	std::string usage("Usage: 'client_max_body_size <positive, non null, numerical value>;'");

	if (tokens.size() != 2)
		return (invalid_number_arguments(1, tokens.size() - 1, tokens[0], usage));
	if (!is_num(tokens.back().c_str()))
		return (argument_not_numerical(tokens.back(), tokens[0], usage));
	client_max_body_size = std::strtol(tokens.back().c_str(), NULL, 10);
	if (client_max_body_size == 0) {
		std::cerr << "`client_max_body_size' argument can't be null." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	config.setClientMaxBodySize(client_max_body_size);
	return 1;
}

int parse_methods(const std::vector<std::string>& tokens, AConfig& config) {
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

int parse_methods_config(const std::vector<std::string>& tokens, Config& config) {
	return parse_methods(tokens, config);
}

int parse_methods_location(const std::vector<std::string>& tokens, Location& location) {
	return parse_methods(tokens, location);
}

int parse_root(const std::vector<std::string>& tokens, AConfig& config) {
	std::string usage("Usage: 'root [/absolute/path];'");
	std::string path;

	if (tokens.size() != 2)
		return (invalid_number_arguments(1, tokens.size() - 1, tokens[0], usage));
	path = tokens[1];
	if (path[0] != '/')
		return wrong_path_format(tokens[0], usage);
	if (!check_path(path, tokens[0], usage))
		return 0;
	config.setRoot(path);
	return 1;
}

int parse_root_config(const std::vector<std::string>& tokens, Config& config) {
	return parse_root(tokens, config);
}

int parse_root_location(const std::vector<std::string>& tokens, Location& location) {
	return parse_root(tokens, location);
}

int parse_autoindex(const std::vector<std::string>& tokens, AConfig& config) {
	std::string usage("Usage: 'autoindex <on/off>;'");
	std::string response;

	if (tokens.size() != 2)
		return (invalid_number_arguments(1, tokens.size() - 1, tokens[0], usage));
	response = tokens[1];
	if (response != "on" && response != "off") {
		std::cerr << "Argument `" << response << "' is invalid";
		std::cerr<< " for `autoindex' instruction." << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	config.setAutoindex(response == "on");
	return 1;
}

int parse_autoindex_config(const std::vector<std::string>& tokens, Config& config) {
	return parse_autoindex(tokens, config);
}

int parse_autoindex_location(const std::vector<std::string>& tokens, Location& location) {
	return parse_autoindex(tokens, location);
}

int parse_index(const std::vector<std::string>& tokens, AConfig& config) {
	std::string usage("Usage: 'index <page1>.html <page2>.php [...];'");
	std::list<std::string> index_pages;

	if (tokens.size() < 2)
		return (instruction_need_at_least(1, tokens[0], usage));
	for (std::vector<std::string>::const_iterator it = ++tokens.begin(); it != tokens.end(); it++) {
		index_pages.push_back(*it);
	}
	config.setIndex(index_pages);
	return 1;
}

int parse_index_config(const std::vector<std::string>& tokens, Config& config) {
	return parse_index(tokens, config);
}

int parse_index_location(const std::vector<std::string>& tokens, Location& location) {
	return parse_index(tokens, location);
}

int parse_upload_dir(const std::vector<std::string>& tokens, Config& config) {
	std::string usage("Usage: 'upload_dir [/absolute/path];'");
	std::string path;

	if (tokens.size() != 2)
		return (invalid_number_arguments(1, tokens.size() - 1, tokens[0], usage));
	path = tokens[1];
	if (path[0] != '/')
		return wrong_path_format(tokens[0], usage);
	if (!check_path(path, tokens[0], usage))
		return 0;
	config.setUploadDir(path);
	return 1;
}

int parse_cgi(const std::vector<std::string>& tokens, AConfig& config) {
	std::string usage("Usage: 'cgi [*.<ext>] [/absolute/path];'");
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
		return 0;
	}
	if (cgi_path[0] != '/')
		return wrong_path_format(tokens[0], usage);
	if (!check_path(cgi_path, tokens[0], usage))
		return 0;
	config.setCgiExtension(cgi_file_ext.substr(2));
	config.setCgiPath(cgi_path);
	return 1;
}

int parse_cgi_config(const std::vector<std::string>& tokens, Config& config) {
	return parse_cgi(tokens, config);
}

int parse_cgi_location(const std::vector<std::string>& tokens, Location& location) {
	return parse_cgi(tokens, location);
}
