/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config_parsing.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 13:10:23 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/08 13:10:24 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_PARSING
# define CONFIG_PARSING
# include <iostream>
# include <string>
# include <list>
# include <vector>
# include <cstdlib>
# include <cstring>
# include <fstream>

# include "Config.hpp"
# include "Syntax.hpp"

# define TOTAL_LOCATION_INSTRUCTIONS 5

class Syntax;

enum t_bracket_type {
	OPENING,
	CLOSING
};

int 						check_config_file(const std::string &filepath, std::ifstream& config_file);
int 						check_main_bloc(std::ifstream& config_file, std::vector<Config>& configs);
int							check_server_bloc(std::ifstream& config_file, std::vector<Config>& configs);
int							check_location_bloc(std::ifstream& config_file,
								const std::vector<std::string>& tokens, Config& config);

std::string					trim_comments(const std::string &line_buffer);
bool						is_num(const char* str);
int							check_ip_format(const std::string& ip);
std::string					trim_whitespaces(const std::string& line_buffer);
std::vector<std::string>	split_whitespaces(const std::string& line_buffer);
std::vector<std::string> 	split(const std::string& line_buffer, const std::string& charset);
int 						trim_semicolon(std::vector<std::string>& tokens);

int 						parse_listen(const std::vector<std::string>&, Config&);
int 						parse_server_name(const std::vector<std::string>&, Config&);
int 						parse_error_page(const std::vector<std::string>&, Config&);
int 						parse_client_max_body_size(const std::vector<std::string>&, Config&);
int 						parse_methods(const std::vector<std::string>&, AConfig&);
int 						parse_methods_config(const std::vector<std::string>&, Config&);
int 						parse_methods_location(const std::vector<std::string>&, Location&);
int 						parse_root(const std::vector<std::string>&, AConfig&);
int 						parse_root_config(const std::vector<std::string>&, Config&);
int 						parse_root_location(const std::vector<std::string>&, Location&);
int 						parse_autoindex(const std::vector<std::string>&, AConfig&);
int 						parse_autoindex_config(const std::vector<std::string>&, Config&);
int							parse_autoindex_location(const std::vector<std::string>&, Location&);
int 						parse_index(const std::vector<std::string>&, AConfig&);
int							parse_index_config(const std::vector<std::string>&, Config&);
int							parse_index_location(const std::vector<std::string>&, Location&);
int							parse_upload_dir(const std::vector<std::string>&, Config&);
int							parse_cgi(const std::vector<std::string>&, AConfig&);
int							parse_cgi_config(const std::vector<std::string>&, Config&);
int							parse_cgi_location(const std::vector<std::string>&, Location&);

int							check_path(const std::string &path, const std::string& instruction,
										  const std::string& usage);
int 						bracket_not_found(const std::string& line_buffer, const std::string& context, int type);
int							mandatory_instruction_not_found(const std::string& instruction);
int							unknown_instruction(const std::string& token, const std::string& context);
int 						semicolon_not_found(const std::string& instruction);
int							multiple_instructions_found(const std::string& instruction);
int							closing_bracket_not_alone(const std::string& line_buffer, const std::string& context);
int							invalid_number_arguments(int expected, int found,
								const std::string& instruction, const std::string& usage);
int							wrong_path_format(const std::string& instruction, const std::string& usage);
int 						argument_not_numerical(const std::string& token,
								const std::string& instruction, const std::string& usage);
int 						instruction_need_at_least(int expected, const std::string& instruction,
								const std::string& usage);
#endif
