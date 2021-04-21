/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cchenot <cchenot@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/02 15:35:41 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/21 19:59:35 by cchenot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

int main(int argc, char **argv) {
	WebServer webserv;
	std::string filepath;

	if (argc >= 2 && argc <= 3) {
		for (int i = 1; i < argc; i++) {
			std::string argument(argv[i]);
			if (argument == "-v") {
				DEBUG_START(true);
			}
			else 
				filepath = argument;
		}
	}
	if (filepath.empty())
		filepath = "default.conf";
	if (!webserv.parsing(filepath))
		return EXIT_FAILURE;
	webserv.setup_servers();
	webserv.routine();
	return (EXIT_SUCCESS);
}
