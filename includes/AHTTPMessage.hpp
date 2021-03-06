/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHTTPMessage.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 17:08:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/27 10:16:52 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AHTTPMESSAGE_HPP
# define AHTTPMESSAGE_HPP

# include <string>
# include <cstring>
# include <iostream>
# include <stdexcept>
# include <list>
# include "Syntax.hpp"
# include "Headers.hpp"

class AHTTPMessage {

	public:

		class AStartLine {

			public:

				AStartLine(void);
				AStartLine(const AStartLine &x);
				virtual ~AStartLine(void);

				const std::string &get_http_version(void) const;

				void set_http_version(const std::string &http_version);

				void reset(void);

			protected:

				AStartLine &operator=(const AStartLine &x);

			private:

				std::string _http_version;

		};

		class HTTPHeaders : public Headers {

			public:

				HTTPHeaders(void);
				HTTPHeaders(const HTTPHeaders &x);
				~HTTPHeaders(void);
				HTTPHeaders &operator=(const HTTPHeaders &x);

				void erase(header_name_t key);

				void insert(const header_t &header);
				void insert(header_name_t key, const std::string& unparsed_value);

				bool key_exists(header_name_t key) const;

				const std::list<std::string>& get_value(header_name_t key) const throw (std::invalid_argument);
				const std::string &get_unparsed_value(header_name_t key) const throw (std::invalid_argument);

				void set_value(header_name_t key, const std::list<std::string>& parsed_value) throw (std::invalid_argument);

		};

		AHTTPMessage(void);
		AHTTPMessage(const AHTTPMessage &x) throw(std::bad_alloc);
		virtual ~AHTTPMessage(void);

		HTTPHeaders &get_headers(void);
		const HTTPHeaders &get_headers(void) const;
		ByteArray& get_body(void);
		const ByteArray& get_body(void) const;
		void set_body(const ByteArray& body);

		virtual void reset(void);

	protected:

		AHTTPMessage &operator=(const AHTTPMessage &x) throw(std::bad_alloc);

	private:

		HTTPHeaders _headers;
		ByteArray _body;
};

#endif
