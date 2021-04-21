/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cchenot <cchenot@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 01:39:02 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/21 19:59:20 by cchenot          ###   ########.fr       */
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

# include "Server.hpp"
# include "Client.hpp"
# include "VirtualServer.hpp"
# include "ConfigParsing.hpp"
# include "Debugger.hpp"

# define DEFAULT_MAX_CONNECTION 10
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
		static void set_non_blocking(int file_descriptor);

	private:
		WebServer(const WebServer& src);
		WebServer& operator=(const WebServer& rhs);

		void _accept_connection(const Server& server);
		void _build_select_list();
		void _read_socks();
		void _close_sockets();
		void _write_socks();
		void _process_internal_server_error(const Client& client);
		void _close_error(std::list<Client>::iterator& it, const Client::ClientError& e);

		std::ifstream _config_file;
		std::list<Server> _servers;
		std::list<VirtualServer> _virtual_servers;
		size_t _max_connection;
		std::list<Client> _clients;
		fd_set _sockets_list[2];
		int _highest_socket;
		bool _exit;

};
#endif
