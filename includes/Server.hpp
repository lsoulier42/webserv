/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/02 15:34:25 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/02 15:34:26 by lsoulier         ###   ########.fr       */
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
# include "WebServer.hpp"
# include "Config.hpp"

# define DEFAULT_BACKLOG 5 //TODO: figure out what backlog means

class WebServer;

class Server {
	public:
		Server();
		Server(const Server& src);
		Server& operator=(const Server& rhs);
		~Server();

		void setup_default_server();
		int getServerSd() const;
		struct sockaddr* getSockAddr() const;
		socklen_t* getAddrLen() const;
		const Config* getConfig() const;
		void setConfig(Config* config);

	private:
		void _create_socket_descriptor();
		void _change_socket_options();
		void _bind_socket();
		void _set_listen_mode() const;

		Config* _config;
		int _server_sd;
		struct sockaddr_in _sock_addr;
		int _reuse_addr;
		int _addr_len;
};

#endif
