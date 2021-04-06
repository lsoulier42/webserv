/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:20:32 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/05 20:19:26 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# define SUCCESS 0
# define INTERRUPT -1
# define BAD_REQUEST 400
# define URI_TOO_LONG 414
# define NOT_IMPLEMENTED 501

# include <iostream>
# include <string>
# include <unistd.h>
# include <sys/socket.h>

class								Request {
	public:
									Request(void);
		explicit					Request(int socket);
									Request(const Request &x);
									~Request(void);
		Request						&operator=(const Request &x);
		void						process(void);
	private:
		enum						_method_t {
			GET,
			HEAD,
			POST,
			PUT,
			DELETE,
			CONNECT,
			OPTIONS,
			TRACE,
			DEFAULT
		};
		struct						_method_entry {
			_method_t				_method;
			std::string				_str;
			size_t					_length;
		};
		struct						_request_line_t {
			_method_t				_method;
			std::string				_request_target;
			std::string				_http_version;
		};
		static const size_t			_limit_request_line_size;
		static const size_t			_limit_header_size;
		static const _method_entry	_method_tab[];
		const int					_socket;
		_request_line_t				_request_line;
		int							_read_request_line(char *buffer);
		int							_parse_request_line(char *buffer);
		int							_process_headers(void);
		int							_process_request_line(void);
};

#endif
