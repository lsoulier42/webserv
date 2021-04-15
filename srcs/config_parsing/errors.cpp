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

int check_instruction_path(const std::string &path, const std::string& instruction, const std::string& usage) {
	if (!Syntax::is_valid_path(path)) {
		std::cerr << "Error in instruction `" << instruction << "'" << std::endl;
		std::cerr << "The path `" << path << "' can't be opened: ";
		std::cerr << strerror(errno) << std::endl;
		std::cerr << usage << std::endl;
		return 0;
	}
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
