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
# include <ctime>
# include <fcntl.h>
# include <cstdlib>
# include <iostream>
# include <unistd.h>
# include <cstring>
# include <cerrno>
# include <string>
# define DEFAULT_PORT 80
# define DEFAULT_MAX_CONNECTION 5
# define DEFAULT_BUFFER_SIZE 1025
# define DEFAULT_BACKLOG 5

class Server {
	public:
		Server();
		~Server();

		void setup_server_socket();
		void bind_socket(int port);
		void listen_mode();
		void accept_connection();
		void set_non_blocking(int socket_fd);
		void build_select_list();
		void handle_data(int listnum);
		int sock_gets(int socket_fd, char *str, size_t count);
		int sock_write(int socket_fd, char *str, size_t count);
		void read_socks();
		void connect(void);

	private:
		Server& operator=(const Server& rhs);
		Server(const Server& src);

		std::allocator<int> _alloc;
		int _server_fd;
		int _addrlen;
		int _reuse_addr;
		struct sockaddr_in _sock_addr;
		int _max_connection;
		int *_connect_list;
		fd_set _socks;
		int _high_sock;
		int _exit;
};

#endif
