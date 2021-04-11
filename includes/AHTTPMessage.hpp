/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHTTPMessage.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 17:08:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/11 01:58:40 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AHTTPMESSAGE_HPP
# define AHTTPMESSAGE_HPP

# include <string>
# include <vector>
# include <list>
# include <iostream>
# include <stdexcept>

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

		class Headers {

			public:

				typedef std::pair<std::string, std::string>	header_t;

				Headers(void);
				Headers(const Headers &x);
				~Headers(void);
				Headers &operator=(const Headers &x);

				void insert(const header_t &header);
				bool key_exists(const std::string &key) const;
				std::string &get_value(const std::string &key) throw (std::invalid_argument);
				const std::string &get_value(const std::string &key) const throw (std::invalid_argument);
				void reset(void);
				void render(void) const;

			private:

				static const size_t _tab_size;
				std::vector<std::list<header_t>*> _tab;

				unsigned long _hash(const char *buf) const;

		};

		AHTTPMessage(void);
		AHTTPMessage(const AHTTPMessage &x);
		virtual ~AHTTPMessage(void);

		Headers &get_headers(void);
		const Headers &get_headers(void) const;
		const std::string &get_body(void) const;

		void set_body(const std::string &body);

		void reset(void);
		virtual void render(void) const = 0;

	protected:

		AHTTPMessage &operator=(const AHTTPMessage &x);

	private:

		Headers _headers;
		std::string _body;

};

#endif
