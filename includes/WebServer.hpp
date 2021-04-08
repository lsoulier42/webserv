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
# include <vector>
# include <map>
# include <sstream>

# include "Server.hpp"
# include "Client.hpp"

# define DEFAULT_MAX_CONNECTION 5
# define DEFAULT_BUFFER_SIZE 1025

class WebServer {
	public:
		WebServer(const std::vector<Server>& servers);
		~WebServer();

		void routine();
		void setup_servers();

	private:
		WebServer();
		WebServer(const WebServer& src);
		WebServer& operator=(const WebServer& rhs);

		void accept_connection(const Server& server);
		void set_non_blocking(int socket_fd);
		void build_select_list();
		void read_socks();
		void close_sockets();

		std::vector<Server> _servers;
		int _max_connection;
		std::vector<Client> _clients;
		std::map<int, int> _config_assoc;
		fd_set _sockets_list;
		int _highest_socket;
		int _exit;
};
#endif
