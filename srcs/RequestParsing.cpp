/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParsing.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/19 09:30:19 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/28 09:54:29 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParsing.hpp"

size_t RequestParsing::_uri_max_size = 8192;
size_t RequestParsing::_header_max_size = 8192;

void
RequestParsing::parsing(Client &client) {
	ByteArray& 	input(client._input);
	
	while (!client._closing && !input.empty()) {

		if (client._exchanges.empty() || client._exchanges.back().first.get_status() == Request::REQUEST_RECEIVED)
			client._exchanges.push_back(std::make_pair(Request(client), Response()));

		Client::exchange_t	&current_exchange(client._exchanges.back());
		Request				&request(current_exchange.first);

		_handle_request_line(current_exchange, input);
		_handle_headers(client, current_exchange, input);
		_handle_body(current_exchange, input);
		_handle_trailers(current_exchange, input);
		if (request.get_status() != Request::REQUEST_RECEIVED)
			return ;
	}
}

void
RequestParsing::_handle_request_line(Client::exchange_t &exchange, ByteArray &input) {
	Request				&request(exchange.first);
	Response			&response(exchange.second);
	int					ret(0);

	if (_request_line_received(request, input)
		&& SUCCESS != (ret = _collect_request_line_elements(request, input))) {
		DEBUG_COUT("Request line parsing failure (" <<
			request.get_ident()<< ") : " << Syntax::status_codes_tab[(status_code_t)ret].reason_phrase);
		_failure(response, (status_code_t)ret);
	}
}

void
RequestParsing::_handle_headers(Client &client, Client::exchange_t &exchange, ByteArray &input) {
	Request				&request(exchange.first);
	Response			&response(exchange.second);
	int					ret(0);

	while (_header_received(request, input))
		_collect_header(request, input);
	if (_headers_received(request, input) && SUCCESS != (ret = _check_headers(client, request))) {
		DEBUG_COUT("Headers parsing failure (" << request.get_ident() <<
			") : " << Syntax::status_codes_tab[(status_code_t)ret].reason_phrase);
		_failure(response, (status_code_t) ret);
	}
}

void
RequestParsing::_handle_body(Client::exchange_t &exchange, ByteArray& input) {
	Request				&request(exchange.first);
	Response			&response(exchange.second);
	std::stringstream	ss;
	int					fd;

	if (!request.body_is_writen() && request.get_status() == Request::HEADERS_RECEIVED
		&& request.get_tmp_fd() == 0) {
		ss << "./tmp/i" << request.get_id();
		request.set_tmp_filename(ss.str());
		fd = open(request.get_tmp_filename().c_str(),
			O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, S_IRUSR | S_IWUSR);
		if (fd < 0) {
			DEBUG_COUT("Error during creation of tmp file (" << request.get_ident() << ")");
			_failure(response, INTERNAL_SERVER_ERROR);
		}
		request.set_tmp_fd(fd);
	}
	if (request.get_status() == Request::HEADERS_RECEIVED && !request.body_is_received()) {
		if (request.is_chunked())
			_collect_chunked(request, input);
		else
			_collect_unchunked(request, input);
	}
	if (request.body_is_received()) {
		if (_trailer_expected(request))
			request.set_status(Request::BODY_RECEIVED);
		else
			request.set_status(Request::REQUEST_RECEIVED);
	}
}

void
RequestParsing::_handle_trailers(Client::exchange_t &exchange, ByteArray &input) {
	Request				&request(exchange.first);
	Response			&response(exchange.second);
	int					ret(0);

	while (_trailer_received(request, input))
		_collect_header(request, input);
	if (_trailers_received(request, input) && SUCCESS != (ret = _check_trailer(request, input)))
		_failure(response, (status_code_t)ret);
}

void
RequestParsing::_collect_chunked(Request &request, ByteArray &input) {
	std::stringstream ss;
	size_t size_pos;
	size_t line_len;

	while ((size_pos = input.find("\r\n")) != ByteArray::npos) {
		ss.clear();
		line_len = 0;
		ss << std::hex << input.substr(0, size_pos);
		ss >> line_len;
		if (line_len == 0 && input.size() == 5) {
			input.pop_front(size_pos + 4);
			request.set_body_received();
			DEBUG_COUT("Chunked body has been completely received (" << request.get_ident() << ")");
			return ;
		} else if (line_len == 0) {
			return ;
		}
		if (input.size() - size_pos - 2 < line_len + 2)
			return ;
		input.pop_front(size_pos + 2);
		request.get_body().append(input.sub_byte_array(0, line_len));
		request.get_body_size_received() += line_len;
		input.pop_front(line_len + 2);
	}
}

