/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:20:32 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/05 13:46:40 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <unistd.h>

class						Request {
	public:
							Request(void);
		explicit			Request(int socket);
							Request(const Request &x);
							~Request(void);
		Request				&operator=(const Request &x);
		void				process(void);
	private:
		enum				_method_t {
			GET,
			HEAD,
			POST,
			PUT,
			DELETE,
			CONNECT,
			OPTIONS,
			TRACE,
			NOT_RECOGNIZED
		};
		struct				_request_line_t {
			_method_t		_method;
			char			*_request_target;
			char			*_http_version;
		};
		static const size_t	_limit_request_line_size;
		const int			_socket;
		_request_line_t		_request_line;
		void				_read_request_line(void);
};

#endif
