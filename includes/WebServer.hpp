/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 01:39:02 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/08 21:52:40 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP
# include <sys/socket.h>
# include <netinet/in.h>
# include <ctime>
# include <fcntl.h>
# include <cstdlib>
# include <iostream>
# include <unistd.h>
# include <cstring>
# include <cerrno>
# include <string>
# include <map>
# include <sstream>
# include <fstream>
# include <ios>
# include <set>

# include "Server.hpp"
# include "Client.hpp"
# include "VirtualServer.hpp"
# include "ConfigParsing.hpp"

# define DEFAULT_MAX_CONNECTION 10

class Server;

class WebServer {
	public:
		WebServer();
		~WebServer();

		int parsing(const std::string& filepath);
		void setup_servers();
		void routine();
		static void set_non_blocking(int file_descriptor);

		static bool verbose;

	private:
		WebServer(const WebServer& src);
		WebServer& operator=(const WebServer& rhs);
		void accept_connection(const Server& server);

		void build_select_list();
		void read_socks();
		void close_sockets();

		std::ifstream _config_file;
		std::list<Server> _servers;
		std::list<VirtualServer> _virtual_servers;
		size_t _max_connection;
		std::list<Client> _clients;
		fd_set _sockets_list;
		int _highest_socket;
		bool _exit;

};
#endif
