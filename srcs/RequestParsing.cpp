/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParsing.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/19 09:30:19 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/19 10:52:15 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

void
RequestParsing::_input_str_parsing(Client &client) {
	std::string	&input_str(client._input_str);
	while (!client._closing && !input_str.empty()) {
		if (client._exchanges.empty() || client._exchanges.back().first.get_status() == Request::REQUEST_RECEIVED)
			client._exchanges.push_back(std::make_pair(Request(client._virtual_servers.front()), Response()));
		exchange_t	&current_exchange(client._exchanges.back());
		Request		&request(current_exchange.first);
		if (_request_line_received(request, input_str) && SUCCESS != (ret = _collect_request_line_elements(request, input_str))) {
			_failure(client, current_exchange, ret);
			return ;
		}
		while (_header_received(request, input_str))
			_collect_header(client, current_exchange);
		if (_headers_received(request, input_str) && SUCCESS != (ret = _check_headers(request, input_str))) {
			_failure(client, current_exchange, ret);
			return ;
		}
		if (_body_received(request, input_str))
			_collect_body(client, current_exchange);
		while (_trailer_received(request, input_str))
			_collect_header(client, current_exchange);
		if (_trailers_received(request, input_str) && SUCCESS != _check_trailer(request, input_str))
			return ;
		if (request.get_status() != Request::REQUEST_RECEIVED)
			return ;
	}
}

void
RequestParsing::_failure(Client &client, exchange_t &exchange, status_code_t status_code) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);

	client._closing = true;
	request.set_compromising(true);
	request.set_status(Request::REQUEST_RECEIVED);
	response.get_status_line().set_status_code(status_code);
}

bool
RequestParsing::_request_line_received(const Request &request, const std::string &input_str) const {
	return (request.get_status() == Request::START && std::string::npos != input_str.find("\r\n"));
}

bool
RequestParsing::_header_received(const Request &request, const std::string &input_str) const {
	return (request.get_status() == Request::REQUEST_LINE_RECEIVED
			&& !_headers_received(request)
			&& std::string::npos != input_str.find("\r\n"));
}

bool
RequestParsing::_headers_received(const Request &request, const std::string &input_str) const {
	return (request.get_status() == Request::REQUEST_LINE_RECEIVED
			&& !input_str.compare(0, 2, "\r\n"));
}

bool
RequestParsing::_body_received(const Request &request, const std::string &input_str) const {
	return (request.get_status() == Request::HEADERS_RECEIVED
			&& ((_transfer_encoding_chunked(request)
					&& input_str.find("0\r\n\r\n") != std::string::npos)
				|| (request.get_headers().key_exists(CONTENT_LENGTH)
					&& input_str.size() >= static_cast<unsigned long>(std::atol(request.get_headers().get_value(CONTENT_LENGTH).front().c_str())))));
}

bool
RequestParsing::_trailer_received(const Request &request, const std::string &input_str) const {
	return (request.get_status() == Request::BODY_RECEIVED
			&& !_trailers_received(request)
			&& std::string::npos != input_str.find("\r\n"));
}

//TODO:: pas du tout comme ca qu'on repere la fin des trailers, provisoire, pour test
bool
RequestParsing::_trailers_received(const Request &request, const std::string &input_str) const {
	return (request.get_status() == Request::BODY_RECEIVED
			&& !input_str.compare(0, 2, "\r\n"));
}

//TODO:: pas du tout comme ca qu'on repere la fin des trailers, provisoire, pour test
bool
RequestParsing::_trailers_received(const Request &request, const std::string &input_str) const {
	return (request.get_status() == Request::BODY_RECEIVED
			&& !input_str.compare(0, 2, "\r\n"));
}

bool
RequestParsing::_transfer_encoding_chunked(const Request &request) {
	if (request.get_headers().key_exists(TRANSFER_ENCODING)) {
		const std::list<std::string>& transfer_encoding_values = request.get_headers().get_value(TRANSFER_ENCODING);
		if (!transfer_encoding_values.empty())
			if (std::find(transfer_encoding_values.begin(), transfer_encoding_values.end(),
				Syntax::encoding_types_tab[CHUNKED].name) != transfer_encoding_values.end())
				return true;
	}
	return false;
}

