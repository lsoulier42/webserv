/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:20:32 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/09 20:23:07 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# define CONTINUE_READING 1
# define CONTINUE_PARSING 2
# define RECEIVED 3

# include <string>
# include <vector>
# include <list>
# include <stdexcept>
# include <cstdlib>
# include <iostream>
# include "parsing.hpp"
# include "AHTTPMessage.hpp"

class													Request : public AHTTPMessage {

	public:

		class											RequestLine : public AHTTPMessage::AStartLine {

			public:
														RequestLine(void);
														RequestLine(const RequestLine &x);
														~RequestLine(void);
				RequestLine								&operator=(const RequestLine &x);

				method_t								get_method(void) const;
				const std::string						&get_request_target(void) const;

				void									set_method(const std::string &method_str);
				void									set_request_target(const std::string &request_target);

				void									reset(void);
				void									render(void) const;

			private:

				method_t								_method;
				std::string								_request_target;

		};

														Request(void);
		explicit										Request(int socket);
														Request(const Request &x);
														~Request(void);
		Request											&operator=(const Request &x);

		const RequestLine								&get_request_line(void) const;
		size_t											get_limit_body_size() const;

		int												append(const std::string &data);
		void											reset(void);
		void											render(void) const;

	private:

		enum											_request_status_t {
			START,
			REQUEST_LINE_RECEIVED,
			HEADERS_RECEIVED,
			BODY_RECEIVED,
			REQUEST_RECEIVED
		};

		_request_status_t								_status;
		std::string										_str;
		size_t											_limit_body_size;
		RequestLine										_request_line;

		bool											_request_line_received(void) const;
		bool											_header_received(void) const;
		bool											_headers_received(void) const;
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
		int												_collect_header(void);
		int												_check_headers(void);
		int												_collect_body(void);
		//TODO : link to client_max_body_size config value
};

#endif
