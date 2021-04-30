/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResponse.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/16 20:33:46 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/30 01:45:04 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIRESPONSE_HPP
# define CGIRESPONSE_HPP

# include <string>
# include "Headers.hpp"
# include "ByteArray.hpp"
# include "Syntax.hpp"

class CGIResponse {

	public:

		class CGIHeaders : public Headers {

			public:

				CGIHeaders(void);
				CGIHeaders(const CGIHeaders &x);
				~CGIHeaders(void);
				CGIHeaders &operator=(const CGIHeaders &x);

				void erase(cgi_header_name_t key);

				bool key_exists(cgi_header_name_t key) const;

				const std::string &get_unparsed_value(cgi_header_name_t key) const throw (std::invalid_argument);
				const std::list<std::string>& get_value(cgi_header_name_t key) const throw (std::invalid_argument);

		};

		enum status_t {
			START,
			HEADERS_RECEIVED,
			RESPONSE_RECEIVED
		};

		enum type_t {
			DOCUMENT,
			LOCAL_REDIRECT,
			CLIENT_REDIRECT,
			CLIENT_REDIRECT_DOC,
			NO_TYPE
		};

		CGIResponse(void);
		CGIResponse(const CGIResponse &x);
		~CGIResponse(void);
		CGIResponse &operator=(const CGIResponse &x);

		status_t get_status(void) const;
		type_t get_type(void) const;
		CGIHeaders &get_headers(void);
		const CGIHeaders &get_headers(void) const;
		bool get_content_reception(void) const;
//		const ByteArray &get_body(void) const;

		void set_status(status_t status);
		void set_type(type_t type);
		void set_content_reception(bool content_reception);
//		void set_body(const ByteArray &body);

		void reset(void);

	private:

		status_t _status;
		type_t _type;
		CGIHeaders _headers;
		bool _content_reception;

};

#endif
