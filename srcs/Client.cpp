/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/10 17:57:28 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

const size_t	Client::_buffer_size(900);

Client::Client(void) :
	_sd(),
	_addr(),
	_socket_len(),
	_configs(),
	_input_str(),
	_requests() {}

Client::Client(int sd, struct sockaddr addr, socklen_t socket_len, const std::list<const Config*> &configs) :
	_sd(sd),
	_addr(addr),
	_socket_len(socket_len),
	_configs(configs),
	_input_str(),
	_request() {}

Client::Client(const Client &x) :
	_sd(x._sd),
	_addr(x._addr),
	_socket_len(x._socket_len),
	_configs(x._configs),
	_input_str(x._input_str),
	_request(x._request) {}

Client::~Client(void) {}

Client
&Client::operator=(const Client &x) {
	_requests = x._requests;
	_input_str = x._input_str;
	return (*this);
}

int
Client::get_sd(void) const {
	return (_sd);
}

/*
struct sockaddr
Client::get_addr(void) const {
	return _addr;
}

socklen_t
Client::get_socket_len(void) const {
	return _socket_len;
}

std::list<const Config*>
Client::get_configs() const {
	return _configs;
}
*/

int
Client::read_socket(void) {
	char	buffer[_buffer_size + 1];
	int		ret;
	if (0 >= (ret = read(_sd, buffer, _buffer_size))) {
		if (0 == ret)
			std::cout << "the client closed the connection." << std::endl;
		else
			std::cout << "error during reading." << std::endl;
		return (FAILURE);
	}
	buffer[ret] = '\0';
	_input_str += (std::string(buffer));
	if (CONTINUE_READING == _request_parsing())
		return (SUCCESS);
	else
		return (_process(ret));
}

Request
&Client::get_first_incomplete_request(void) {
	for (std::vector<Request>::iterator it(_request.begin()) ; it != _request.end() ; it++)
		if (it->get_status() != REQUEST_RECEIVED)
			return (*it);
	_request.push_back(Request());
	return (_request.back());
}

int
Client::_input_str_parsing(void) {
	int	ret(CONTINUE_PARSING);
	while (ret == CONTINUE_PARSING) {
		switch (_status) {
			case START :
				ret = _started_request_parsing();
				break ;
			case REQUEST_LINE_RECEIVED :
				ret = _request_line_received_parsing();
				break ;
			case HEADERS_RECEIVED :
				ret = _headers_received_parsing();
				break ;
			case BODY_RECEIVED :
				ret = _body_received_parsing();
				break ;
			case REQUEST_RECEIVED :
				ret = RECEIVED;
				break ;
		}
	}
	return (ret);
}

/*
 * ici, on traite la requete
 * une fois traitee, la requete doit etre reset
 * return (autre chose que SUCCESS) entrainera la fermeture de la connexion et la suppression de l'objet Client dans Webserver::read_socks()
 */
int
Client::_process(int status) {
	std::cout << "-----> REPONSE : " << status << std::endl << std::endl;
	_request.render();
	_request.reset();
	return (SUCCESS);
}
