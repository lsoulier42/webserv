/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Syntax.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 21:57:25 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/13 03:17:10 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Syntax.hpp"

const Syntax::method_tab_entry_t Syntax::method_tab[] =
{
	{GET, "GET"},
	{HEAD, "HEAD"},
	{POST, "POST"},
	{PUT, "PUT"},
	{DELETE, "DELETE"},
	{CONNECT, "CONNECT"},
	{OPTIONS, "OPTIONS"},
	{TRACE, "TRACE"},
};

const Syntax::status_code_tab_entry_t Syntax::status_codes_tab[] =
{
	{CONTINUE, 100, "100", "Continue"},
	{SWITCHING_PROTOCOLS, 101, "101", "Switching Protocols"},
	{OK, 200, "200", "OK"},
	{CREATED, 201, "201", "Created"},
	{ACCEPTED, 202, "202", "Accepted"},
	{NON_AUTHORITATIVE_INFORMATION, 203, "203", "Non-Authoritative Information"},
	{NO_CONTENT, 204, "204", "No Content"},
	{RESET_CONTENT, 205, "205", "Reset Content"},
	{PARTIAL_CONTENT, 206, "206", "Partial Content"},
	{MULTIPLE_CHOICES, 300, "300", "Multiple Choices"},
	{MOVED_PERMANENTLY, 301, "301", "Moved Permanently"},
	{FOUND, 302, "302", "Found"},
	{SEE_OTHER, 303, "303", "See Other"},
	{NOT_MODIFIED, 304, "304", "Not Modified"},
	{USE_PROXY, 305, "305", "Use Proxy"},
	{TEMPORARY_REDIRECT, 307, "307", "Temporary Redirect"},
	{BAD_REQUEST, 400, "400", "Bad Request"},
	{UNAUTHORIZED, 401, "401", "Unauthorized"},
	{PAYMENT_REQUIRED, 402, "402", "Payment Required"},
	{FORBIDDEN, 403, "403", "Forbidden"},
	{NOT_FOUND, 404, "404", "Not Found"},
	{METHOD_NOT_ALLOWED, 405, "405", "Method Not Allowed"},
	{NOT_ACCEPTABLE, 406, "406", "Not Acceptable"},
	{PROXY_AUTHENTICATION_REQUIRED, 407, "407", "Proxy Authentication Required"},
	{REQUEST_TIMEOUT, 408, "408", "Request Time-out"},
	{CONFLICT, 409, "409", "Conflict"},
	{GONE, 410, "410", "Gone"},
	{LENGTH_REQUIRED, 411, "411", "Length Required"},
	{PRECONDITION_FAILED, 412, "412", "Precondition Failed"},
	{PAYLOAD_TOO_LARGE, 413, "413", "Payload Too Large"},
	{URI_TOO_LONG, 414, "414", "URI Too Long"},
	{UNSUPPORTED_MEDIA_TYPE, 415, "415", "Unsupported Media Type"},
	{RANGE_NOT_SATISFIABLE, 416, "416", "Range Not Satisfiable"},
	{EXPECTATION_FAILED, 417, "417", "Expectation Failed"},
	{UPGRADE_REQUIRED, 426, "426", "Upgrade Required"},
	{INTERNAL_SERVER_ERROR, 500, "500", "Internal Server Error"},
	{NOT_IMPLEMENTED, 501, "501", "Not Implemented"},
	{BAD_GATEWAY, 502, "502", "Bad Gateway"},
	{SERVICE_UNAVAILABLE, 503, "503", "Service Unavailable"},
	{GATEWAY_TIMEOUT, 504, "504", "Gateway Timeout"},
	{HTTP_VERSION_NOT_SUPPORTED, 505, "505", "HTTP Version Not Supported"}
};

const Syntax::instruction_tab_entry_t Syntax::server_instructions_tab[] =
{
	{LISTEN, "listen"},
	{SERVER_NAME, "server_name"},
	{ERROR_PAGE, "error_page"},
	{CLIENT_MAX_BODY_SIZE, "client_max_body_size"},
	{LOCATION_INSTRUCTION, "location"},
	{METHODS, "methods"},
	{ROOT, "root"},
	{AUTOINDEX, "autoindex"},
	{INDEX, "index"},
	{UPLOAD_DIR, "upload_dir"},
	{CGI, "cgi"}
};

const Syntax::instruction_tab_entry_t Syntax::location_instructions_tab[] =
{
	server_instructions_tab[METHODS],
	server_instructions_tab[ROOT],
	server_instructions_tab[AUTOINDEX],
	server_instructions_tab[INDEX],
	server_instructions_tab[CGI]
};

const Syntax::header_tab_entry_t Syntax::headers_tab[] =
{
	{ACCEPT_CHARSETS, "Accept-Charsets"},
	{ACCEPT_LANGUAGE, "Accept-Language"},
	{ALLOW, "Allow"},
	{AUTHORIZATION, "Authorization"},
	{CONTENT_LANGUAGE, "Content-Language"},
	{CONTENT_LENGTH, "Content-Length"},
	{CONTENT_LOCATION, "Content-Location"},
	{CONTENT_TYPE, "Content-Type"},
	{DATE, "Date"},
	{HOST, "Host"},
	{LAST_MODIFIED, "Last-Modified"},
	{LOCATION, "Location"},
	{REFERER, "Referer"},
	{RETRY_AFTER, "Retry-After"},
	{SERVER, "Server"},
	{TRANSFER_ENCODING, "Transfer-Encoding"},
	{USER_AGENT, "User-Agent"},
	{WWW_AUTHENTICATE, "WWW-Authenticate"}
};

