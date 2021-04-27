/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/02 15:34:25 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/06 20:03:53 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP
# include <string>
# include <iostream>
# include <list>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <cstring>
# include <cerrno>
# include <cstdlib>
# include <unistd.h>
# include <sstream>
# include "WebServer.hpp"
# include "VirtualServer.hpp"

# define DEFAULT_BACKLOG 32

class WebServer;

class Server {
	public:
		Server();
		Server(const Server& src);
		Server& operator=(const Server& rhs);
		~Server();

		void setup_default_server();
		int get_server_sd() const;
		struct sockaddr* get_sock_addr() const;
		socklen_t* get_addr_len() const;
		const VirtualServer* get_virtual_server() const;
		void set_virtual_server(VirtualServer* virtual_server);

	private:
		void _create_socket_descriptor();
		void _change_socket_options();
		void _bind_socket();
		void _set_listen_mode() const;

		VirtualServer *_virtual_server;
		int _server_sd;
		struct sockaddr_in _sock_addr;
		int _reuse_addr;
		int _addr_len;
};

#endif
