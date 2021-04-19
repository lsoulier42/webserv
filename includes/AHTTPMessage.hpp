/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHTTPMessage.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 17:08:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/18 10:04:52 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AHTTPMESSAGE_HPP
# define AHTTPMESSAGE_HPP

# include <string>
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
				virtual void render(void) const = 0;

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

				void insert(const header_t &header);
				void insert(header_name_t key, const std::string& unparsed_value);

				bool key_exists(header_name_t key) const;

				const std::list<std::string>& get_value(header_name_t key) const throw (std::invalid_argument);
				const std::string &get_unparsed_value(header_name_t key) const throw (std::invalid_argument);

				void set_value(header_name_t key, const std::list<std::string>& parsed_value) throw (std::invalid_argument);

		};

		AHTTPMessage(void);
		AHTTPMessage(const AHTTPMessage &x);
		virtual ~AHTTPMessage(void);

		HTTPHeaders &get_headers(void);
		const HTTPHeaders &get_headers(void) const;
		const std::string &get_body(void) const;

		void set_body(const std::string &body);

		void reset(void);
		virtual void render(void) const = 0;

	protected:

		AHTTPMessage &operator=(const AHTTPMessage &x);

	private:

		HTTPHeaders _headers;
		std::string _body;

};

#endif
