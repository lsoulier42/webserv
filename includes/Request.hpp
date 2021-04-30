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

		size_t get_body_size_expected(void) const;
		void set_body_size_expected(size_t body_size);

		size_t& get_body_size_received(void);
		size_t get_body_size_received(void) const;

		bool body_is_received(void) const;
		void set_body_received(void);

		int get_tmp_fd(void) const;
		void set_tmp_fd(int fd);

		int write_tmp_file(void);

		std::string get_tmp_filename(void) const;
		void set_tmp_filename(const std::string& tmp_filename);

		size_t& get_tmp_file_size();
		size_t get_tmp_file_size() const;

		bool body_is_writen() const;
		void set_body_written();

		void reset(void);

	private:
		int						_id;
		ByteArray				_raw;
		request_status_t		_status;
		RequestLine				_request_line;
		const VirtualServer*	_virtual_server;
		const Location*			_location;
		const struct sockaddr	_client_addr;
		bool					_chunked_body;
		int						_tmp_fd;
		std::string				_tmp_filename;
		size_t					_body_size_expected;
		size_t					_body_size_received;
		bool 					_body_received;
		size_t					_tmp_file_size;
		bool					_body_written;

		static int _indexes;
};

#endif

