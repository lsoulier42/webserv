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

# define DEFAULT_BACKLOG 5 //TODO: figure out what backlog means

class Server {
	public:
		Server();
		Server(const std::string& ipAddr, int port);
		Server(const Server& src);
		Server& operator=(const Server& rhs);
		~Server();

		std::string getServerName() const;
		void setServerName(const std::string &serverName);
		int getId() const;
		void setId(int id);
		bool isDefault() const;
		void setDefault(bool isDefault);
		std::string getIpAddr() const;
		void setIpAddr(const std::string& ipAddr);
		int getPort() const;
		void setPort(int port);
		std::string getRoot() const;
		void setRoot(const std::string &root);
		bool isAutoindex() const;
		void setAutoindex(bool autoindex);
		std::list<std::string> getIndex() const;
		void setIndex(const std::list<std::string> &index);
		std::list<std::string> getMethods() const;
		void setMethods(const std::list<std::string> &methods);
		int getBufferBodySize() const;
		void setBufferBodySize(int bufferBodySize);
		std::string getUploadDir() const;
		void setUploadDir(const std::string &uploadDir);

		void setup_default_server();
		int getServerSd() const;
		struct sockaddr* getSockAddr() const;
		socklen_t* getAddrLen() const;

	private:
		void _create_socket_descriptor();
		void _change_socket_options();
		void _bind_socket();
		void _set_listen_mode();

		int _id;
		std::string _server_name;
		bool _is_default;
		std::string _ip_addr;
		int _port;
		std::string _root;
		bool _autoindex;
		std::list<std::string> _index;
		std::list<std::string> _methods;
		int _buffer_body_size;
		std::string _upload_dir;
		//TODO : location (need vector of future class instance Location ?
		//TODO : CGI params

		int _server_sd;
		struct sockaddr_in _sock_addr;
		int _reuse_addr;
		int _addr_len;

		static int server_id;
};

#endif
