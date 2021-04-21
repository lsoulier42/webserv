/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 18:57:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/19 13:18:04 by mdereuse         ###   ########.fr       */
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
# include "CGIResponse.hpp"

class RequestParsing;
class ResponseHandling;

class Client {

	friend class RequestParsing;

	public:

		typedef std::pair<Request, Response> exchange_t;

		Client(void);
		explicit Client(int sd, struct sockaddr addr, socklen_t socket_len,
			const std::list<const VirtualServer*> &virtual_servers, bool connection_refused);
		Client(const Client &x);
		~Client(void);
		Client &operator=(const Client &x) throw(std::bad_alloc);

		int get_sd(void) const;
		int get_fd(void) const;
		int get_cgi_fd(void) const;
		
		int read_socket(void) throw (ClientError);
		int write_socket(void) throw(std::bad_alloc, ClientError);
		int read_file(void) throw(ClientError);
		int read_cgi(void);
		int free_output(void);

		class ClientError : public std::exception {
			public:
				ClientError(status_code_t error_code) throw() :
					_error_code(error_code) {}
				virtual ~ClientError() throw() {}
				virtual const char* what() const throw() {
					return (Syntax::status_codes_tab[_error_code].reason_phrase.c_str());
				}
				status_code_t get_error_code() const throw() {
					return (_error_code);
				}
			private:
				status_code_t _error_code;
		};

	private:

		static const size_t _buffer_size;

		const int _sd;
		int _fd;
		int _cgi_fd;
		const struct sockaddr _addr;
		const socklen_t _socket_len;
		const std::list<const VirtualServer*> _virtual_servers;
		std::string _input_str;
		std::string _begin_response;
		char*		_output;
		size_t 		_output_size;
		std::string _cgi_output_str;
		std::list<exchange_t> _exchanges;
		bool _closing;
		bool _connection_refused;

		/* debug function
		 *
		 *
		 */
		void _send_debug_str(const std::string& str) const;

		/* Response sending
		 *
		 *
		 */
		int _process_connection_refused();
		int _process(exchange_t &exchange);

		int _process_error(exchange_t &exchange);
		int _process_GET(exchange_t &exchange);
		int _process_cgi(exchange_t &exchange);
		std::string _build_resource_path(Request &request);
		int _open_file_to_read(const std::string &path);
		int _build_begin_response(exchange_t &exchange);
		int _get_default_index(exchange_t &exchange);
		std::string _format_index_path(const std::string& dir_path, const std::string& index_file);
		int _build_output(Response& response) throw(std::bad_alloc);

	/* Autoindex
		 *
		 *
		 *
		 */
		int	_generate_autoindex(exchange_t &exchange);
		void _format_autoindex_entry(std::stringstream& ss, const std::string& filename, exchange_t& exchange, bool is_dir);
		std::string _format_autoindex_page(exchange_t& exchange, const std::set<std::string>& directory_names,
			const std::set<std::string>& file_names);
		std::string _format_directory_name(const std::string& directory_name);
    
		/* CGI
		 *
		 */
    
		bool _is_cgi_related(const Request &request) const;
		std::string _build_cgi_script_path(const Request &request) const;
		int _create_cgi_child_process(void);
		int _cgi_output_str_parsing(void);
		void _collect_cgi_header(CGIResponse &cgi_response);

};

#endif
