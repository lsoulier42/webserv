/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 19:21:17 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/29 22:50:56 by mdereuse         ###   ########.fr       */
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

			private:

				status_code_t _status_code;

		};

		enum response_status_t {
			START,
			HEAD_SENT,
			RESPONSE_SENT
		};

		Response(void);
		Response(const Response &x);
		~Response(void);
		Response &operator=(const Response &x);

		response_status_t get_status(void) const;
		size_t get_sending_indicator(void) const;
		StatusLine &get_status_line(void);
		const StatusLine &get_status_line(void) const;
		ByteArray &get_head(void);
		const ByteArray &get_head(void) const;
		ByteArray &get_content(void);
		const ByteArray &get_content(void) const;
		std::string get_target_path(void) const;
		std::string get_content_type(void) const;

		void set_status(response_status_t status);
		void set_sending_indicator(size_t sending_indicator);
		void set_target_path(const std::string& target_path);
		void set_content_type(const std::string& content_type);

		void reset(void);

	private:

		response_status_t _status;
		size_t _sending_indicator;
		StatusLine _status_line;
		ByteArray _head;
		ByteArray _content;
		std::string _target_path;
		std::string _content_type;

};

#endif
