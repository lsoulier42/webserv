/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Path.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 07:15:40 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/22 02:44:10 by mdereuse         ###   ########.fr       */
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
Path::is_unreserved_char(const char c) {
	return (is_alphanum_char(c) || is_mark_char(c));
}

bool
Path::is_hex_char(const char c) {
	return (is_digit_char(c)
			|| (c >= 'a' && c <= 'f')
			|| (c >= 'A' && c <= 'F'));
}

/*
bool
Path::patate_reserved(const char c) {
	return (c == ';'
			|| c == '/'
			|| c == '?'
			|| c == ':'
			|| c == '@'
			|| c == '&'
			|| c == '='
			|| c == '+'
			|| c == '$'
			|| c == ','
			|| c == '['
			|| c == ']');
}
*/

bool
Path::is_escaped_char(const std::string &path, size_t index) {
	if (path.size() < index + 3 || path.compare(index, 1, "%"))
		return (false);
	if (!is_hex_char(path[index + 1]) || !is_hex_char(path[index + 2]))
		return (false);
	return (true);
}

bool
Path::is_reserved_path_char(const char c) {
	return (c == ':'
			|| c == '@'
			|| c == '&'
			|| c == '='
			|| c == '+'
			|| c == '$'
			|| c == ',');
}

bool
Path::is_reserved_query_char(const char c) {
	return (c == ';'
			|| c == '/'
			|| c == '?'
			|| c == ':'
			|| c == '@'
			|| c == '&'
			|| c == '='
			|| c == '+'
			|| c == ','
			|| c == '$');
}

bool
Path::is_reserved_user_info_char(const char c) {
	return (c == ';'
			|| c == ':'
			|| c == '&'
			|| c == '='
			|| c == '+'
			|| c == '$'
			|| c == ',');
}

bool
Path::is_reserved_reg_name_char(const char c) {
	return (c == '$'
			|| c == ','
			|| c == ';'
			|| c == ':'
			|| c == '@'
			|| c == '&'
			|| c == '='
			|| c == '+');
}

bool
Path::is_uric_no_slash_char(const std::string &path, size_t index) {
	return (is_unreserved_char(path[index])
			|| is_escaped_char(path, index)
			|| path[index] == ';'
			|| path[index] == '?'
			|| path[index] == ':'
			|| path[index] == '@'
			|| path[index] == '&'
			|| path[index] == '='
			|| path[index] == '+'
			|| path[index] == '$'
			|| path[index] == ',');
}

