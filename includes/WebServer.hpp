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
		static std::string methods_array[TOTAL_METHODS];

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
		enum t_methods {
			GET,
			HEAD,
			POST,
			PUT,
			DELETE,
			CONNECT,
			OPTIONS,
			TRACE,
			TOTAL_METHODS
		};
		enum t_informational_codes {
			CONTINUE 100,
			SWITCHING_PROTOCOLS
		};
		enum t_successful_codes {
			OK 200,
			CREATED,
			ACCEPTED,
			NON_AUTHORITATIVE_INFORMATION,
			NO_CONTENT,
			RESET_CONTENT,
			PARTIAL_CONTENT
		};
		enum t_redirection_codes {
			MULTIPLE_CHOICES 300,
			MOVED_PERMANENTLY,
			FOUND,
			SEE_OTHER,
			NOT_MODIFIED,
			USE_PROXY,
			TEMPORARY_REDIRECT 307
		};
		enum t_client_error_codes {
			BAD_REQUEST 400,
			UNAUTHORIZED,
			PAYMENT_REQUIRED,
			FORBIDDEN,
			NOT_FOUND,
			METHOD_NOT_ALLOWED,
			NOT_ACCEPTABLE,
			PROXY_AUTHENTICATION_REQUIRED,
			REQUEST_TIMEOUT,
			CONFLICT,
			GONE,
			LENGTH_REQUIRED,
			PRECONDITION_FAILED,
			PAYLOAD_TOO_LARGE,
			URI_TOO_LONG,
			UNSUPPORTED_MEDIA_TYPE,
			RANGE_NOT_SATISFIABLE,
			EXPECTATION_FAILED,
			UPGRADE_REQUIRED 426
		};
		enum t_server_error_codes {
			INTERNAL_SERVER_ERROR 500,
			NOT_IMPLEMENTED,
			BAD_GATEWAY,
			SERVICE_UNAVAILABLE,
			GATEWAY_TIMEOUT,
			HTTP_VERSION_NOT_SUPPORTED
		};
		int check_config_file(const std::string& filepath);
		int check_main_bloc();
		int check_server_bloc();
		int check_location_bloc();
		static bool is_num(const char* str);
		static bool is_informational_code(int code);
		static bool is_successful_code(int code);
		static bool is_redirection_code(int code);
		static bool is_client_error_code(int code);
		static bool is_server_error_code(int code);
		static bool is_error_code(int code);
		static int method_index(const std::string&);

		static std::string trim_comments(const std::string& line_buffer);
		static std::string trim_whitespaces(const std::string& line_buffer);
		static int trim_semicolon(std::vector<std::string>& tokens);
		static std::vector<std::string> WebServer::split(const std::string& line_buffer, const std::string& charset);
		static std::vector<std::string> split_whitespaces(const std::string& line_buffer);
		static int check_ip_format(const std::string&);
		static int check_path(const std::string&);

		int parse_listen(const std::vector<std::string>&, Config&);
		int parse_server_name(const std::vector<std::string>&, Config&);
		int parse_error_page(const std::vector<std::string>&, Config&);
		int parse_client_max_body_size(const std::vector<std::string>&, Config&);
		int parse_location(const std::vector<std::string>&, Config&);
		int parse_methods(const std::vector<std::string>&, AConfig&);
		int parse_methods_config(const std::vector<std::string>&, Config&);
		int parse_methods_location(const std::vector<std::string>&, Location&);
		int parse_root(const std::vector<std::string>&, AConfig&);
		int parse_root_config(const std::vector<std::string>&, Config&);
		int parse_root_location(const std::vector<std::string>&, Location&);
		int parse_autoindex(const std::vector<std::string>&, AConfig&);
		int parse_autoindex_config(const std::vector<std::string>&, Config&);
		int parse_autoindex_location(const std::vector<std::string>&, Location&);
		int parse_index(const std::vector<std::string>&, AConfig&);
		int parse_index_config(const std::vector<std::string>&, Config&);
		int parse_index_location(const std::vector<std::string>&, Location&);
		int parse_upload_dir(const std::vector<std::string>&, Config&);
		int parse_cgi(const std::vector<std::string>&, AConfig&);
		int parse_cgi_config(const std::vector<std::string>&, Config&);
		int parse_cgi_location(const std::vector<std::string>&, Location&);

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
