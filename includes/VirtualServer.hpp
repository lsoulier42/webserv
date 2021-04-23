/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: louise <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/15 19:32:20 by louise            #+#    #+#             */
/*   Updated: 2021/04/20 11:49:54 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUALSERVER_HPP
# define VIRTUALSERVER_HPP
# include <iostream>
# include <string>
# include <list>
# include <vector>
# include "Location.hpp"
# include "Syntax.hpp"



class VirtualServer {
	public:
		VirtualServer();
		~VirtualServer();
		VirtualServer(const VirtualServer& src);
		VirtualServer& operator=(const VirtualServer& rhs);

		std::string get_ip_addr() const;
		void set_ip_addr(const std::string& ipAddr);
		const std::string &get_port_str(void) const;
		void set_port_str(const std::string &port_str);
		int get_port() const;
		void set_port(int port);
		std::list<std::string> get_server_names() const;
		void set_server_names(const std::list<std::string> &serverNames);
		std::list<status_code_t> get_error_page_codes() const;
		void set_error_page_codes(const std::list<status_code_t>& errorPageCodes);
		const std::string &get_error_page_path() const;
		void set_error_page_path(const std::string& errorPagePath);
		std::list<Location>& get_locations();
		const std::list<Location>& get_locations() const;
		void add_location(const Location& location);

		static std::list<const VirtualServer*> build_virtual_server_list(const std::list<VirtualServer>& virtual_servers,
			const VirtualServer* default_server);

	private:
		std::string _ip_addr;
		std::string _port_str;
		int _port;
		std::list<std::string> _server_names;
		std::list<status_code_t> _error_page_codes;
		std::string _error_page_path;
		std::list<Location> _locations;

};
#endif
