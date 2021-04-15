/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/07 16:55:59 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/07 16:56:00 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP
# include <iostream>
# include <string>
# include <vector>
# include <list>

class Location {
	public:
		Location();
		Location(const Location& src);
		Location& operator=(const Location& rhs);
		~Location();

		std::string get_path() const;
		void set_path(const std::string& path);
		const std::string &get_root() const;
		void set_root(const std::string &root);
		std::list<std::string> get_methods() const;
		void set_methods(const std::list<std::string> &methods);
		std::list<std::string> get_index() const;
		void set_index(const std::list<std::string> &index);
		std::string get_cgi_extension() const;
		void set_cgi_extension(const std::string &cgiExtension);
		std::string get_cgi_path() const;
		void set_cgi_path(const std::string &cgiPath);
		bool is_autoindex() const;
		void set_autoindex(bool autoindex);

	private:
		std::string _path;
		std::string _root;
		std::list<std::string> _methods;
		std::list<std::string> _index;
		std::string _cgi_extension;
		std::string _cgi_path;
		bool _autoindex;
};
#endif
