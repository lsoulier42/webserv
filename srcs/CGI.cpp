/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/27 15:15:10 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/27 16:15:55 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI(void) {}

bool
CGI::is_cgi_related(const Request &request) {
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	path(request_target.substr(0, request_target.find("?")));
	std::string extension(request.get_location()->get_cgi_extension());

	return (path.find(".") != std::string::npos
			&& !extension.empty()
			&& !(path.substr(path.rfind("."))).compare(0, extension.size(), extension));
}

//TODO:: generer un nom unique pour le fichier temporaire
//TODO:: eviter la copie du body de la requete
int
CGI::init(Client &client) {
	Request		&request(client._exchanges.front().first);

	if (0 > (client._cgi_input_fd = open("/tmp/cgi_input_webserver", O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, S_IRUSR | S_IWUSR)))
		return (FAILURE);
	client._cgi_input = request.get_body();
	return (SUCCESS);
}

int
CGI::write_input(Client &client) {
	exchange_t	&exchange(client._exchanges.front());
	Response	&response(exchange.second);
	size_t		buffer_size(std::min(Client::_buffer_size, client._cgi_input.size()));
	ssize_t		ret;

	ret = write(client._cgi_input_fd, client._cgi_input.c_str(), buffer_size);
	if (ret < 0) {
		close(client._cgi_input_fd);
		client._cgi_input_fd = 0;
		client._cgi_input.clear();
		return (FAILURE);
	}
	client._cgi_input.pop_front(ret);
	if (client._cgi_input.empty()) {
		close(client._cgi_input_fd);
		client._cgi_input_fd = 0;
		return (_launch_script(exchange));
	}
	return (SUCCESS);
}

int
CGI::_create_child_process(void) {
	pid_t	pid;

	pid = fork();
	while (pid == -1 && errno == EAGAIN)
		pid = fork();
	return (pid);
}

int
CGI::
