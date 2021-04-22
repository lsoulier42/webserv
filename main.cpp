/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/22 01:15:50 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/22 01:57:53 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./includes/Path.hpp"
#include <iostream>

int		main(void)
{
	std::string	test("asjofdisjfd/oijf");
	if (Path::is_query_string(test))
		std::cout << "ok" << std::endl;
	else
		std::cout << "not ok" << std::endl;
	return (0);
}
