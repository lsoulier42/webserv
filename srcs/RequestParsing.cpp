/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParsing.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/19 09:30:19 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/19 13:24:40 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParsing.hpp"

void
RequestParsing::parsing(Client &client) {
	int			ret(0);
	std::string	&input_str(client._input_str);
	while (!client._closing && !input_str.empty()) {
		if (client._exchanges.empty() || client._exchanges.back().first.get_status() == Request::REQUEST_RECEIVED)
			client._exchanges.push_back(std::make_pair(Request(client._virtual_servers.front()), Response()));
		Client::exchange_t	&current_exchange(client._exchanges.back());
		Request				&request(current_exchange.first);
		if (_request_line_received(request, input_str) && SUCCESS != (ret = _collect_request_line_elements(request, input_str))) {
			_failure(client, current_exchange, (status_code_t)ret);
			return ;
		}
		while (_header_received(request, input_str))
			_collect_header(request, input_str);
		if (_headers_received(request, input_str) && SUCCESS != (ret = _check_headers(client, request))) {
			_failure(client, current_exchange, (status_code_t)ret);
			return ;
		}
		if (_body_received(request, input_str))
			_collect_body(request, input_str);
		while (_trailer_received(request, input_str))
			_collect_header(request, input_str);
		if (_trailers_received(request, input_str) && SUCCESS != (ret = _check_trailer(request, input_str))) {
			_failure(client, current_exchange, (status_code_t)ret);
			return ;
		}
		if (request.get_status() != Request::REQUEST_RECEIVED)
			return ;
	}
}

void
RequestParsing::_failure(Client &client, Client::exchange_t &exchange, status_code_t status_code) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);

	client._closing = true;
	request.set_compromising(true);
	request.set_status(Request::REQUEST_RECEIVED);
	response.get_status_line().set_status_code(status_code);
}

bool
RequestParsing::_request_line_received(const Request &request, const std::string &input_str) {
	return (request.get_status() == Request::START && std::string::npos != input_str.find("\r\n"));
}

bool
RequestParsing::_header_received(const Request &request, const std::string &input_str) {
	return (request.get_status() == Request::REQUEST_LINE_RECEIVED
			&& !_headers_received(request, input_str)
			&& std::string::npos != input_str.find("\r\n"));
}

bool
RequestParsing::_headers_received(const Request &request, const std::string &input_str) {
	return (request.get_status() == Request::REQUEST_LINE_RECEIVED
			&& !input_str.compare(0, 2, "\r\n"));
}

bool
RequestParsing::_body_received(const Request &request, const std::string &input_str) {
	return (request.get_status() == Request::HEADERS_RECEIVED
			&& ((_transfer_encoding_chunked(request)
					&& input_str.find("0\r\n\r\n") != std::string::npos)
				|| (request.get_headers().key_exists(CONTENT_LENGTH)
					&& input_str.size() >= static_cast<unsigned long>(std::atol(request.get_headers().get_value(CONTENT_LENGTH).front().c_str())))));
}

bool
RequestParsing::_trailer_received(const Request &request, const std::string &input_str) {
	return (request.get_status() == Request::BODY_RECEIVED
			&& !_trailers_received(request, input_str)
			&& std::string::npos != input_str.find("\r\n"));
}

