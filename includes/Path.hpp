/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Path.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 09:14:11 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/21 09:41:16 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PATH_HPP
# define PATH_HPP

# include <string>

class Path {
	public:

		static bool is_alpha_char(const char c);
		static bool is_digit_char(const char c);
		static bool is_alphanum_char(const char c);
		static bool is_hex_char(const char c);
		static bool is_unreserved_path_char(const char c);
		static bool is_reserved_path_char(const char c);
		static bool is_extra_path_char(const char c);
		static bool is_escaped_path_char(const std::string &path, size_t index);
		static bool is_path_segment(const std::string &path);
		static bool is_absolute_path(const std::string &path);
		static bool is_query_string(const std::string &path);
		static bool is_local_path_query(const std::string &path);

	private:

		Path(void);

};

#endif
