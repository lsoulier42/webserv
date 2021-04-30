/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chris <chris@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 18:57:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/30 02:30:49 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <unistd.h>
# include <string>
# include <cstring>
# include <list>
# include <set>
# include <cstdlib>
# include <ctime>
# include <sys/time.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <dirent.h>
# include <algorithm>
# include <fcntl.h>
# include <cstdio>
# include <cerrno>
# include <sstream>
# include "Request.hpp"
# include "Response.hpp"
# include "Syntax.hpp"
# include "VirtualServer.hpp"
# include "CGIMetaVariables.hpp"
# include "CGIScriptArgs.hpp"
# include "CGIResponse.hpp"

# ifdef __APPLE__
#  define st_mtim st_mtimespec
# endif

class RequestParsing;
class ResponseHandling;
class CGI;

class Client {

	friend class RequestParsing;
	friend class Request;
	friend class CGI;

	public:

		typedef std::pair<Request, Response> exchange_t;

		Client(void);
		explicit Client(int sd, struct sockaddr addr, socklen_t socket_len,
			const std::list<const VirtualServer*> &virtual_servers, bool connection_refused);
		Client(const Client &x);
		~Client(void);
		Client &operator=(const Client &x);

		int get_sd(void) const;
		int get_fd(void) const;
		int get_cgi_input_fd(void) const;
		int get_cgi_output_fd(void) const;
		int get_file_write_fd(void) const;
		std::string get_PUT_file(void);
		
		int read_socket(void);
		int write_socket(void);
		int read_target_resource(void);
		int	write_target_resource(void);
		int read_cgi_output(void);
		int write_cgi_input(void);

		int open_file_to_write(void);

		char *get_ip_addr() const;
		std::string get_ident() const;

		static const size_t read_buffer_size;

	private:

		const int _sd;
		int _fd;
		int _cgi_input_fd;
		int _cgi_output_fd;
		int	_file_write_fd;
		const struct sockaddr _addr;
		const socklen_t _socket_len;
		const std::list<const VirtualServer*> _virtual_servers;
		ByteArray _input;
		ByteArray _output;
		ByteArray _cgi_input;
		ByteArray _cgi_output;
		CGIResponse _cgi_response;
		ByteArray _file_write_str;
		std::list<exchange_t> _exchanges;
		bool _closing;
		bool _connection_refused;

		/* Response sending
		 *
		 *
		 */
		int _process_connection_refused();
		int _process(exchange_t &exchange);

		int _process_error(exchange_t &exchange);
		int _process_GET(exchange_t &exchange);
		int _process_HEAD(exchange_t &exchange);
		int _process_POST(exchange_t &exchange);
		int	_process_PUT(exchange_t &exchange);
		int	_process_DELETE(exchange_t &exchange);
		int	_process_CONNECT(exchange_t &exchange);
		int	_process_OPTIONS(exchange_t &exchange);
		int	_process_TRACE(exchange_t &exchange);

		std::string _build_resource_path(Request &request);
		int _open_file_to_read(const std::string &path);
		void _build_head_response(exchange_t &exchange);
		std::string _format_index_path(const std::string& dir_path, const std::string& index_file);
		void _rebuild_request_target(exchange_t &exchange, const std::string& path);

		/* Autoindex
		 *
		 *
		 *
		 */
		int	_generate_autoindex(exchange_t &exchange);
		void _format_autoindex_entry(std::stringstream& ss, const std::string& filename, exchange_t& exchange, bool is_dir);
		std::string _format_autoindex_page(exchange_t& exchange, const std::set<std::string>& directory_names,
			const std::set<std::string>& file_names);

    
		/* CGI
		 *
		 */

		int _cgi_init(exchange_t &exchange);

};

#endif
