/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:20:32 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/07 19:02:46 by mdereuse         ###   ########.fr       */
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

class										Request {

	public:

											Request(void);
		explicit							Request(int socket);
											Request(const Request &x);
											~Request(void);
		Request								&operator=(const Request &x);

		int									append(const std::string &data);
		void								reset(void);

	private:

		enum								_request_status_t {
			EMPTY,
			STARTED,
			REQUEST_LINE_RECEIVED,
			HEADERS_RECEIVED,
			REQUEST_RECEIVED
		};

		struct								_request_line_t {

			enum							_method_t {
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

			struct							_method_entry_t {
				_method_t					_method;
				std::string					_str;
				size_t						_length;
			};

											_request_line_t(void);
											_request_line_t(const _request_line_t &x);
			_request_line_t					&operator=(const _request_line_t &x);

			void							_reset(void);

			static const _method_entry_t	_method_tab[];

			_method_t						_method;
			std::string						_request_target;
			std::string						_http_version;

		};

		struct								_headers_t {

			struct							_header_entry_t {
				std::string					key;
				std::string					value;
			};

											_headers_t(void);
											_headers_t(const _headers_t &x);
											~_headers_t(void);
			_headers_t						&operator=(const _headers_t &x);

			void							_reset(void);
			unsigned long					_hash(const char *buf) const;

			static const size_t				_headers_tab_size;

			_headers_entry_t				**_tab;

		};

		static const size_t					_limit_request_line_size;
		static const size_t					_limit_header_size;

		_request_status_t					_status;
		std::string							_str;
		_request_line_t						_request_line;
		_headers_t							_headers;

		int									_parse_request(void);
		int									_parse_request_line(void);
		void								_parse_header(void);

};

#endif
