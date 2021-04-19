/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResponse.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/16 20:33:46 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/19 03:26:18 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIRESPONSE_HPP
# define CGIRESPONSE_HPP

# include <string>
# include "Headers.hpp"
# include "Syntax.hpp"

class CGIResponse {

	public:

		class CGIHeaders : public Headers {

			public:

				CGIHeaders(void);
				CGIHeaders(const CGIHeaders &x);
				~CGIHeaders(void);
				CGIHeaders &operator=(const CGIHeaders &x);

				bool key_exists(cgi_header_name_t key) const;

				const std::string &get_unparsed_value(cgi_header_name_t key) const throw (std::invalid_argument);
				const std::list<std::string>& get_value(cgi_header_name_t key) const throw (std::invalid_argument);

		};

		CGIResponse(void);
		CGIResponse(const CGIResponse &x);
		~CGIResponse(void);
		CGIResponse &operator=(const CGIResponse &x);

		CGIHeaders &get_headers(void);
		const CGIHeaders &get_headers(void) const;
		const std::string &get_body(void) const;

		void set_body(const std::string &body);

	private:

		CGIHeaders _headers;
		std::string _body;

};

#endif
