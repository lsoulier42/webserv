/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResponse.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/16 20:33:46 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/16 22:23:36 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIRESPONSE_HPP
# define CGIRESPONSE_HPP

# include <string>

class CGIResponse {

	public:

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