bool
RequestParsing::_body_expected(const Request &request) const {
	return (_transfer_encoding_chunked(request)
		|| request.get_headers().key_exists(CONTENT_LENGTH));
}

bool
RequestParsing::_trailer_expected(const Request &request) const {
	return ((request.get_headers().key_exists(TRAILER)));
}

int
Client::_collect_request_line_elements(Request &request, std::string &input_str) {
	size_t		first_sp(0);
	size_t		scnd_sp(0);
	size_t		end_rl(input_str.find("\r\n"));

	if (std::string::npos == (first_sp = input_str.find_first_of(' '))
			|| std::string::npos == (scnd_sp = input_str.find_first_of(' ', first_sp + 1))) {
		input_str.erase(0, end_rl + 2);
		return (BAD_REQUEST);
	}
	request.get_request_line().set_method(input_str.substr(0, first_sp));
	request.get_request_line().set_request_target(input_str.substr(first_sp + 1, scnd_sp - first_sp - 1));
	if (request.get_request_line().get_request_target()[0] != '/')
		return (BAD_REQUEST);
	request.get_request_line().set_http_version(input_str.substr(scnd_sp + 1, (end_rl - scnd_sp - 1)));
	input_str.erase(0, end_rl + 2);
	if (DEFAULT_METHOD == request.get_request_line().get_method())
		return (NOT_IMPLEMENTED);
	request.set_status(Request::REQUEST_LINE_RECEIVED);
	_pick_location(request);
	return (SUCCESS);
}

void
Client::_collect_header(Request &request, std::string &input_str) {
	size_t				col(0);
	size_t				end_header(input_str.find("\r\n"));
	header_t			current_header;

	if (std::string::npos != (col = input_str.find_first_of(':'))) {
		current_header.name = input_str.substr(0, col);
		current_header.unparsed_value = Syntax::trim_whitespaces(input_str.substr(col + 1, (end_header - col - 1)));
		request.get_headers().insert(current_header);
	}
	input_str.erase(0, end_header + 2);
}

int
Client::_check_headers(Request &request, std::string &input_str) {
	input_str.erase(0, input_str.find("\r\n") + 2);
	if (_process_request_headers(request) == FAILURE)
		return (BAD_REQUEST);
	if (_body_expected(request, input_str))
		request.set_status(Request::HEADERS_RECEIVED);
	else
		request.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

int
Client::_check_trailer(Request &request, std::string &input_str) {
	(void)input_str;
	request.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

int
Client::_collect_body(Request &request, std::string &input_str) {
	size_t		body_length(0);

	if (_transfer_encoding_chunked(request))
		body_length = input_str.find("0\r\n\r\n") + 5;
	else if (request.get_headers().key_exists(CONTENT_LENGTH))
		body_length = static_cast<unsigned long>(std::atol(request.get_headers().get_value(CONTENT_LENGTH).front().c_str()));
	request.set_body(input_str.substr(0, body_length));
	input_str.erase(0, body_length);
	if (_trailer_expected(request, input_str))
		request.set_status(Request::BODY_RECEIVED);
	else
		request.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

void
Client::_pick_virtual_server(Request &request) {
	std::vector<std::string> host_elements;
	std::list<std::string> server_names;

	request.set_virtual_server(_virtual_servers.front());
	if (!request.get_headers().key_exists(HOST))
		return ;
	for(std::list<const VirtualServer*>::const_iterator it = _virtual_servers.begin();
		it != _virtual_servers.end() ; it++) {
		server_names = (*it)->get_server_names();
		for (std::list<std::string>::const_iterator cit = server_names.begin();
			cit != server_names.end(); cit++) {
			if (*cit == request.get_headers().get_unparsed_value(HOST)) {
				request.set_virtual_server(*it);
				return ;
			}
		}
	}
}

void
Client::_pick_location(Request &request) {
	std::string					request_target(request.get_request_line().get_request_target());
	std::string					absolute_path(request_target.substr(0, request_target.find('?')));
	const std::list<Location>	&locations(request.get_virtual_server()->get_locations());

	for (std::list<Location>::const_iterator it(locations.begin()) ; it != locations.end() ; it++)
		if (!absolute_path.compare(0, (it->get_path()).size(), it->get_path())
				&& request.get_location()->get_path().size() < it->get_path().size())
			request.set_location(&(*it));
}
