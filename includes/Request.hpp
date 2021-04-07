/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:20:32 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/07 14:22:13 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# define BAD_REQUEST 400
# define URI_TOO_LONG 414
# define NOT_IMPLEMENTED 501
# define SUCCESS 0

# include <string>
# include <iostream>

class								Client;

class								Request {

	public:

									Request(void);
		explicit					Request(int socket);
									Request(const Request &x);
									~Request(void);
		Request						&operator=(const Request &x);

		void						reset(void);
		int							append(const std::string &data);

	private:

		enum						_request_status_t {
			EMPTY,
			STARTED,
			REQUEST_LINE_RECEIVED,
			HEADERS_RECEIVED,
			REQUEST_RECEIVED
		};

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

		_request_status_t			_status;
		std::string					_str;
		_request_line_t				_request_line;

		int							_parse_request(void);
		int							_parse_request_line(void);
};

#endif
