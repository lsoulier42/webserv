/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:20:32 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/14 23:09:52 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <iostream>
# include "parsing.hpp"
# include "Syntax.hpp"
# include "AHTTPMessage.hpp"

class Request : public AHTTPMessage {

	public:

		class RequestLine : public AHTTPMessage::AStartLine {

			public:

				RequestLine(void);
				RequestLine(const RequestLine &x);
				~RequestLine(void);
				RequestLine &operator=(const RequestLine &x);

				method_t get_method(void) const;
				const std::string &get_request_target(void) const;

				void set_method(const std::string &method_str);
				void set_request_target(const std::string &request_target);

				void reset(void);
				void render(void) const;

			private:

				method_t _method;
				std::string _request_target;

		};

		enum request_status_t {
			START,
			REQUEST_LINE_RECEIVED,
			HEADERS_RECEIVED,
			BODY_RECEIVED,
			REQUEST_RECEIVED,
			REQUEST_PROCESSED
		};

		Request(void);
		Request(const Config *virtual_server);
		Request(const Request &x);
		~Request(void);
		Request &operator=(const Request &x);

		request_status_t get_status(void) const;
		RequestLine &get_request_line(void);
		const RequestLine &get_request_line(void) const;
		bool get_compromising(void) const;
		const Config* get_virtual_server() const;

		void set_status(request_status_t status);
		void set_compromising(bool compromising);
		void set_virtual_server(const Config* virtual_server);

		void reset(void);
		void render(void) const;

	private:

		request_status_t _status;
		RequestLine _request_line;
		bool _compromising;
		const Config* _virtual_server;
};

#endif
