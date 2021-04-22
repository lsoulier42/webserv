/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Path.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 09:14:11 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/22 01:42:39 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PATH_HPP
# define PATH_HPP

# include <string>
# include <vector>
# include "Syntax.hpp"

class Path {

	public:

		static bool is_alpha_char(const char c);
		static bool is_digit_char(const char c);
		static bool is_alphanum_char(const char c);
		static bool is_hex_char(const char c);
		static bool is_mark_char(const char c);
		static bool is_unreserved_path_char(const char c);
		static bool is_reserved_path_char(const char c);
		static bool is_extra_path_char(const char c);
		static bool is_escaped_path_char(const std::string &path, size_t index);
		static bool is_uric_path_char(const std::string &path, size_t index);
		static bool is_uric_no_slash_path_char(const std::string &path, size_t index);

		static bool is_path_segment(const std::string &path);
		static bool is_absolute_path(const std::string &path);

		static bool is_query_string(const std::string &path);
		static bool is_fragment(const std::string &path);
		static bool is_scheme(const std::string &path);
		static bool is_user_info(const std::string &path);
		static bool is_hostname(const std::string &path);
		static bool is_ipv4_address(const std::string &path);
		static bool is_port(const std::string &path);
		static bool is_host(const std::string &path);
		static bool is_hostport(const std::string &path);
		static bool is_server(const std::string &path);
		static bool is_reg_name(const std::string &path);
		static bool is_authority_component(const std::string &path);
		static bool is_net_path(const std::string &path);
		static bool is_hier_part(const std::string &path);
		static bool is_opaque_part(const std::string &path);
		static bool is_absolute_uri(const std::string &path);
		static bool is_fragment_uri(const std::string &path);
		static bool is_local_path_query(const std::string &path);

	private:

		Path(void);

};

#endif
