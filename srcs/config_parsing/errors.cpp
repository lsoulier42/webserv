/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   errors.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 13:09:25 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/08 13:09:26 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parsing.hpp"

const method_tab_entry_t	method_tab[] =
		{
				{GET, "GET", 3},
				{HEAD, "HEAD", 4},
				{POST, "POST", 4},
				{PUT, "PUT", 3},
				{DELETE, "DELETE", 6},
				{CONNECT, "CONNECT", 7},
				{OPTIONS, "OPTIONS", 7},
				{TRACE, "TRACE", 5},
				{DEFAULT, "", 0}
		};

int method_index(const std::string& method) {
	for(int i = 0; i < DEFAULT; i++) {
		if (method == method_tab[i].str)
			return i;
	}
	return -1;
}

bool is_informational_code(int code) {
	return code == 100 || code == 101;
}

bool is_successful_code(int code) {
	return code >= 200 && code <= 206;
}

bool is_redirection_code(int code) {
	return (code >= 300 && code <= 305) || code == 307;
}

bool is_client_error_code(int code) {
	return (code >= 400 && code <= 417) || code == 426;
}

bool is_server_error_code(int code) {
	return code >= 500 && code <= 505;
}

bool is_error_code(int code) {
	return is_server_error_code(code) || is_client_error_code(code);
}

int mandatory_instruction_not_found(const std::string& instruction) {
	std::cerr << "`" << instruction <<"' instruction not found in server bloc." << std::endl;
	return 0;
}

int unknown_instruction(const std::string& token, const std::string& context) {
	std::cerr << "Unknown instruction `" << token;
	std::cerr<< " in " << context << " context." << std::endl;
	return 0;
}

int semicolon_not_found(const std::string& instruction) {
	std::cerr << "Instruction `" << instruction << "' needs an ending semicolon." << std::endl;
	return 0;
}

int multiple_instructions_found(const std::string& instruction) {
	std::cerr << "Instruction `" << instruction << "' is present more than once" << std::endl;
	return 0;
}

int closing_bracket_not_alone(const std::string& line_buffer, const std::string& context) {
	std::cerr << "Closing bracket is not the single instruction on the line";
	std::cerr<< " in " << context << " context, and found with: " << line_buffer << std::endl;
	return 0;
}

int invalid_number_arguments(int expected, int found, const std::string& instruction, const std::string& usage) {
	std::cerr << "`" << instruction << "' instruction needs " << expected <<" argument(s) ";
	std::cerr << "and " << found << " were found." << std::endl;
	std::cerr << usage << std::endl;
	return 0;
}

int bracket_not_found(const std::string& line_buffer, const std::string& context, int type) {
	std::string type_str = type == OPENING ? "opening" : "closing";

	std::cerr << "Found `" << line_buffer << "' when expected " << type_str << " bracket";
	std::cerr<< " in " << context << " context." << std::endl;
	return 0;
}

int wrong_path_format(const std::string& instruction, const std::string& usage) {
	std::cerr << "Wrong path format in `" << instruction << "' instruction." << std::endl;
	std::cerr << usage << std::endl;
	return 0;
}

int check_path(const std::string &path, const std::string& instruction, const std::string& usage) {
	std::ifstream test(path.c_str());
	if (!test) {
		std::cerr << "Error in instruction `" << instruction << "'" << std::endl;
		std::cerr << "The path `" << path << "' can't be opened: ";
		std::cerr << strerror(errno) << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
	test.close();
	return 1;
}

int argument_not_numerical(const std::string& token, const std::string& instruction, const std::string& usage) {
	std::cerr << "This argument: `" << token << "' in instruction`" << instruction;
	std::cerr << " must be numerical." << std::endl;
	std::cerr << usage << std::endl;
	return 0;
}

int instruction_need_at_least(int expected, const std::string& instruction, const std::string& usage) {
	std::cerr << "`" << instruction << "' instruction needs at least ";
	std::cerr << expected << " argument(s)." << std::endl;
	std::cerr << usage << std::endl;
	return 0;
}
