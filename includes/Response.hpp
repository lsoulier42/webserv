/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 19:21:17 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/10 23:01:43 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include "AHTTPMessage.hpp"

class Response : public AHTTPMessage {

	public:

		class StatusLine : public AHTTPMessage::AStartLine {

			public:

				StatusLine(void);
				StatusLine(const StatusLine &x);
				~StatusLine(void);
				StatusLine &operator=(const StatusLine &x);

				status_code_t get_status_code(void) const;

				void set_status_code(status_code_t status_code);

				void reset(void);
				void render(void) const;

			private:

				status_code_t _status_code;

		};

		Response(void);
		Response(const Response &x);
		~Response(void);
		Response &operator=(const Response &x);

		StatusLine &get_status_line(void);
		const StatusLine &get_status_line(void) const;
		std::string get_target_path(void) const;
		void set_target_path(const std::string& target_path);
		std::string get_content_type(void) const;
		void set_content_type(const std::string& content_type);

		void reset(void);
		void render(void) const;

	private:
		StatusLine _status_line;
		std::string _target_path;
		std::string _content_type;

};

#endif
