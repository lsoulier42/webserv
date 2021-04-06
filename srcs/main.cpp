/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/02 15:35:41 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/02 15:35:42 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

int main(int argc, char **argv) {
	WebServer webserv;
	std::string filepath;

	if (argc >= 2 && argc <= 3) {
		for (int i = 1; i < 3; i++) {
			std::string argument(argv[i]);
			if (argument == "-verbose")
				WebServer::verbose = true;
			else
				filepath = argument;
		}
	}
	webserv.setup_servers();
	webserv.routine();
	return (0);
}
