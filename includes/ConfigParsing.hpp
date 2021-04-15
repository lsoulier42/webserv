/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParsing.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: louise <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/15 20:59:31 by louise            #+#    #+#             */
/*   Updated: 2021/04/15 20:59:32 by louise           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSING_HPP
# define CONFIGPARSING_HPP

# include <iostream>
# include <string>
# include <list>
# include <vector>
# include <set>
# include <cstdlib>
# include <cstring>
# include <fstream>
# include <cerrno>

# include "VirtualServer.hpp"
# include "Syntax.hpp"

# define INSTRUCTION_TOKEN 0

enum t_bracket_type {
	OPENING_BRACKET,
	CLOSING_BRACKET
};

class ConfigParsing {
	public:
		~ConfigParsing();

	static int check_config_file(const std::string &filepath, std::ifstream& config_file);
	static int check_main_bloc(std::ifstream& config_file, std::list<VirtualServer>& virtual_servers);
	static int check_server_bloc(std::ifstream& config_file, std::list<VirtualServer>& virtual_servers);
	static int check_location_bloc(std::ifstream& config_file,
		const std::vector<std::string>& tokens, VirtualServer& virtual_server);
	static std::vector<std::string> format_instruction_line(std::ifstream& config_file);

	static int parse_listen(const std::vector<std::string>& tokens, VirtualServer& virtual_server);
	static int parse_server_name(const std::vector<std::string>& tokens, VirtualServer& virtual_server);
	static int parse_error_page(const std::vector<std::string>& tokens, VirtualServer& virtual_server);
	static int parse_client_max_body_size(const std::vector<std::string>& tokens, VirtualServer& virtual_server);
	static int parse_upload_dir(const std::vector<std::string>& tokens, VirtualServer& virtual_server);

	static int parse_root(const std::vector<std::string>& tokens, Location& location);
	static int parse_methods(const std::vector<std::string>& tokens, Location& location);
	static int parse_index(const std::vector<std::string>& tokens, Location& location);
	static int parse_cgi(const std::vector<std::string>& tokens, Location& location);
	static int parse_autoindex(const std::vector<std::string>& tokens, Location& location);

	static int bracket_not_found(const std::string& line_buffer, const std::string& context, t_bracket_type type);
	static int mandatory_instruction_not_found(const std::string& instruction);
	static int unknown_instruction(const std::string& token, const std::string& context);
	static int semicolon_not_found(const std::string& instruction);
	static int multiple_instructions_found(const std::string& instruction);
	static int closing_bracket_not_alone(const std::string& line_buffer, const std::string& context);
	static int invalid_number_arguments(int expected, int found,
		const std::string& instruction, const std::string& usage);
	static int wrong_path_format(const std::string& instruction, const std::string& usage);
	static int argument_not_numerical(const std::string& token,
		const std::string& instruction, const std::string& usage);
	static int instruction_need_at_least(int expected, const std::string& instruction,
		const std::string& usage);
	static int check_instruction_path(const std::string &path, const std::string& instruction,
		const std::string& usage);
	static bool is_server_instruction(const std::string& token);
	static bool is_location_instruction(const std::string& token);
	static int server_instruction_handler(std::vector<std::string>& tokens,
		VirtualServer& virtual_server, std::set<server_instruction_t>& instructions_filled);
	static int location_instructions_handler(std::vector<std::string>& tokens,
		Location& location, std::set<location_instruction_t>& instructions_filled);

	private:
		ConfigParsing();
		ConfigParsing(const ConfigParsing& src);
		ConfigParsing& operator=(const ConfigParsing& rhs);

};
#endif
