/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/24 15:33:40 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/25 09:34:10 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>

int		main(void)
{
	int			size_buffer(3);
	int			ret;
	std::string	text("aaaaaaaaaaaaaaaaa");
	std::string	text1("bbbbbbbbbbbbbbb");
	int	fd(open("file", O_WRONLY|O_NONBLOCK|O_TRUNC, 0666));
	for (int i(0) ; i < 3 ; i++) {
		if (!text.empty()) {
			ret = write(fd, text.c_str(), size_buffer);
			text.erase(0, ret);
		}
	}
	int	fd1(open("file", O_WRONLY|O_NONBLOCK|O_TRUNC, 0666));
	while (!(text.empty() && text1.empty())) {
		if (!text.empty()) {
			ret = write(fd, text.c_str(), size_buffer);
			text.erase(0, ret);
			if (text.empty())
				close(fd);
		}
		if (!text1.empty()) {
			ret = write(fd1, text1.c_str(), size_buffer);
			text1.erase(0, ret);
			if (text1.empty())
				close(fd1);
		}
	}
	return (0);
}
