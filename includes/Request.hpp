/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:20:32 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/08 18:52:52 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# define BAD_REQUEST 400
# define URI_TOO_LONG 414
# define NOT_IMPLEMENTED 501
# define CONTINUE 1
# define RECEIVED 2

# include <string>
# include <vector>
# include <list>
# include <stdexcept>
# include <cstdlib>
# include <iostream>

class													Request {

	public:

		class											RequestLine {

			public:

				enum									method_t {
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

														RequestLine(void);
														RequestLine(const RequestLine &x);
														~RequestLine(void);
				RequestLine								&operator=(const RequestLine &x);

				method_t								get_method(void) const;
				const std::string						&get_request_target(void) const;
				const std::string						&get_http_version(void) const;

				void									set_method(const std::string &method_str);
				void									set_request_target(const std::string &request_target);
				void									set_http_version(const std::string &http_version);

				void									reset(void);
				void									render(void) const;

			private:

				struct									_method_tab_entry_t {
					method_t							_method;
					std::string							_str;
					size_t								_length;
				};

				static const _method_tab_entry_t		_method_tab[];

				method_t								_method;
				std::string								_request_target;
				std::string								_http_version;

		};

		class											Headers {

			public:

				struct									header_t {
					std::string							key;
					std::string							value;
				};

														Headers(void);
														Headers(const Headers &x);
														~Headers(void);
				Headers									&operator=(const Headers &x);

				void									insert(const std::string &header_name, const std::string &header_value);
				bool									key_exists(const std::string &key) const;
				std::string								&get_value(const std::string &key) throw (std::invalid_argument);
				const std::string						&get_value(const std::string &key) const throw (std::invalid_argument);
				void									reset(void);
				void									render(void) const;

			private:

				static const size_t						_tab_size;
				std::vector<std::list<header_t>*>		_tab;

				unsigned long							_hash(const char *buf) const;

		};

														Request(void);
		explicit										Request(int socket);
														Request(const Request &x);
														~Request(void);
		Request											&operator=(const Request &x);

		const RequestLine								&get_request_line(void) const;
		const Headers									&get_headers(void) const;
		const std::string								&get_body(void) const;

		int												append(const std::string &data);
		void											reset(void);
		void											render(void) const;

	private:

		enum											_request_status_t {
			EMPTY,
			STARTED,
			REQUEST_LINE_RECEIVED,
			HEADERS_RECEIVED,
			BODY_RECEIVED,
			REQUEST_RECEIVED
		};

		static const size_t								_limit_request_line_size;
		static const size_t								_limit_header_size;
		static const size_t								_limit_body_size;

		_request_status_t								_status;
		std::string										_str;
		RequestLine										_request_line;
		Headers											_headers;
		std::string										_body;

		bool											_body_expected(void) const;
		bool											_body_received(void) const;
		bool											_trailer_expected(void) const;
		bool											_trailer_received(void) const;
		int												_request_parsing(void);
		int												_empty_request_parsing(void);
		int												_started_request_parsing(void);
		int												_request_line_received_parsing(void);
		int												_headers_received_parsing(void);
		int												_body_received_parsing(void);
		int												_collect_request_line_elements(void);
		void											_collect_header(void);
		int												_check_headers(void);
		int												_collect_body(void);

};

#endif