void
RequestParsing::_collect_unchunked(Request &request, ByteArray &input) {
	size_t 	input_size = input.size(),
			body_received = request.get_body_size_received(),
			body_expected = request.get_body_size_expected(),
			end_of_body = input_size;

	if (input_size + body_received > body_expected) {
		end_of_body = body_expected - body_received;
		request.get_body().append(input.sub_byte_array(0, end_of_body));
		request.set_body_received();
		DEBUG_COUT("Not chunked body has been completely received (" << request.get_ident() << ")");
	}
	else
		request.get_body().append(input);
	request.get_body_size_received() += end_of_body;
	input.pop_front(end_of_body);
}

void
RequestParsing::_failure(Response &response, status_code_t status_code) {
	if (response.get_status_line().get_status_code() == TOTAL_STATUS_CODE)
		response.get_status_line().set_status_code(status_code);
}

bool
RequestParsing::_request_line_received(const Request &request, const ByteArray &input) {
	return (request.get_status() == Request::START && ByteArray::npos != input.find("\r\n"));
}

bool
RequestParsing::_header_received(const Request &request, const ByteArray &input) {
	return (request.get_status() == Request::REQUEST_LINE_RECEIVED
			&& !_headers_received(request, input)
			&& ByteArray::npos != input.find("\r\n"));
}

bool
RequestParsing::_headers_received(const Request &request, const ByteArray &input) {
	std::string input_str(input.c_str(), input.size());
	return (request.get_status() == Request::REQUEST_LINE_RECEIVED
			&& !input_str.compare(0, 2, "\r\n"));
}

bool
RequestParsing::_trailer_received(const Request &request, const ByteArray &input) {
	return (request.get_status() == Request::BODY_RECEIVED
			&& !_trailers_received(request, input)
			&& ByteArray::npos != input.find("\r\n"));
}

bool
RequestParsing::_trailers_received(const Request &request, const ByteArray &input) {
	std::string input_str(input.c_str(), input.size());
	return (request.get_status() == Request::BODY_RECEIVED
			&& !input_str.compare(0, 2, "\r\n"));
}

bool
RequestParsing::_body_expected(const Request &request) {
	return (request.get_headers().key_exists(TRANSFER_ENCODING)
			|| request.get_headers().key_exists(CONTENT_LENGTH));
}

bool
RequestParsing::_trailer_expected(const Request &request) {
	return ((request.get_headers().key_exists(TRAILER)));
}

bool
RequestParsing::_is_allowed_method(const std::list<std::string>& allowed_methods, const std::string& method) {
	for(std::list<std::string>::const_iterator it = allowed_methods.begin(); it != allowed_methods.end(); it++) {
		if (*it == method)
			return true;
	}
	return false;
}

int
RequestParsing::_collect_request_line_elements(Request &request, ByteArray &input) {
	size_t						end_rl(input.find("\r\n"));
	std::string					request_line = input.substr(0, end_rl);
	std::vector<std::string> 	rl_elements = Syntax::split(request_line, " ");
	std::list<std::string>		allowed_methods;

	request.set_status(Request::REQUEST_LINE_RECEIVED);
	DEBUG_COUT("Request line received: \"" << request_line << "\" (" << request.get_ident() << ")");
	if (rl_elements.size() != 3) {
		input.pop_front(end_rl + 2);
		return (BAD_REQUEST);
	}
	if (!Syntax::is_accepted_value(rl_elements[0], Syntax::method_tab, DEFAULT_METHOD)) {
		input.pop_front(end_rl + 2);
		return (NOT_IMPLEMENTED);
	}
	if (rl_elements[1].size() > _uri_max_size || (rl_elements[1][0] != '/' && rl_elements[1][0] != '*')) {
		input.pop_front(end_rl + 2);
		return (BAD_REQUEST);
	}
	if (rl_elements[2].find('/') == std::string::npos || rl_elements[2].find('.') == std::string::npos) {
		input.pop_front(end_rl + 2);
		return (BAD_REQUEST);
	}
	std::vector<std::string> http_elements = Syntax::split(rl_elements[2], "/");
	if (http_elements.size() != 2 || http_elements[0] != "HTTP") {
		input.pop_front(end_rl + 2);
		return (BAD_REQUEST);
	}
	double http_version = strtod(http_elements[1].c_str(), NULL);
	if (http_version > 1.1 || http_version < 1.0) {
		input.pop_front(end_rl + 2);
		return (HTTP_VERSION_NOT_SUPPORTED);
	}
	request.get_request_line().set_method(rl_elements[0]);
	request.get_request_line().set_request_target(rl_elements[1]);
	request.get_request_line().set_http_version(rl_elements[2]);
	request.set_raw(input.sub_byte_array(0, end_rl + 2));
	input.pop_front(end_rl + 2);
	_pick_location(request);
	if (request.get_location()) {
		allowed_methods = request.get_location()->get_methods();
		if (!_is_allowed_method(allowed_methods, rl_elements[0]))
			return (METHOD_NOT_ALLOWED);
	}
	return (SUCCESS);
}

