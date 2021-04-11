/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 18:57:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/11 03:59:40 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <unistd.h>
# include <string>
# include <list>
# include <stdlib.h>
# include <sys/socket.h>
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
		int _fill_response(exchange_t &exchange);
		int _open_file_to_read(void);
		int _read_file(void);
		int _build_output_str(exchange_t &exchange);
		int _write_socket(exchange_t &exchange);

		const Config* _extract_virtual_server(const exchange_t& exchange) const;
		/* Headers handlers
		 *
		 *
		 */
		int _accept_charset_handler(const AHTTPMessage::Headers::header_t& header);
		int _accept_language_handler(const AHTTPMessage::Headers::header_t& header);

};

#endif
