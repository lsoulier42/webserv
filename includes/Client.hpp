/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chris <chris@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 18:57:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/26 12:47:34 by mdereuse         ###   ########.fr       */
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
# include "Path.hpp"

# ifdef __APPLE__
#  define st_mtim st_mtimespec
# endif

class RequestParsing;
class ResponseHandling;

class Client {

	friend class RequestParsing;
	friend class Request;

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
		std::string get_target_path(void);
		
		int read_socket(void);
		int write_socket(void);
		int read_target_resource(void);
		int read_cgi_output(void);
		int write_cgi_input(void);
		int	write_target_resource(void);

		char *get_ip_addr() const;
		std::string get_ident() const;

	private:

		static const size_t _buffer_size;

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
		int _build_output(exchange_t &exchange);
		int _get_default_index(exchange_t &exchange);
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

		bool _is_cgi_related(const Request &request);

		/* CGI input preparation
		 */
		int _prepare_cgi(exchange_t &exchange);

		/* Interface setting
		 */
		int _create_cgi_child_process(void);
		int _handle_cgi(exchange_t &exchange);
    
		/* CGI response parsing
		 */
		int _cgi_output_parsing(int ret);
		void _collect_cgi_header(void);
		int _check_cgi_headers(void);
		void _collect_cgi_body(void);

		/* Status predicates
		 */
		bool _cgi_header_received(void);
		bool _cgi_headers_received(void);
		bool _cgi_body_received(void);
		bool _cgi_body_expected(void);

		/* Type predicates
		 */
		bool _is_local_redirection(const std::string &location);
		bool _is_client_redirection(const std::string &location);
		bool _is_redirection_status(const std::string &status_line);
		bool _is_document_response(void);
		bool _is_local_redirect_response(void);
		bool _is_client_redirect_response(void);
		bool _is_client_redirect_response_with_document(void);

		/* Handling CGI responses
		 */
		int _handle_cgi_response(void);
		int _handle_local_redirect_cgi_response(void);
		int _handle_client_redirect_cgi_response(void);
		int _handle_client_redirect_doc_cgi_response(void);
		int _handle_document_cgi_response(void);

};

#endif
