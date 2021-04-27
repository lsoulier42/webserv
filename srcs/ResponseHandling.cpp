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


std::string
ResponseHandling::_html_content_language_parser(const Response& response) {
	std::string content_language_str, line_tag;
	std::string body(response.get_body().c_str(), response.get_body().size());
	size_t html_tag_pos, end_html_tag_pos, lang_pos, end_lang_pos;

	html_tag_pos = body.find("<html");
	if (html_tag_pos != std::string::npos) {
		line_tag = body.substr(html_tag_pos);
		end_html_tag_pos = line_tag.find_first_of('>');
		if (end_html_tag_pos != std::string::npos) {
			line_tag = line_tag.substr(0, end_html_tag_pos);
			lang_pos = line_tag.find("lang=\"");
			if (lang_pos != std::string::npos) {
				line_tag = line_tag.substr(lang_pos + 6);
				end_lang_pos = line_tag.find_first_of('"');
				if (end_lang_pos != std::string::npos) {
					content_language_str = line_tag.substr(0, end_lang_pos);
					if (!RequestParsing::is_valid_language_tag(content_language_str))
						content_language_str.clear();
				}
			}
		}
	}
	return content_language_str;
}

std::string
ResponseHandling::_xml_content_language_parser(const Response& response) {
	std::string content_language_str, line_tag;
	std::string body(response.get_body().c_str(), response.get_body().size());
	size_t lang_pos, end_lang_pos;

	lang_pos = body.find("xml:lang=\"");
	if (lang_pos != std::string::npos) {
		line_tag = body.substr(lang_pos + 10);
		end_lang_pos = line_tag.find_first_of('"');
		if (end_lang_pos != std::string::npos) {
			content_language_str = line_tag.substr(0, end_lang_pos);
			if (!RequestParsing::is_valid_language_tag(content_language_str))
				content_language_str.clear();
		}
	}
	return content_language_str;
}

bool
ResponseHandling::_is_accepted_language(const std::string& language_found, const std::list<std::string>& allowed_languages) {
	if (allowed_languages.empty())
		return true;
	for(std::list<std::string>::const_iterator it = allowed_languages.begin(); it != allowed_languages.end(); it++) {
		if (*it == "*")
			return true;
		if (language_found.find(*it) != std::string::npos)
			return true;
	}
	return false;
}

int
ResponseHandling::_response_content_language_handler(Client::exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	std::string language;
	const std::string& content_type = response.get_content_type();

	if (content_type == Syntax::mime_types_tab[TEXT_HTML].name)
		language = _html_content_language_parser(response);
	else if (content_type == Syntax::mime_types_tab[APPLICATION_XML].name)
		language = _xml_content_language_parser(response);
	if (!language.empty()) {
		if(request.get_headers().key_exists(ACCEPT_LANGUAGE) &&
		   !_is_accepted_language(language, request.get_headers().get_value(ACCEPT_LANGUAGE))) {
			response.get_status_line().set_status_code(NOT_ACCEPTABLE);
			return FAILURE;
		}
		response.get_headers().insert(CONTENT_LANGUAGE, language);
	}
	return (SUCCESS);
}

int
ResponseHandling::_response_content_length_handler(Client::exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	struct stat buf;
	std::stringstream ss;

	if (request.get_request_line().get_method() == PUT) {
		ss << response.get_body().size();
		response.get_headers().insert(CONTENT_LENGTH, ss.str());
		return (SUCCESS);
	}
	if (stat(response.get_target_path().c_str(), &buf) != -1) {
		if (buf.st_size > static_cast<long>(request.get_location()->get_client_max_body_size())) {
			response.get_status_line().set_status_code(PAYLOAD_TOO_LARGE);
			return FAILURE;
		}
		ss << buf.st_size;
		response.get_headers().insert(CONTENT_LENGTH, ss.str());
	}
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

bool
ResponseHandling::_is_accepted_charset(const std::string& charset_found, const std::list<std::string>& allowed_charsets) {
	if (allowed_charsets.empty())
		return true;
	for (std::list<std::string>::const_iterator it = allowed_charsets.begin();
		it != allowed_charsets.end(); it++) {
		if (*it == "*")
			return true;
		if (charset_found.find(Syntax::str_to_lower(*it)) != std::string::npos)
			return true;
	}
	return false;
}

std::string
ResponseHandling::_html_charset_parser(const Response& response) {
	std::string body(response.get_body().c_str(), response.get_body().size());
	std::string header_bloc, charset, meta_tag("<meta charset=\"") ;
	size_t header_bloc_begin, header_bloc_end, meta_bloc_begin, meta_bloc_end;

	header_bloc_begin = body.find("<head");
	if (header_bloc_begin != std::string::npos) {
		header_bloc = body.substr(header_bloc_begin);
		header_bloc_end = header_bloc.find("</head>");
		if (header_bloc_end != std::string::npos) {
			header_bloc = header_bloc.substr(0, header_bloc_end);
			meta_bloc_begin = header_bloc.find(meta_tag);
			if (meta_bloc_begin != std::string::npos) {
				charset = header_bloc.substr(meta_bloc_begin + meta_tag.size());
				meta_bloc_end = charset.find_first_of('"');
				if (meta_bloc_end != std::string::npos) {
					charset = Syntax::str_to_lower(charset.substr(0, meta_bloc_end));
				}
			}
		}
	}
	return charset;
}

std::string
ResponseHandling::_xml_charset_parser(const Response& response) {
	std::string body(response.get_body().c_str(), response.get_body().size());
	std::string xml_tag, encoding, charset;
	size_t xml_tag_begin, xml_tag_end, encoding_begin, encoding_end;

	xml_tag_begin = body.find("<?xml");
	if (xml_tag_begin != std::string::npos) {
		xml_tag = body.substr(xml_tag_begin);
		xml_tag_end = xml_tag.find("?>");
		if (xml_tag_end != std::string::npos) {
			xml_tag = xml_tag.substr(0, xml_tag_end);
			encoding_begin = xml_tag.find("encoding=\"");
			if (encoding_begin != std::string::npos) {
				encoding = xml_tag.substr(encoding_begin + 10);
				encoding_end = encoding.find_first_of('"');
				if (encoding_end != std::string::npos) {
					charset = Syntax::str_to_lower(encoding.substr(0, encoding_end));
				}
			}
		}
	}
	return charset;
}

int
ResponseHandling::_response_content_type_handler(Client::exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	std::string charset, content_type;

	content_type = response.get_content_type();
	if (content_type == Syntax::mime_types_tab[TEXT_HTML].name)
		charset = _html_charset_parser(response);
	else if (content_type == Syntax::mime_types_tab[APPLICATION_XML].name)
		charset = _xml_charset_parser(response);
	if (!charset.empty()) {
		if (request.get_headers().key_exists(ACCEPT_CHARSET) &&
			!_is_accepted_charset(charset, request.get_headers().get_value(ACCEPT_CHARSET))) {
			response.get_status_line().set_status_code(NOT_ACCEPTABLE);
			return FAILURE;
		}
		content_type += "; charset=" + charset;
	}
	response.get_headers().insert(CONTENT_TYPE, content_type);
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