void
RequestParsing::_collect_header(Request &request, ByteArray &input) {
	size_t				col(0);
	size_t				end_header(input.find("\r\n"));
	std::string			header = input.substr(0, end_header);
	header_t			current_header;

	if (ByteArray::npos != (col = header.find_first_of(':'))) {
		DEBUG_COUT("Request header received: \"" << header << "\"(" << request.get_ident() << ")");
		current_header.name = header.substr(0, col);
		current_header.unparsed_value = Syntax::trim_whitespaces(header.substr(col + 1));
		request.get_headers().insert(current_header);
	}
	request.get_raw() += input.sub_byte_array(0, end_header + 2);
	input.pop_front(end_header + 2);
}

int
RequestParsing::_check_headers(Client &client, Request &request) {
	Headers		headers = request.get_headers();
	int			ret = 0;
	bool 		host_found = false;
	std::string	http_version = request.get_request_line().get_http_version();

	if (!http_version.empty())
		http_version = http_version.substr(5);
	for(Headers::iterator it = headers.begin(); it != headers.end(); it++) {
		if (it->unparsed_value.size() > _header_max_size
			|| (host_found && it->name == Syntax::headers_tab[HOST].name && http_version == "1.0"))
			ret = BAD_REQUEST;
		if (it->name == Syntax::headers_tab[HOST].name)
			host_found = true;
	}
	if (!host_found && http_version == "1.1")
		ret = BAD_REQUEST;
	if (ret == 0)
		ret = _process_request_headers(client, request);
	client._input.pop_front(client._input.find("\r\n") + 2);
	if (_body_expected(request))
		request.set_status(Request::HEADERS_RECEIVED);
	else
		request.set_status(Request::REQUEST_RECEIVED);
	return (ret);
}

