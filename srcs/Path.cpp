/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Path.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 07:15:40 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/21 07:54:16 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Path.hpp"

bool
Path::is_alpha_char(const char c) {
	return ((c >= 'a' && c <= 'z')
			|| (c >= 'A' && c <= 'Z'));
}

bool
Path::is_digit_char(const char c) {
	return (c >= '0' && c <= '9');
}

bool
Path::is_alphanum_char(const char c) {
	return (is_alpha_char(c) || is_digit_char(c));
}

bool
Path::is_mark_char(const char c) {
	return (c == '-'
			|| c == '_'
			|| c == '.'
			|| c == '!'
			|| c == '~'
			|| c == '*'
			|| c == '\''
			|| c == '('
			|| c == ')');
}

bool
Path::is_hex_char(const char c) {
	return (is_digit_char(c)
			|| (c >= 'a' && c <= 'f')
			|| (c >= 'A' && c <= 'F'));
}

bool
Path::is_unreserved_path_char(const std::string &path, size_t index) {
	return (is_alphanum_char(path[index]) || is_mark_char(path[index]));
}

bool
Path::is_extra_path_char(const std::string &path

bool
Path::is_escaped_path_char(const std::string &path, size_t index) {
	if (path.size() < index + 3 || path.compare(index, 1, "%"))
		return (false);
	if (!is_hex_char(path[index + 1]) || !is_hex_char(path[index + 2]))
		return (false);
	return (true);
}

bool
Path::is_path_segment(const std::string &path) {
	size_t index(0);
	while (index < path.size()) {
		if (is_unreserved_path_char(path, index) || is_extra_path_char(path, index))
			index++;
		else if (is_escaped_path_char(path, index))
			index += 3;
		else
			return (false);
	}
	return (true);
}

bool
Path::is_absolute_path(const std::string &path) {
	if (path.compare(0, 1, "/"))
		return (false);
	size_t		begin(1);
	size_t		end(path.find("/", begin));
	std::string	seg;
	while (end != std::string::npos) {
		seg = path.substr(begin, end - begin);
		if (!is_path_segment(seg))
			return (false);
		begin = end + 1;
		end = path.find("/", begin);
	}
	seg = path.substr(begin, end - begin);
	return (is_path_segment(seg));
}

bool
Path::_is_local_path_query(const std::string &path) {

}
