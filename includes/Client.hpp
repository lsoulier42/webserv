/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 18:57:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/14 23:12:32 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <unistd.h>
# include <string>
# include <list>
# include <cstdlib>
# include <ctime>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <algorithm>
# include <fcntl.h>
# include <cerrno>
# include "Request.hpp"
# include "Response.hpp"
# include "Syntax.hpp"

# define SUCCESS 0
# define FAILURE -1
# define PENDING 1

class Client {

	public:

		typedef std::pair<Request, Response> exchange_t;

		Client(void);
		explicit Client(int sd, struct sockaddr addr, socklen_t socket_len, const std::list<const Config*> &configs);
		Client(const Client &x);
		~Client(void);
		Client &operator=(const Client &x);

		int get_sd(void) const;
		int get_fd(void) const;
  
		int read_socket(void);
		int read_file(void);

	private:

		static const size_t _buffer_size;

		const int _sd;
		int _fd;
		const struct sockaddr _addr;
		const socklen_t _socket_len;
		const std::list<const Config*> _configs;
		std::string _input_str;
		std::string _output_str;
		std::list<exchange_t> _exchanges;
		bool _closing;

		/* Request reception
		 *
		 *
		 */
		void _failure(exchange_t &exchange, status_code_t status_code);
		bool _request_line_received(const Request &request) const;
		bool _header_received(const Request &request) const;
		bool _headers_received(const Request &request) const;
		bool _body_received(const Request &request) const;
		bool _trailer_received(const Request &request) const;
		bool _trailers_received(const Request &request) const;
		bool _body_expected(const Request &request) const;
		bool _trailer_expected(const Request &request) const;
		void _input_str_parsing(void);
		int _collect_request_line_elements(exchange_t &exchange);
		int _collect_header(exchange_t &exchange);
		int _check_headers(exchange_t &exchange);
		int _check_trailer(exchange_t &exchange);
		int _collect_body(exchange_t &exchange);
		void _pick_virtual_server(Request &request);

		/* Headers handlers
		 *
		 *
		 */
		int _headers_parsers(Request &request);
		int _header_accept_charset_parser(Request &request);
		int _header_accept_language_parser(Request &request);
		int _header_authorization_parser(Request &request);
		int _header_content_length_parser(Request &request);
		int _header_content_type_parser(Request &request);
		int _header_date_parser(Request &request);
		int _header_host_parser(Request &request);
		int _header_referer_parser(Request &request);
		int _header_transfer_encoding_parser(Request &request);
		int _header_user_agent_parser(Request &request);

		/* Header handlers helpers
		 *
		 *
		 */
		static bool _transfer_encoding_chunked(const Request &request);
		static std::list<std::string> _parse_coma_q_factor(const std::string& unparsed_value);
		static bool _comp_q_factor(const std::pair<std::string, float> & a, const std::pair<std::string, float> & b);
		static bool _is_valid_language_tag(const std::string& language_tag);
		static std::string _build_effective_request_URI(const Request::RequestLine& requestLine, const std::string& header_host_value);
		static bool is_valid_http_date(const std::string& date_str);
		void _extract_virtual_server(Request &current_request, const std::string& host_value);


		/* debug function
		 *
		 *
		 */
		void _send_debug_str(const std::string& str) const;

		/* Response sending
		 *
		 *
		 */
		int _process(exchange_t &exchange);
		int _process_error(exchange_t &exchange);
		int _process_GET(exchange_t &exchange);
		std::string _build_path_ressource(Request &request);
		int _open_file_to_read(const std::string &path);
		int _build_output_str(exchange_t &exchange);
		int _write_socket(exchange_t &exchange);

};

#endif