int
RequestParsing::_check_trailer(Request &request, ByteArray &input) {
	(void)input;
	request.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

void
RequestParsing::_pick_virtual_server(Client &client, Request &request) {
	std::vector<std::string> host_elements;
	std::list<std::string> server_names;

	request.set_virtual_server(client._virtual_servers.front());
	if (!request.get_headers().key_exists(HOST))
		return ;
	for(std::list<const VirtualServer*>::const_iterator it = client._virtual_servers.begin();
		it != client._virtual_servers.end() ; it++) {
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
RequestParsing::_pick_location(Request &request) {
	std::string					request_target(request.get_request_line().get_request_target());
	std::string					absolute_path(request_target.substr(0, request_target.find('?')));
	const std::list<Location>	&locations(request.get_virtual_server()->get_locations());

	if (absolute_path[0] != '/')
		absolute_path = "/" + absolute_path;
	for (std::list<Location>::const_iterator it(locations.begin()) ; it != locations.end() ; it++)
		if (!absolute_path.compare(0, (it->get_path()).size(), it->get_path())
				&& request.get_location()->get_path().size() < it->get_path().size())
			request.set_location(&(*it));
}

/*
 * Headers parsers
 */

bool RequestParsing::_comp_q_factor(const std::pair<std::string, float> & a, const std::pair<std::string, float> & b) {
	return a.second > b.second;
}

std::list<std::string>
RequestParsing::_parse_coma_q_factor(const std::string& unparsed_value) {
	std::list<std::string> new_list;
	std::vector<std::string> elements_split;
	std::list<std::pair<std::string, float> > elements;
	float q_factor;
	size_t size, q_factor_pos;

	elements_split = Syntax::split(unparsed_value, ", ");
	size = elements_split.size();
	for (size_t i = 0; i < size; i++) {
		q_factor_pos = elements_split[i].find(";q=");
		if (q_factor_pos != std::string::npos) {
			q_factor = strtod(elements_split[i].substr(q_factor_pos + 4).c_str(), NULL);
			elements_split[i] = elements_split[i].substr(0, q_factor_pos);
		}
		else
			q_factor = 1.0;
		elements.push_back(std::make_pair(elements_split[i], q_factor));
	}
	elements.sort(_comp_q_factor);
	for(std::list<std::pair<std::string, float> >::iterator it = elements.begin(); it != elements.end(); it++) {
		new_list.push_back(it->first);
	}
	return new_list;
}

int
RequestParsing::_request_accept_charset_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(ACCEPT_CHARSET);
	std::list<std::string> charsets_list = _parse_coma_q_factor(unparsed_header_value);

	request.get_headers().set_value(ACCEPT_CHARSET, charsets_list);
	return (SUCCESS);
}

bool
RequestParsing::is_valid_language_tag(const std::string& language_tag) {
	std::vector<std::string> compounds = Syntax::split(language_tag, "-");
	std::string language, script, region;

	if (language_tag == "*")
		return (true);
	language = compounds[0];
	if (!Syntax::str_is_alpha(language)
		|| language.size() < 2 || language.size() > 3)
		return (false);
	if (compounds.size() > 2) {
		script = compounds[1];
		region = compounds[2];
		if (!Syntax::str_is_alnum(script) || !Syntax::str_is_alnum(region))
			return (false);
		if (script.size() < 2 || region.size() < 2)
			return (false);
	}
	if (compounds.size() == 2) {
		region = compounds[1];
		if (!Syntax::str_is_alnum(region) || region.size() < 2)
			return (false);
	}
	return (true);
}

int
RequestParsing::_request_accept_language_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(ACCEPT_LANGUAGE);
	std::list<std::string> languages_list = _parse_coma_q_factor(unparsed_header_value);

	for(std::list<std::string>::const_iterator it = languages_list.begin(); it != languages_list.end(); it++) {
		if (!is_valid_language_tag(*it))
			return (FAILURE);
	}
	request.get_headers().set_value(ACCEPT_LANGUAGE, languages_list);
	return (SUCCESS);
}

int
RequestParsing::_request_authorization_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(AUTHORIZATION);
	std::vector<std::string> compounds = Syntax::split(unparsed_header_value, " ");
	std::list<std::string> definitive_value;

	if (compounds.size() != 2)
		return (FAILURE);
	definitive_value.push_back(compounds[0]); // authorization type
	definitive_value.push_back(compounds[1]); // authorization credentials
	request.get_headers().set_value(AUTHORIZATION, definitive_value);
	return (SUCCESS);
}

int
RequestParsing::_request_content_length_parser(Request &request) {
	std::string content_length_str = request.get_headers().get_unparsed_value(CONTENT_LENGTH);
	std::list<std::string> definitive_value;
	unsigned long value;

	if(request.get_headers().key_exists(TRANSFER_ENCODING))
		return (FAILURE);
	if (!Syntax::str_is_num(content_length_str))
		return (FAILURE);
	value = std::strtol(content_length_str.c_str(), NULL, 10);
	definitive_value.push_back(content_length_str);
	request.set_body_size_expected(value);
	request.get_headers().set_value(CONTENT_LENGTH, definitive_value);
	return (SUCCESS);
}

int
RequestParsing::_request_content_type_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(CONTENT_TYPE);
	std::list<std::string> content_type_list;
	std::string mime_type;
	std::string parameter;
	size_t semicolon_pos;

	semicolon_pos = unparsed_header_value.find_first_of(';');
	mime_type = unparsed_header_value.substr(0, semicolon_pos);
	content_type_list.push_back(mime_type);
	if (semicolon_pos != std::string::npos) {
		parameter = unparsed_header_value.substr(semicolon_pos + 1);
		content_type_list.push_back(parameter);
	}
	if (!Syntax::is_accepted_value<Syntax::mime_type_entry_t>(mime_type,
		Syntax::mime_types_tab, TOTAL_MIME_TYPES))
		return (FAILURE);
	request.get_headers().set_value(CONTENT_TYPE, content_type_list);
	return (SUCCESS);
}

