/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cchenot <cchenot@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 01:39:02 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/28 21:26:41 by mdereuse         ###   ########.fr       */
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
# include <sstream>
# include <csignal>

# include "Server.hpp"
# include "Client.hpp"
# include "VirtualServer.hpp"
# include "ConfigParsing.hpp"
# include "Debugger.hpp"

# define DEFAULT_MAX_CONNECTION 1024
# define MAX_BUFFER_SIZE 1000000
# define READ 0
# define WRITE 1

class Server;

class WebServer {
	public:
		WebServer();
		~WebServer();

		int parsing(const std::string& filepath);
		void setup_servers();
		void routine();
		static int sig_value;
		static void sigint_handler(int signum);

		static size_t buffer_size;

	private:
		WebServer(const WebServer& src);
		WebServer& operator=(const WebServer& rhs);

		void _accept_connection(const Server& server);
		void _build_select_list();
		void _read_socks();
		void _close_sockets();
		void _write_socks();

		std::ifstream _config_file;
		std::list<Server> _servers;
		std::list<VirtualServer> _virtual_servers;
		size_t 					_max_connection;
		std::list<Client> 		_clients;
		fd_set 					_sockets_list[2];
		int 					_highest_socket;
};
#endif
