/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 19:21:17 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/10 13:42:58 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "AHTTPMessage.hpp"

class													Response : public AHTTPMessage {

	public:

		class											StatusLine : public AHTTPMessage::AStartLine {

			public:
														StatusLine(void);
														StatusLine(const StatusLine &x);
														StatustLine(void);
				StatusLine								&operator=(const StatusLine &x);

				status_code_t									get_status_code(void) const;
				const std::string						&get_reason_phrase(void) const;

				void									set_status_code(int status_code);
				void									set_reason_phrase(const std::string &reason_phrase);

				void									reset(void);
				void									render(void) const;

			private:

				status_code_t							_status_code;
				std::string								_reason_phrase;

		};

														Response(void);
														Response(const Response &x);
														~Response(void);
		Response										&operator=(const Response &x);

		const StatusLine								&get_status_line(void) const;

		void											set_status_line(const StatusLine &status_line);

		void											reset(void);
		void											render(void) const;

	private:

		StatusLine										_status_line;

};

#endif
