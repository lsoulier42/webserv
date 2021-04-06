/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 18:57:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/06 22:39:26 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Request.hpp"
# include <iostream>

class													Client {
	public:
														Client(void);
		explicit										Client(int sd);
														Client(const Client &x);
														~Client(void);
		Client											&operator=(const Client &x);
		void											process(void);
		int												get_sd(void) const;
	private:
		const int										_sd;
		Request											_request;
};

#endif