bool
RequestParsing::is_valid_http_date(const std::string& date_str) {
	char* strptime_ret;
	int i = -1;
	std::string HTTP_date_fmt[3] = {"%a, %d %b %Y %T", "%A, %d-%b-%y %T %Z", "%a %b  %d %T %Y"};
	struct tm timeval;

	while(++i < 3) {
		strptime_ret = strptime(date_str.c_str(), HTTP_date_fmt[i].c_str(), &timeval);
		if (strptime_ret && (*strptime_ret == '\0' || *strptime_ret == 'G'))
			return (true);
	}
	return (false);
}

int
RequestParsing::_request_date_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(DATE);
	std::list<std::string> definitive_value;

	if (!is_valid_http_date(unparsed_header_value))
		return (FAILURE);
	definitive_value.push_back(unparsed_header_value);
	request.get_headers().set_value(DATE, definitive_value);
	return (SUCCESS);
}

int
RequestParsing::_request_host_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(HOST);
	std::vector<std::string> compounds;
	std::list<std::string> definitive_value;

	compounds = Syntax::split(unparsed_header_value, ":");
	if (compounds.size() > 2)
		return (FAILURE);
	definitive_value.push_back(compounds[0]);// host name
	if (compounds.size() == 2)
		definitive_value.push_back(Syntax::trim_whitespaces(compounds[1])); //port
	request.get_headers().set_value(HOST, definitive_value);
	return (SUCCESS);
}

int
RequestParsing::_request_referer_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(REFERER);
	URI_form_t uri_form = Syntax::get_URI_form(unparsed_header_value);

	if (uri_form != ABSOLUTE_URI && uri_form != PARTIAL_URI)
		return (FAILURE);
	request.get_headers().set_value(REFERER,
		std::list<std::string>(1, unparsed_header_value));
	return (SUCCESS);
}

int
RequestParsing::_request_transfer_encoding_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(TRANSFER_ENCODING);
	std::list<std::string> encoding_types_list = _parse_coma_q_factor(unparsed_header_value);

	for(std::list<std::string>::iterator it = encoding_types_list.begin();
		it != encoding_types_list.end(); it++)
		if (!Syntax::is_accepted_value(*it, Syntax::encoding_types_tab, TOTAL_ENCODING_TYPES))
			return (FAILURE);
	if (encoding_types_list.back() == Syntax::encoding_types_tab[CHUNKED].name)
		request.set_chunked();
	request.get_headers().set_value(TRANSFER_ENCODING, encoding_types_list);
	return (SUCCESS);
}

int
RequestParsing::_request_user_agent_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(USER_AGENT);
	std::vector<std::string> compounds = Syntax::split(unparsed_header_value, " ");
	std::list<std::string> definitive_value;

	for(size_t i = 0; i < compounds.size(); i++)
		definitive_value.push_back(compounds[i]);
	request.get_headers().set_value(USER_AGENT, definitive_value);
	return (SUCCESS);
}

int
RequestParsing::_process_request_headers(Client &client, Request &request) {
	typedef int (*header_parser_t)(Request &);
	const AHTTPMessage::HTTPHeaders& headers = request.get_headers();

	header_parser_t handler_functions[] = {&RequestParsing::_request_accept_charset_parser,
		&RequestParsing::_request_accept_language_parser, &RequestParsing::_request_authorization_parser,
		&RequestParsing::_request_content_length_parser, &RequestParsing::_request_content_type_parser,
		&RequestParsing::_request_date_parser, &RequestParsing::_request_host_parser,
		&RequestParsing::_request_referer_parser, &RequestParsing::_request_transfer_encoding_parser,
		&RequestParsing::_request_user_agent_parser,
	};
	_pick_virtual_server(client, request);
	for(size_t i = 0; i < TOTAL_REQUEST_HEADERS; i++) {
		if (headers.key_exists(Syntax::request_headers_tab[i].header_index)) {
			if ((*handler_functions[i])(request) == FAILURE)
				return (BAD_REQUEST);
		}
	}
	return SUCCESS;
}