//TODO:: pas du tout comme ca qu'on repere la fin des trailers, provisoire, pour test
bool
RequestParsing::_trailers_received(const Request &request, const std::string &input_str) {
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
RequestParsing::_body_expected(const Request &request) {
	return (_transfer_encoding_chunked(request)
		|| request.get_headers().key_exists(CONTENT_LENGTH));
}

bool
RequestParsing::_trailer_expected(const Request &request) {
	return ((request.get_headers().key_exists(TRAILER)));
}

int
RequestParsing::_collect_request_line_elements(Request &request, std::string &input_str) {
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
RequestParsing::_collect_header(Request &request, std::string &input_str) {
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
RequestParsing::_check_headers(Client &client, Request &request) {
	client._input_str.erase(0, client._input_str.find("\r\n") + 2);
	if (_process_request_headers(client, request) == FAILURE)
		return (BAD_REQUEST);
	if (_body_expected(request))
		request.set_status(Request::HEADERS_RECEIVED);
	else
		request.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

int
RequestParsing::_check_trailer(Request &request, std::string &input_str) {
	(void)input_str;
	request.set_status(Request::REQUEST_RECEIVED);
	return (SUCCESS);
}

int
RequestParsing::_collect_body(Request &request, std::string &input_str) {
	size_t		body_length(0);

	if (_transfer_encoding_chunked(request))
		body_length = input_str.find("0\r\n\r\n") + 5;
	else if (request.get_headers().key_exists(CONTENT_LENGTH))
		body_length = static_cast<unsigned long>(std::atol(request.get_headers().get_value(CONTENT_LENGTH).front().c_str()));
	request.set_body(input_str.substr(0, body_length));
	input_str.erase(0, body_length);
	if (_trailer_expected(request))
		request.set_status(Request::BODY_RECEIVED);
	else
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
	return SUCCESS;
}

/* basic language tag check :
 * regular examples : fr, en-gb, en-US, fr-Latin-FR, cel-gaulish
 * accepted formats, parsed on '-' delimiter :
 * - language : 2 or 3 alpha char
 * - region (if exist) : >= 2 alpha-num char (both upper and lower case accepted)
 * - script (if exist) : >= 2 alpha-num char
 * - more than 3 compounds are accepted without format checking
 * - asterisk form '*'
 * case insensitive
 *
 */

bool
RequestParsing::is_valid_language_tag(const std::string& language_tag) {
	std::vector<std::string> compounds = Syntax::split(language_tag, "-");
	std::string language, script, region;

	if (language_tag == "*")
		return true;
	language = compounds[0];
	if (!Syntax::str_is_alpha(language)
		|| language.size() < 2 || language.size() > 3)
		return false;
	if (compounds.size() > 2) {
		script = compounds[1];
		region = compounds[2];
		if (!Syntax::str_is_alnum(script) || !Syntax::str_is_alnum(region))
			return false;
		if (script.size() < 2 || region.size() < 2)
			return false;
	}
	if (compounds.size() == 2) {
		region = compounds[1];
		if (!Syntax::str_is_alnum(region) || region.size() < 2)
			return false;
	}
	return true;
}

int
RequestParsing::_request_accept_language_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(ACCEPT_LANGUAGE);
	std::list<std::string> languages_list = _parse_coma_q_factor(unparsed_header_value);

	for(std::list<std::string>::const_iterator it = languages_list.begin(); it != languages_list.end(); it++) {
		if (!is_valid_language_tag(*it))
			return FAILURE;
	}
	request.get_headers().set_value(ACCEPT_LANGUAGE, languages_list);
	return SUCCESS;
}

int
RequestParsing::_request_authorization_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(AUTHORIZATION);
	std::vector<std::string> compounds = Syntax::split(unparsed_header_value, " ");
	std::list<std::string> definitive_value;

	if (compounds.size() != 2)
		return FAILURE;
	definitive_value.push_back(compounds[0]); // authorization type
	definitive_value.push_back(compounds[1]); // authorization credentials
	request.get_headers().set_value(AUTHORIZATION, definitive_value);
	return SUCCESS;
}

int
RequestParsing::_request_content_length_parser(Request &request) {
	std::string content_length_str = request.get_headers().get_unparsed_value(CONTENT_LENGTH);
	std::list<std::string> definitive_value;

	if(request.get_headers().key_exists(TRANSFER_ENCODING))
		return FAILURE;
	if (!Syntax::str_is_num(content_length_str))
		return FAILURE;
	definitive_value.push_back(content_length_str);
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
		return FAILURE;
	request.get_headers().set_value(CONTENT_TYPE, content_type_list);
	return SUCCESS;
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
			return true;
	}
	return false;
}

/* _date_handler :
 * accept the three HTTP dates formats, in order in the std::string array :
 * - real example : Sun, 06 Nov 1994 08:49:37 GMT
 * - obsolete 1 (RFC 850) example : Sunday, 06-Nov-94 08:49:37 GMT
 * - obsolete 2 (ANSI C) example : Sun Nov  6 08:49:37 1994
 *
 */

int
RequestParsing::_request_date_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(DATE);
	std::list<std::string> definitive_value;

	if (!is_valid_http_date(unparsed_header_value))
		return FAILURE;
	definitive_value.push_back(unparsed_header_value);
	request.get_headers().set_value(DATE, definitive_value);
	return SUCCESS;
}

