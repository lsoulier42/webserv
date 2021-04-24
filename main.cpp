/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/24 15:33:40 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/24 18:19:49 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <fcntl.h>
#include <iostream>

int		main(void)
{
	int	fd(open("patate", O_WRONLY|O_NONBLOCK, 0666));
	int	fd1(open("patate", O_WRONLY|O_NONBLOCK, 0666));
	int	fd2(open("patate", O_WRONLY|O_NONBLOCK, 0666));
	std::cout << fd << std::endl;
	std::cout << fd1 << std::endl;
	std::cout << fd2 << std::endl;
	write(fd, "patate", 6);
	write(fd1, "tomate", 6);
	write(fd2, "courgette", 9);
	write(fd, "jaune", 5);
	write(fd1, "rouge", 5);
	write(fd2, "vert", 4);
	return (0);
}
