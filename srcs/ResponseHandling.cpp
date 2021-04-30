/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandling.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chris <chris@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/19 17:53:15 by louise            #+#    #+#             */
/*   Updated: 2021/04/24 06:37:09 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseHandling.hpp"
#include "RequestParsing.hpp"

ResponseHandling::ResponseHandling() {

}

ResponseHandling::~ResponseHandling() {

}

void
ResponseHandling::_pick_content_type(Client::exchange_t &exchange) {
	Response& response = exchange.second;
	const std::string& path = response.get_target_path();
	std::string content_type = Syntax::mime_types_tab[APPLICATION_OCTET_STREAM].name;
	std::string extension;
	size_t extension_point_pos;

	extension_point_pos = path.find_last_of('.');
	if (extension_point_pos != std::string::npos) {
		extension = path.substr(extension_point_pos);
		for(size_t i = 0; i < TOTAL_MIME_TYPES; i++) {
			if (extension == Syntax::mime_types_tab[i].ext) {
				content_type = Syntax::mime_types_tab[i].name;
				break ;
			}
		}
	}
	response.set_content_type(content_type);
}

int
ResponseHandling::process_response_headers(Client::exchange_t &exchange) {
	int (*response_handlers[])(Client::exchange_t&) = {&ResponseHandling::_response_allow_handler,
		&ResponseHandling::_response_content_language_handler, &ResponseHandling::_response_content_length_handler,
		&ResponseHandling::_response_content_location_handler, &ResponseHandling::_response_content_type_handler,
		&ResponseHandling::_response_date_handler, &ResponseHandling::_response_last_modified_handler,
		&ResponseHandling::_response_location_handler, &ResponseHandling::_response_retry_after_handler,
		&ResponseHandling::_response_server_handler, &ResponseHandling::_response_transfer_encoding_handler,
		&ResponseHandling::_response_www_authenticate_handler};

	_pick_content_type(exchange);
	for (size_t i = 0; i < TOTAL_RESPONSE_HEADERS; i++) {
		if (!(response_handlers[i])(exchange))
				return FAILURE;
		}
	return (SUCCESS);
}

int ResponseHandling::process_cgi_response_headers(Client::exchange_t &exchange) {
	Response& response = exchange.second;
	size_t response_header_size = 0;
	std::stringstream ss;
	header_name_t cgi_headers[] = {CONTENT_LANGUAGE, DATE, SERVER,
		TRANSFER_ENCODING, WWW_AUTHENTICATE, TOTAL_HEADER_NAMES};
	int (*response_handlers[])(Client::exchange_t&) = { &ResponseHandling::_response_content_language_handler,
		&ResponseHandling::_response_date_handler, &ResponseHandling::_response_server_handler,
		&ResponseHandling::_response_transfer_encoding_handler,	&ResponseHandling::_response_www_authenticate_handler };

	response.set_content_type(response.get_headers().get_unparsed_value(CONTENT_TYPE));
	ss << response.get_body().size();
	response.get_headers().insert(CONTENT_LENGTH, ss.str());
	while (cgi_headers[response_header_size] != TOTAL_HEADER_NAMES)
		response_header_size++;
	for (size_t i = 0; i < response_header_size; i++) {
		if (!response.get_headers().key_exists(cgi_headers[i]) && !(response_handlers[i])(exchange))
			return (FAILURE);
	}
	return (SUCCESS);
}

int
ResponseHandling::_response_allow_handler(Client::exchange_t &exchange) {
	Request &request = exchange.first;
	Response &response = exchange.second;
	std::string method_output;
	status_code_t status_code = response.get_status_line().get_status_code();
	std::list<std::string> allowed_methods = request.get_location()->get_methods();

	if (status_code == METHOD_NOT_ALLOWED || request.get_request_line().get_method() == OPTIONS) {
		for (std::list<std::string>::iterator it = allowed_methods.begin(); it != allowed_methods.end(); it++)
			method_output += *it + ",";
		method_output = method_output.substr(0, method_output.size() - 1);
		response.get_headers().insert(ALLOW, method_output);
	}
	return (SUCCESS);
}

int
ResponseHandling::_response_content_language_handler(Client::exchange_t &exchange) {
	Request &request = exchange.first;
	Response &response = exchange.second;

	if (request.get_headers().key_exists(CONTENT_LANGUAGE))
		response.get_headers().insert(CONTENT_LANGUAGE,
		request.get_headers().get_value(CONTENT_LANGUAGE).front());
	return (SUCCESS);
}

int
ResponseHandling::_response_content_length_handler(Client::exchange_t &exchange) {
	Request				&request = exchange.first;
	Response			&response = exchange.second;
	std::stringstream	ss;
	struct stat			buf;

	if (request.get_request_line().get_method() == PUT)
		ss << 0;
	else if (stat(response.get_target_path().c_str(), &buf) != -1)
		ss << buf.st_size;
	else
		return (SUCCESS);
	response.get_headers().insert(CONTENT_LENGTH, ss.str());
	return (SUCCESS);
}

int
ResponseHandling::_response_content_location_handler(Client::exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	location_str(request_target.substr(0, request_target.find('?')));
	method_t method = request.get_request_line().get_method();

	if (response.get_status_line().get_status_code() < BAD_REQUEST && (method == PUT || method == POST))
		response.get_headers().insert(CONTENT_LOCATION, location_str);
	return (SUCCESS);
}

int
ResponseHandling::_response_content_type_handler(Client::exchange_t &exchange) {
	Response& response = exchange.second;

	response.get_headers().insert(CONTENT_TYPE, response.get_content_type());
	return (SUCCESS);
}

std::string
ResponseHandling::get_current_HTTP_date(void) {
	struct tm *date = NULL;
	char buff[64];
	struct timeval tv;

	gettimeofday(&tv, NULL);
	date = localtime(&(tv.tv_sec));
	strftime(buff, sizeof(buff), "%a, %d %b %Y %T GMT+02", date);
	return (std::string(buff));
}

int
ResponseHandling::_response_date_handler(Client::exchange_t &exchange) {
	Response& response = exchange.second;

	response.get_headers().insert(DATE, get_current_HTTP_date());
	return (SUCCESS);
}

int
ResponseHandling::_response_last_modified_handler(Client::exchange_t &exchange) {
	Response& response = exchange.second;
	struct stat buf;
	struct tm *date = NULL;
	char time_buf[64];

	if (stat(response.get_target_path().c_str(), &buf) != -1) {
		date = localtime(&buf.st_mtim.tv_sec);
		strftime(time_buf, sizeof(time_buf), "%a, %d %b %Y %T GMT+02", date);
		response.get_headers().insert(LAST_MODIFIED, std::string(time_buf));
	}
	return (SUCCESS);
}

int
ResponseHandling::_response_location_handler(Client::exchange_t &exchange) {
	Response&	response = exchange.second;
	Request&	request = exchange.first;
	int status_code = response.get_status_line().get_status_code();

	if ((request.get_request_line().get_method() == PUT
				|| request.get_request_line().get_method() == POST)
			&& (status_code == OK || status_code == CREATED || status_code == NO_CONTENT)
			&& request.get_headers().key_exists(CONTENT_LOCATION))
		response.get_headers().insert(CONTENT_LOCATION,
			request.get_headers().get_unparsed_value(CONTENT_LOCATION));
	return (SUCCESS);
}

int
ResponseHandling::_response_retry_after_handler(Client::exchange_t &exchange) {
	Response& response = exchange.second;
	std::stringstream ss;

	if (response.get_status_line().get_status_code() == SERVICE_UNAVAILABLE) {
		ss << DELAY_RETRY_AFTER;
		response.get_headers().insert(RETRY_AFTER, ss.str());
	}
	return (SUCCESS);
}

int
ResponseHandling::_response_server_handler(Client::exchange_t &exchange) {
	Response& response = exchange.second;

	response.get_headers().insert(SERVER, PROGRAM_VERSION);
	return (SUCCESS);
}

int
ResponseHandling::_response_transfer_encoding_handler(Client::exchange_t &exchange) {
	Response &response = exchange.second;

	response.get_headers().insert(TRANSFER_ENCODING, Syntax::encoding_types_tab[IDENTITY].name);
	return (SUCCESS);
}

int
ResponseHandling::_response_www_authenticate_handler(Client::exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	int status_code = response.get_status_line().get_status_code();

	if (status_code == UNAUTHORIZED && request.get_headers().key_exists(AUTHORIZATION))
		response.get_headers().insert(WWW_AUTHENTICATE, request.get_headers().get_value(AUTHORIZATION).front());
	return (SUCCESS);
}

void
ResponseHandling::generate_basic_headers(Client::exchange_t &exchange) {
	Response&	response = exchange.second;
	Request&	request = exchange.first;
	status_code_t error_code = response.get_status_line().get_status_code();
	method_t method = request.get_request_line().get_method();
	std::stringstream ss;

	ss << response.get_body().size();
	_response_server_handler(exchange);
	_response_date_handler(exchange);
	if (method != DELETE)
		_response_content_type_handler(exchange);
	if ((method == HEAD	&& error_code == METHOD_NOT_ALLOWED)
		|| method == OPTIONS)
		response.get_headers().insert(CONTENT_LENGTH, "0");
	else if (method != DELETE)
		response.get_headers().insert(CONTENT_LENGTH, ss.str());
	if (error_code == METHOD_NOT_ALLOWED || method == OPTIONS)
		_response_allow_handler(exchange);
	if (error_code == FORBIDDEN)
		_response_www_authenticate_handler(exchange);
	if (error_code == SERVICE_UNAVAILABLE)
		_response_retry_after_handler(exchange);
}
