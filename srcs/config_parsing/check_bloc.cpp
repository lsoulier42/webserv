/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_check_bloc.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 13:07:40 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/08 13:07:41 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parsing.hpp"

int check_config_file(const std::string &filepath, std::ifstream& config_file) {
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
	config_file.open(filepath.c_str(), std::ios_base::in);
	if (!config_file) {
		std::cerr << "The file `" << filepath << "' can't be opened: ";
		std::cerr << strerror(errno) << std::endl;
		return 0;
	}
	return (1);
}

int check_main_bloc(std::ifstream& config_file, std::vector<Config>& configs) {
	std::string line_buffer;
	std::vector<std::string> tokens;
	std::string usage("Usage: `server {'");

	while(config_file) {
		std::getline(config_file, line_buffer);
		line_buffer = trim_comments(line_buffer);
		line_buffer = trim_whitespaces(line_buffer);
		tokens = split_whitespaces(line_buffer);
		if (line_buffer.empty() || tokens.empty())
			continue;
		if (tokens[0] != "server")
			return unknown_instruction(tokens[0], "main");
		if (tokens.size() == 1 || (tokens.size() == 2 && tokens[1] != "{"))
			return(bracket_not_found(tokens[1], "main", OPENING));
		if (tokens.size() > 2)
			return (invalid_number_arguments(1, tokens.size() - 1, "server", usage));
		if (!check_server_bloc(config_file, configs))
			return 0;
	}
	if (configs.empty()) {
		std::cerr << "Config file is empty." << std::endl;
		return 0;
	}
	return 1;
}

int check_server_bloc(std::ifstream& config_file, std::vector<Config>& configs) {
	std::string line_buffer;
	std::vector<std::string> tokens;
	Config new_config;
	bool closing_bracket = false;
	bool instructions_complete[TOTAL_SERVER_INSTRUCTIONS] = {false};
	std::string instructions[TOTAL_SERVER_INSTRUCTIONS] = {"listen", "server_name",
	   "error_page", "client_max_body_size", "location", "methods",
	   "root", "autoindex", "index", "upload_dir", "cgi"};
	int (*instructions_functions[TOTAL_SERVER_INSTRUCTIONS])(const std::vector<std::string>&,
		Config&) = { &parse_listen, &parse_server_name, &parse_error_page,
		&parse_client_max_body_size, NULL, &parse_methods_config,
		&parse_root_config, &parse_autoindex_config, &parse_index_config,
		&parse_upload_dir, &parse_cgi_config };

	while(config_file) {
		std::getline(config_file, line_buffer);
		line_buffer = trim_comments(line_buffer);
		line_buffer = trim_whitespaces(line_buffer);
		tokens = split_whitespaces(line_buffer);
		if (line_buffer.empty() || tokens.empty())
			continue;
		if (tokens[0] == "}") {
			if (tokens.size() > 1)
				return(closing_bracket_not_alone(line_buffer, "server"));
			closing_bracket = true;
			break;
		}
		for(int i = 0; i < TOTAL_SERVER_INSTRUCTIONS; i++) {
			if (instructions[i] == tokens[0]) {
				if (instructions_complete[i] && i != LOCATION)
					return (multiple_instructions_found(instructions[i]));
				if (i != LOCATION && !trim_semicolon(tokens))
					return semicolon_not_found(instructions[i]);
				if ((i != LOCATION && !(*instructions_functions[i])(tokens, new_config))
					|| (i == LOCATION && !check_location_bloc(config_file, tokens, new_config)))
					return 0;
				instructions_complete[i] = true;
				break;
			}
			if (i == TOTAL_SERVER_INSTRUCTIONS - 1)
				return(unknown_instruction(tokens[0], "server"));
		}
	}
	if (!closing_bracket)
		return(bracket_not_found(line_buffer, "server", CLOSING));
	if (!instructions_complete[LISTEN])
		return (mandatory_instruction_not_found("listen"));
	if (!instructions_complete[ROOT])
		return (mandatory_instruction_not_found("root"));
	configs.push_back(new_config);
	return (1);
}

int check_location_bloc(std::ifstream& config_file, const std::vector<std::string>& tokens, Config& config) {
	std::string usage("Usage: 'location [/random/path/name] {'");
	std::string path, bracket, line_buffer;
	std::vector<std::string> location_tokens;
	Location new_location;
	bool closing_bracket = false;
	bool instructions_complete[TOTAL_LOCATION_INSTRUCTIONS] = {false};
	std::string instructions[TOTAL_LOCATION_INSTRUCTIONS] = {"methods",
		"root", "autoindex", "index", "cgi"};
	int (*instructions_functions[TOTAL_LOCATION_INSTRUCTIONS])(const std::vector<std::string>&,
		Location&) = { &parse_methods_location, &parse_root_location,
		&parse_autoindex_location, &parse_index_location, &parse_cgi_location};

	if(tokens.size() != 3)
		return (invalid_number_arguments(2, tokens.size() - 1, tokens[0], usage));
	path = tokens[1];
	bracket = tokens[2];
	if (path[0] != '/')
		return(wrong_path_format(tokens[0], usage));
	if (bracket != "{")
		return(bracket_not_found(bracket, tokens[0], OPENING));
	new_location.setPath(path);
	while(config_file) {
		std::getline(config_file, line_buffer);
		line_buffer = trim_comments(line_buffer);
		line_buffer = trim_whitespaces(line_buffer);
		location_tokens = split_whitespaces(line_buffer);
		if (line_buffer.empty() || location_tokens.empty())
			continue;
		if (location_tokens[0] == "}") {
			if (location_tokens.size() > 1)
				return(closing_bracket_not_alone(line_buffer, tokens[0]));
			closing_bracket = true;
			break;
		}
		for(int i = 0; i < TOTAL_LOCATION_INSTRUCTIONS; i++) {
			if (instructions[i] == location_tokens[0]) {
				if (instructions_complete[i])
					return (multiple_instructions_found(instructions[i]));
				if (!trim_semicolon(location_tokens))
					return semicolon_not_found(instructions[i]);
				if (!(*instructions_functions[i])(location_tokens, new_location))
					return 0;
				instructions_complete[i] = true;
				break;
			}
			if (i == TOTAL_LOCATION_INSTRUCTIONS - 1)
				return(unknown_instruction(location_tokens[0], tokens[0]));
		}
	}
	if (!closing_bracket)
		return(bracket_not_found(line_buffer, "server", CLOSING));
	config.addLocation(new_location);
	return 1;
}