int
RequestParsing::_request_host_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(HOST);
	std::vector<std::string> compounds;
	std::list<std::string> definitive_value;

	if (unparsed_header_value.find_first_of(WHITESPACES) != std::string::npos)
		return FAILURE;
	compounds = Syntax::split(unparsed_header_value, ":");
	if (compounds.size() > 2)
		return FAILURE;

	definitive_value.push_back(compounds[0]);// host name
	if (compounds.size() == 2)
		definitive_value.push_back(compounds[1]); //port
	request.get_headers().set_value(HOST, definitive_value);
	return (SUCCESS);
}

int
RequestParsing::_request_referer_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(REFERER);
	URI_form_t uri_form = Syntax::get_URI_form(unparsed_header_value);

	if (uri_form != ABSOLUTE_URI && uri_form != PARTIAL_URI)
		return FAILURE;
	request.get_headers().set_value(REFERER,
		std::list<std::string>(1, unparsed_header_value));
	return SUCCESS;
}

int
RequestParsing::_request_transfer_encoding_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(TRANSFER_ENCODING);
	std::list<std::string> encoding_types_list = _parse_coma_q_factor(unparsed_header_value);

	for(std::list<std::string>::iterator it = encoding_types_list.begin();
		it != encoding_types_list.end(); it++)
		if (!Syntax::is_accepted_value(*it, Syntax::encoding_types_tab, TOTAL_ENCODING_TYPES))
			return FAILURE;
	request.get_headers().set_value(TRANSFER_ENCODING, encoding_types_list);
	return SUCCESS;
}

int
RequestParsing::_request_user_agent_parser(Request &request) {
	std::string unparsed_header_value = request.get_headers().get_unparsed_value(USER_AGENT);
	std::vector<std::string> compounds = Syntax::split(unparsed_header_value, " ");
	std::list<std::string> definitive_value;

	for(size_t i = 0; i < compounds.size(); i++)
		definitive_value.push_back(compounds[i]);
	request.get_headers().set_value(USER_AGENT, definitive_value);
	return SUCCESS;
}

int
RequestParsing::_process_request_headers(Client &client, Request &request) {
	typedef int (*header_parser_t)(Request &);
	const AHTTPMessage::HTTPHeaders& headers = request.get_headers();

	header_parser_t handler_functions[] = {&RequestParsing::_request_accept_charset_parser,
		&RequestParsing::_request_accept_language_parser, &RequestParsing::_request_authorization_parser,
		&RequestParsing::_request_content_length_parser, &RequestParsing::_request_content_type_parser, &RequestParsing::_request_date_parser, NULL,
		&RequestParsing::_request_referer_parser, &RequestParsing::_request_transfer_encoding_parser, &RequestParsing::_request_user_agent_parser,
	};
	if (!request.get_headers().key_exists(HOST)
			|| _request_host_parser(request) == FAILURE)
		return FAILURE;
	_pick_virtual_server(client, request);
	for(size_t i = 0; i < TOTAL_REQUEST_HEADERS; i++) {
		if (Syntax::request_headers_tab[i].header_index != HOST
			&& headers.key_exists(Syntax::request_headers_tab[i].header_index)) {
			if ((*handler_functions[i])(request) == FAILURE)
				return FAILURE;
		}
	}
	return SUCCESS;
}