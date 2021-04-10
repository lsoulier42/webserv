/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 18:57:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/10 22:40:49 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <unistd.h>
# include <string>
# include <vector>
# include <sys/socket.h>
# include "Request.hpp"
# include "Response.hpp"

# define SUCCESS 0
# define FAILURE -1

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
		std::vector<exchange_t> _exchanges;
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
		int _process(void);

};

#endif