const Syntax::accepted_charsets_entry_t Syntax::charsets_tab[] =
{
	{UTF_8, "utf-8"},
	{ISO_8859_1, "iso-8859-1"},
	{UNICODE_1_1, "unicode-1-1"},
	{US_ASCII, "US-ASCII"}
};

const Syntax::mime_type_entry_t Syntax::mime_types_tab[] =
{
	{TEXT_PLAIN, "text/plain"},
	{TEXT_HTML, "text/html"},
	{TEXT_CSS, "text/css"},
	{TEXT_CSV, "text/csv"},
	{IMAGE_BMP, "image/bmp"},
	{IMAGE_GIF, "image/gif"},
	{IMAGE_JPEG, "image/jpeg"},
	{IMAGE_PNG, "image/png"},
	{APPLICATION_OCTET_STREAM, "application/octet-stream"},
	{APPLICATION_JAVASCRIPT, "application/javascript"},
	{APPLICATION_PDF, "application/pdf"},
	{APPLICATION_XML, "application/xml"}
};

const Syntax::encoding_type_entry_t Syntax::encoding_types_tab[] =
{
	{CHUNKED, "chunked"},
	{COMPRESS, "compress"},
	{DEFLATE, "deflate"},
	{GZIP, "gzip"},
	{IDENTITY, "identity"}
};

bool Syntax::is_informational_code(int code) {
	return code == 100 || code == 101;
}

bool Syntax::is_successful_code(int code) {
	return code >= 200 && code <= 206;
}

bool Syntax::is_redirection_code(int code) {
	return (code >= 300 && code <= 305) || code == 307;
}

bool Syntax::is_client_error_code(int code) {
	return (code >= 400 && code <= 417) || code == 426;
}

bool Syntax::is_server_error_code(int code) {
	return code >= 500 && code <= 505;
}

bool Syntax::is_error_code(int code) {
	return is_server_error_code(code) || is_client_error_code(code);
}

int Syntax::fetch_method_index(const std::string& method) {
	for(int i = 0; i < DEFAULT_METHOD; i++) {
		if (method == Syntax::method_tab[i].name)
			return i;
	}
	return -1;
}

bool Syntax::is_implemented_header(const std::string &header_name) {
	for(int i = 0; i < TOTAL_HEADER_NAMES; i++) {
		if (header_name == Syntax::headers_tab[i].name)
			return true;
	}
	return false;
}

std::string Syntax::trim_comments(const std::string &str) {
	std::string new_line;
	size_t hash_char_pos;

	new_line = str;
	hash_char_pos = new_line.find('#');
	if (hash_char_pos == std::string::npos)
		return new_line;
	return new_line.substr(0, hash_char_pos);
}

std::string Syntax::trim_whitespaces(const std::string& str) {
	std::string whitespaces;
	std::string new_line;
	size_t start, end;

	whitespaces = WHITESPACES;
	new_line = str;
	start = new_line.find_first_not_of(whitespaces);
	if (start != std::string::npos)
		new_line = new_line.substr(start);
	end = new_line.find_last_not_of(whitespaces);
	if (end != std::string::npos)
		new_line = new_line.substr(0, end + 1);
	return new_line;
}

std::vector<std::string> Syntax::split(const std::string& str, const std::string& charset) {
	std::vector<std::string> result;
	std::string token;
	size_t ws_pos, progress_pos = 0, wd_len;;

	while(progress_pos < str.size()) {
		ws_pos = str.find_first_of(charset, progress_pos);
		if (ws_pos == std::string::npos)
			break;
		wd_len = ws_pos - progress_pos;
		token = str.substr(progress_pos, wd_len);
		progress_pos += wd_len + 1;
		if(!token.empty())
			result.push_back(token);
	}
	if (progress_pos < str.size())
		result.push_back(str.substr(progress_pos));
	return result;
}

int Syntax::trim_semicolon(std::vector<std::string>& str) {
	std::string last_token = str.back();
	std::string::iterator ite = str.back().end();
	char last_char = *(--ite);

	if (last_token == ";") {
		str.pop_back();
		return 1;
	}
	if (last_char != ';')
		return 0;
	str.back().erase(ite);
	return 1;
}

bool Syntax::str_is_num(const std::string& str) {
	for(size_t i = 0; str[i]; i++) {
		if (!std::isdigit(str[i]))
			return false;
	}
	return true;
}

bool Syntax::str_is_alpha(const std::string& str) {
	for(size_t i = 0; str[i]; i++) {
		if (!std::isalpha(str[i]))
			return false;
	}
	return true;
}

bool Syntax::str_is_alnum(const std::string& str) {
	for(size_t i = 0; str[i]; i++) {
		if (!std::isalpha(str[i]) && !std::isdigit(str[i]))
			return false;
	}
	return true;
}

int Syntax::check_ip_format(const std::string& ip) {
	std::vector<std::string> nums = Syntax::split(ip, ".");
	int num;

	if (nums.size() != 4)
		return 0;
	for(int i = 0; i < 4; i++) {
		if (!str_is_num(nums[i]))
			return 0;
		num = std::strtol(nums[i].c_str(), NULL, 10);
		if (num < 0 || num > 255)
			return 0;
	}
	return 1;
}

URI_form_t Syntax::get_URI_form(const std::string& uri_str) {
	if (uri_str.find_first_of(WHITESPACES) != std::string::npos)
		return INVALID_URI_FORM;
	if (uri_str == "*")
		return ASTERISK_FORM;
	if (uri_str.find_first_of("@") != std::string::npos)
		return AUTHORITY_FORM;
	if (uri_str.find("http://") != std::string::npos
		|| uri_str.find("https://") != std::string::npos)
		return ABSOLUTE_URI;
	return PARTIAL_URI;
}
