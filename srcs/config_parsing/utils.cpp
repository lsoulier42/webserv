/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 13:09:18 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/08 13:09:19 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parsing.hpp"

std::string trim_comments(const std::string &line_buffer) {
	std::string new_line;
	size_t hash_char_pos;

	new_line = line_buffer;
	hash_char_pos = new_line.find('#');
	if (hash_char_pos == std::string::npos)
		return new_line;
	return new_line.substr(0, hash_char_pos);
}

std::string trim_whitespaces(const std::string& line_buffer) {
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

std::vector<std::string> split_whitespaces(const std::string& line_buffer) {
	return split(line_buffer, " \n\r\t\f\v");
}

std::vector<std::string> split(const std::string& line_buffer, const std::string& charset) {
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

int trim_semicolon(std::vector<std::string>& tokens) {
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

bool is_num(const char* str) {
	for(int i = 0; str[i]; i++) {
		if (!std::isdigit(str[i]))
			return false;
	}
	return true;
}

int check_ip_format(const std::string& ip) {
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
