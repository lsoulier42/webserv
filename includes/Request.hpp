/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:20:32 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/23 09:35:30 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <iostream>
# include <sys/socket.h>
# include <arpa/inet.h>
# include "Syntax.hpp"
# include "AHTTPMessage.hpp"
# include "VirtualServer.hpp"

class Location;
class Client;

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
		Request(const Client &client);
		Request(const Request &x);
		~Request(void);
		Request &operator=(const Request &x);

		request_status_t get_status(void) const;
		RequestLine &get_request_line(void);
		const RequestLine &get_request_line(void) const;
		const VirtualServer* get_virtual_server() const;
		const Location* get_location() const;
		const struct sockaddr &get_client_addr(void) const;
		void set_location(const Location* location);

		void set_status(request_status_t status);
		void set_virtual_server(const VirtualServer* virtual_server);

		ByteArray& get_raw(void);
		const ByteArray& get_raw(void) const;
		void set_raw(const ByteArray& raw);

		char *get_ip_addr() const;
		std::string get_ident() const;
		bool is_chunked() const;
		void set_chunked();

		int get_id() const;

		void reset(void);

	private:
		int _id;
		ByteArray _raw;
		request_status_t _status;
		RequestLine _request_line;
		const VirtualServer* _virtual_server;
		const Location*	_location;
		const struct sockaddr _client_addr;
		bool _chunked_body;

		static int _indexes;
};

#endif

