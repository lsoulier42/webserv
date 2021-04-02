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
# include <sys/socket.h>
# include <netinet/in.h>
# include <cstdlib>
# include <iostream>
# include <unistd.h>
# include <cstring>
# include <cerrno>
# include <string>
# define BINDING_PORT 80

class Server {
	public:
		Server();
		Server(const Server& src);
		Server& operator=(const Server& rhs);
		~Server();

		void create_socket();
		void bind_socket(int port);
		void listen_mode();
		void accept_connection();
		int server_routine();
		void connect(void);

	private:
		int _socket_fd;
		int _addrlen;
		struct sockaddr_in _sock_addr;
		int _new_socket;
};

#endif
