/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 01:39:02 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/06 01:39:03 by lsoulier         ###   ########.fr       */
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
# include <fstream>
# include <ios>

# include "Server.hpp"
# include "Config.hpp"

# define DEFAULT_MAX_CONNECTION 5
# define DEFAULT_BUFFER_SIZE 1025

class Server;

class WebServer {
	public:
		WebServer();
		~WebServer();

		int parsing(const std::string& filepath);
		void setup_servers();
		void routine();

		static bool verbose;

	private:
		WebServer(const WebServer& src);
		WebServer& operator=(const WebServer& rhs);

		/* Network functions
		 *
		 *
		 *
		 */
		void accept_connection(const Server& server);
		static void set_non_blocking(int socket_fd);
		void build_select_list();
		void handle_data(int socket_id);
		int sock_gets(int socket_fd, char *str, size_t count);
		void read_socks();
		void close_sockets();

		/* Parsing functions for config file
		 *
		 *
		 *
		 */
		int check_config_file(const std::string& filepath);
		int check_main_bloc();
		int check_server_bloc();
		//int check_location_bloc();
		static bool is_num(const char* str);
		static std::string trim_comments(const std::string& line_buffer);
		static std::string trim_whitespaces(const std::string& line_buffer);
		static int trim_semicolon(std::vector<std::string>& tokens);
		static std::vector<std::string> WebServer::split(const std::string& line_buffer, const std::string& charset);
		static std::vector<std::string> split_whitespaces(const std::string& line_buffer);
		enum t_instructions {
			LISTEN,
			SERVER_NAME,
			ERROR_PAGE,
			CLIENT_MAX_BODY_SIZE,
			LOCATION,
			METHODS,
			ROOT,
			AUTOINDEX,
			INDEX,
			UPLOAD_DIR,
			CGI,
			TOTAL_INSTRUCTIONS
		};
		int parse_listen(const std::vector<std::string>&, Config&);
		int parse_server_name(const std::vector<std::string>&, Config&);
		int parse_error_page(const std::vector<std::string>&, Config&);
		int parse_client_max_body_size(const std::vector<std::string>&, Config&);
		int parse_location(const std::vector<std::string>&, Config&);
		int parse_methods(const std::vector<std::string>&, Config&);
		int parse_root(const std::vector<std::string>&, Config&);
		int parse_autoindex(const std::vector<std::string>&, Config&);
		int parse_index(const std::vector<std::string>&, Config&);
		int parse_upload_dir(const std::vector<std::string>&, Config&);
		int parse_cgi(const std::vector<std::string>&, Config&);
		int check_ip_format(const std::string&);

		std::ifstream _config_file;
		std::vector<Server> _servers;
		std::vector<Config> _configs;
		int _max_connection;
		std::vector<int> _client_sd;
		std::map<int, const Config&> _config_assoc;
		fd_set _sockets_list;
		int _highest_socket;
		bool _exit;
};
#endif
