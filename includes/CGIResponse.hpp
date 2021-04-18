/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResponse.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/16 20:33:46 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/18 06:40:07 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIRESPONSE_HPP
# define CGIRESPONSE_HPP

# include <string>

class CGIResponse {

	public:

		/*
		class CGIHeaders : public Headers {

			public:

				CGIHeaders(void);
				CGIHeaders(const CGIHeaders &x);
				~CGIHeaders(void);
				CGIHeaders &operator=(const CGIHeaders &x);

				void insert(const Headers::header_t &header);

				bool key_exists(header_name_t key) const;

				const std::list<std::string>& get_value(header_name_t key) const throw (std::invalid_argument);
				const std::string &get_unparsed_value(header_name_t key) const throw (std::invalid_argument);

				void set_value(header_name_t key, const std::list<std::string>& parsed_value) throw (std::invalid_argument);

				void reset(void);
				void render(void) const;

			private:

				static const size_t _tab_size;
				std::vector<std::list<header_t>*> _tab;

				unsigned long _hash(const char *buf) const;

		};
		*/

		CGIResponse(void);
		CGIResponse(const CGIResponse &x);
		~CGIResponse(void);
		CGIResponse &operator=(const CGIResponse &x);

		const std::string &get_body(void)const;

		void set_body(const std::string &body);

	private:

		std::string _body;

};

#endif