//TODO:: add optional param starting with ;
bool
Path::is_path_segment(const std::string &path) {
	size_t index(0);
	while (index < path.size()) {
		if (is_unreserved_path_char(path[index]) || is_reserved_path_char(path[index]))
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
	if (path.empty() || path.compare(0, 1, "/"))
		return (false);
	size_t		start_seg(1);
	size_t		end_seg(path.find("/", start_seg));
	std::string	seg;
	while (end_seg != std::string::npos) {
		seg = path.substr(start_seg, end_seg - start_seg);
		if (!is_path_segment(seg))
			return (false);
		start_seg = end_seg + 1;
		end_seg = path.find("/", start_seg);
	}
	seg = path.substr(start_seg);
	return (is_path_segment(seg));
}

bool
Path::is_query_string(const std::string &path) {
	size_t index(0);
	while (index < path.size()) {
		if (is_unreserved_char(path[index])
				|| is_reserved_query_char(path[index]))
			index++;
		else if (is_escaped_char(path, index))
			index += 3;
		else
			return (false);
	}
	return (true);
}

//media type dependent
bool
Path::is_fragment(const std::string &path) {
	return (true);
}

bool
Path::is_scheme(const std::string &path) {
	if (path.empty() || !is_alpha_char(path[0]))
		return (false);
	size_t	index(0);
	while (index < path.size()) {
		if (!is_alphanum_char(path[index])
				&& !(path[index] == '+'
					|| path[index] == '-'
					|| path[index] == '.'))
			return (false);
		index++;
	}
	return (true);
}

bool
Path::is_user_info(const std::string &path) {
	size_t	index(0);
	while (index < path.size()) {
		if (is_unreserved_char(path[index]) 
				|| is_reserved_user_info_char(path[index]))
			index++;
		else if (is_escaped_path_char(path, index))
			index += 3;
		else
			return (false);
	}
	return (true);
}

//TODO:: ameliorer
bool
Path::is_hostname(const std::string &path) {
	size_t	index(0);
	while (index < path.size())
		if (!is_alphanum_char(path[index])
				&& !(path[index] == '.'
					|| path[index] == '-'))
			return (false);
	return (true);
}

bool
Path::is_ipv4_address(const std::string &path) {
	std::vector<std::string>	segs(Syntax::split(path, "."));
	if (segs.size() != 4)
		return (false);
	int	byte;
	for (std::vector<std::string>::iterator it(segs.begin()) ; it != segs.end() ; it++) {
		if (!Syntax::str_is_num(*it))
			return (false);
		byte = std::strtol((*it).c_str(), NULL, 10);
		if (byte < 0 || byte > 255)
			return (false);
	}
	return (true);
}

bool
Path::is_port(const std::string &path) {
	return (Syntax::str_is_num(path));
}

bool
Path::is_host(const std::string &path) {
	return (is_hostname(path) || is_ipv4_address(path));
}

bool
Path::is_hostport(const std::string &path) {
	size_t	start_port(path.find(":"));
	std::string	seg(path.substr(0, start_port));
	if (!is_host(seg))
		return (false);
	if (std::string::npos != start_port) {
		seg = path.substr(start_port + 1);
		return (is_port(seg));
	}
	return (true);
}

bool
Path::is_server(const std::string &path) {
	std::string seg;
	size_t	end_user_info(path.find("@"));
	if (std::string::npos != end_user_info) {
		seg = path.substr(0, end_user_info);
		if (!is_user_info(seg))
			return (false);
		seg = path.substr(end_user_info + 1);
	} else
		seg = path;
	if (!seg.empty())
		return (is_hostport(seg));
	return (true);
}

bool
Path::is_reg_name(const std::string &path) {
	size_t	index(0);
	if (path.empty())
		return (false);
	while (index < path.size()) {
		if (is_unreserved_char(path[index])
				|| is_reserved_reg_name_char(path[index]))
			index++;
		else if (is_escaped_path_char(path, index))
			index += 3;
		else
			return (false);
	}
	return (true);
}

bool
Path::is_authority_component(const std::string &path) {
	return (is_server(path) || is_reg_name(path));
}

bool
Path::is_net_path(const std::string &path) {
	if (path.size() < 3 || path.compare(0, 2, "//"))
		return (false);
	std::string seg;
	size_t		end_authority(path.find("/", 2));
	if (std::string::npos != end_authority) {
		seg = path.substr(2, end_authority - 2);
		if (!is_authority_component(seg))
			return (false);
		seg = path.substr(end_authority + 1);
		return (is_absolute_path(seg));
	}
	seg = path.substr(2);
	return (is_authority_component(seg));
}

bool
Path::is_hier_part(const std::string &path) {
	size_t	start_query(path.find("?"));
	std::string seg(path.substr(0, start_query));
	if (!is_net_path(seg) && !is_absolute_path(seg))
		return (false);
	if (std::string::npos != start_query) {
		seg = path.substr(start_query + 1);
		return (is_query_string(seg));
	}
	return (true);
}

bool
Path::is_opaque_part(const std::string &path) {
	if (path.empty() || !is_uri_no_slash_char(path, 0))
		return (false);
	return (true);
}

bool
Path::is_absolute_uri(const std::string &path) {
	size_t	end_scheme(path.find(":"));
	if (std::string::npos == end_scheme)
		return (false);
	std::string seg(path.substr(0, end_scheme));
	if (!is_scheme(seg) || path.size() == end_scheme + 1)
		return (false);
	seg = path.substr(end_scheme + 1);
	return (is_hier_part(seg) || is_opaque_part(seg));
}

bool
Path::is_fragment_uri(const std::string &path) {
	size_t	start_fragment(path.find("#"));
	std::string	seg(path.substr(0, start_fragment));
	if (!is_absolute_uri(seg))
		return (false);
	if (std::string::npos != start_fragment) {
		seg = path.substr(start_fragment + 1);
		return (is_fragment(seg));
	}
	return (true);
}

bool
Path::is_local_path_query(const std::string &path) {
	size_t	start_query(path.find("?"));
	std::string	seg(path.substr(0, start_query));
	if (!is_absolute_path(seg))
		return (false);
	if (std::string::npos != start_query) {
		seg = path.substr(start_query + 1);
		return (is_query_string(seg));
	}
	return (true);
}
