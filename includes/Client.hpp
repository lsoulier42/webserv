/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 18:57:59 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/08 00:09:43 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <unistd.h>
# include <string>
# include "Request.hpp"

# define SUCCESS 0
# define FAILURE -1

class													Client {

	public:

														Client(void);
		explicit										Client(int sd);
														Client(const Client &x);
														~Client(void);
		Client											&operator=(const Client &x);

		int												get_sd(void) const;

		int												read_socket(void);

	private:

		static const size_t								_buffer_size;

		const int										_sd;
		Request											_request;

		int												_process(int status);

};

#endif
