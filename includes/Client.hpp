/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 18:57:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/13 01:44:05 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <unistd.h>
# include <string>
# include <sstream>
# include <list>
# include <cstdlib>
# include <ctime>
# include <sys/socket.h>
# include <algorithm>
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
  
		int read_socket(void);

	private:

		static const size_t _buffer_size;

		const int _sd;
		const struct sockaddr _addr;
		const socklen_t _socket_len;
		const std::list<const Config*> _configs;
		std::string _input_str;
		std::string _output_str;
		std::list<exchange_t> _exchanges;
		bool _closing;

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
		int _collect_body(exchange_t &exchange);
		int _process(exchange_t &exchange);
		int _fill_response_GET(exchange_t &exchange);
		int _fill_response(exchange_t &exchange);
		int _open_file_to_read(void);
		int _read_file(void);
		int _build_output_str(exchange_t &exchange);
		int _write_socket(exchange_t &exchange);

		/* Headers handlers
		 *
		 *
		 */
		int _headers_handlers(exchange_t& exchange);
		int _header_accept_charset_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_accept_language_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_allow_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_authorization_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_content_length_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_content_language_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_content_location_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_content_type_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_date_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_host_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_last_modified_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_location_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_referer_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_retry_after_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_server_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_transfer_encoding_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_user_agent_handler(exchange_t& exchange, const std::string &unparsed_header_value);
		int _header_www_authenticate(exchange_t& exchange, const std::string &unparsed_header_value);

		/* Header handlers helpers
		 *
		 *
		 */
		static bool _transfer_encoding_chunked(const Request &current_request);
		static std::list<std::string> _parse_coma_q_factor(const std::string& unparsed_value);
		static std::multimap<float, std::string> _split_header_weight(const std::vector<std::string>& elements_split);
		static bool _is_valid_language_tag(const std::string& language_tag);
		static std::list<std::string> _parse_content_type_header_value(const std::string& unparsed_value);
		static std::string _build_effective_request_URI(const Request::RequestLine& requestLine, const std::string& header_host_value);
		void _extract_virtual_server(Request &current_request, const std::string& host_value);

};

#endif
