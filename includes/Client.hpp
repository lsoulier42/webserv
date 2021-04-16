/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 18:57:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/16 01:53:10 by mdereuse         ###   ########.fr       */
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
# include <sys/time.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <algorithm>
# include <fcntl.h>
# include <cerrno>
# include <sstream>
# include "Request.hpp"
# include "Response.hpp"
# include "Syntax.hpp"
# include "VirtualServer.hpp"

class Client {

	public:

		typedef std::pair<Request, Response> exchange_t;

		Client(void);
		explicit Client(int sd, struct sockaddr addr, socklen_t socket_len,
			const std::list<const VirtualServer*> &virtual_servers);
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
		const std::list<const VirtualServer*> _virtual_servers;
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
		void _pick_location(Request &request);

		/* Request headers parser
		 *
		 *
		 */
		int _process_request_headers(Request &request);
		int _request_accept_charset_parser(Request &request);
		int _request_accept_language_parser(Request &request);
		int _request_authorization_parser(Request &request);
		int _request_content_length_parser(Request &request);
		int _request_content_type_parser(Request &request);
		int _request_date_parser(Request &request);
		int _request_host_parser(Request &request);
		int _request_referer_parser(Request &request);
		int _request_transfer_encoding_parser(Request &request);
		int _request_user_agent_parser(Request &request);

		/* Request headers helpers
		 *
		 *
		 */
		static bool _transfer_encoding_chunked(const Request &request);
		static std::list<std::string> _parse_coma_q_factor(const std::string& unparsed_value);
		static bool _comp_q_factor(const std::pair<std::string, float> & a, const std::pair<std::string, float> & b);
		static bool _is_valid_language_tag(const std::string& language_tag);
		static std::string _build_effective_request_URI(const Request::RequestLine& requestLine, const std::string& header_host_value);
		static bool is_valid_http_date(const std::string& date_str);

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
		void _generate_error_headers(exchange_t &exchange);
		void _generate_error_page(exchange_t &exchange);

		/* Response headers handlers
		 *
		 *
		 *
		 */
		int _process_response_headers(exchange_t &exchange);
		int _response_allow_handler(exchange_t &exchange);
		int _response_content_language_handler(exchange_t &exchange);
		int _response_content_length_handler(exchange_t &exchange);
		int _response_content_location_handler(exchange_t &exchange);
		int _response_content_type_handler(exchange_t &exchange);
		int _response_date_handler(exchange_t &exchange);
		int _response_last_modified_handler(exchange_t &exchange);
		int _response_location_handler(exchange_t &exchange);
		int _response_retry_after_handler(exchange_t &exchange);
		int _response_server_handler(exchange_t &exchange);
		int _response_transfer_encoding_handler(exchange_t &exchange);
		int _response_www_authenticate_handler(exchange_t &exchange);

		/* Response header helpers
		 *
		 *
		 *
		 */
		void _pick_content_type(exchange_t &exchange);
		static std::string get_current_HTTP_date(void);
		static bool _is_allowed_method(const std::list<std::string>& allowed_methods, method_t method);
		static std::string _html_content_language_parser(const Response& response);
		static std::string _xml_content_language_parser(const Response& response);
		static bool _is_accepted_language(const std::string& language_found, const std::list<std::string>& allowed_languages);
		static bool _is_accepted_charset(const std::string& charset_found, const std::list<std::string>& allowed_charsets);
		static std::string _html_charset_parser(const Response& response);
		static std::string _xml_charset_parser(const Response& response);
};

#endif
