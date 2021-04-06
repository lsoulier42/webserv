/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/02 15:35:41 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/06 20:07:17 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

int main(int argc, char **argv) {
	Server server1_test("0.0.0.0", 8090);
	std::vector<Server> vector1;
	vector1.push_back(server1_test);
	WebServer webserv(vector1);

	(void)argc;
	(void)argv;
	webserv.setup_servers();
	webserv.routine();
	return (0);
}
