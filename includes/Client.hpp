/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 18:57:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/10 17:33:37 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <unistd.h>
# include <string>
# include <sys/socket.h>
# include "Request.hpp"

# define SUCCESS 0
# define FAILURE -1

class Client {

	public:

		Client(void);
		explicit Client(int sd, struct sockaddr addr, socklen_t socket_len, const std::list<const Config*> &configs);
		Client(const Client &x);
		~Client(void);
		Client &operator=(const Client &x);

		int get_sd(void) const;
		//struct sockaddr get_addr(void) const;
		//socklen_t get_socket_len(void) const;
		//std::list<const Config*> get_configs() const;

		int read_socket(void);

	private:

		static const size_t _buffer_size;

		const int _sd;
		const struct sockaddr _addr;
		const socklen_t _socket_len;
		const std::list<const Config*> _configs;
		std::string _str;
		std::vector<Request> _requests;

		int _process(int status);
		bool _request_line_received(void) const;
		bool _header_received(void) const;
		bool _headers_received(void) const;
		bool _body_expected(void) const;
		bool _body_received(void) const;
		bool _trailer_expected(void) const;
		bool _trailer_received(void) const;
		int _request_parsing(void);
		int _empty_request_parsing(void);
		int _started_request_parsing(void);
		int _request_line_received_parsing(void);
		int _headers_received_parsing(void);
		int _body_received_parsing(void);
		int _collect_request_line_elements(void);
		int _collect_header(void);
		int _check_headers(void);
		int _collect_body(void);

};

#endif
