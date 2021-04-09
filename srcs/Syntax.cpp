/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Syntax.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 21:57:25 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/09 21:57:31 by lsoulier         ###   ########.fr       */
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
	{CONTINUE, 100, "Continue"},
	{SWITCHING_PROTOCOLS, 101, "Switching Protocols"},
	{OK, 200, "OK"},
	{CREATED, 201, "Created"},
	{ACCEPTED, 202, "Accepted"},
	{NON_AUTHORITATIVE_INFORMATION, 203, "Non-Authoritative Information"},
	{NO_CONTENT, 204, "No Content"},
	{RESET_CONTENT, 205, "Reset Content"},
	{PARTIAL_CONTENT, 206, "Partial Content"},
	{MULTIPLE_CHOICES, 300, "Multiple Choices"},
	{MOVED_PERMANENTLY, 301, "Moved Permanently"},
	{FOUND, 302, "Found"},
	{SEE_OTHER, 303, "See Other"},
	{NOT_MODIFIED, 304, "Not Modified"},
	{USE_PROXY, 305, "Use Proxy"},
	{TEMPORARY_REDIRECT, 307, "Temporary Redirect"},
	{BAD_REQUEST, 400, "Bad Request"},
	{UNAUTHORIZED, 401, "Unauthorized"},
	{PAYMENT_REQUIRED, 402, "Payment Required"},
	{FORBIDDEN, 403, "Forbidden"},
	{NOT_FOUND, 404, "Not Found"},
	{METHOD_NOT_ALLOWED, 405, "Method Not Allowed"},
	{NOT_ACCEPTABLE, 406, "Not Acceptable"},
	{PROXY_AUTHENTICATION_REQUIRED, 407, "Proxy Authentication Required"},
	{REQUEST_TIMEOUT, 408, "Request Time-out"},
	{CONFLICT, 409, "Conflict"},
	{GONE, 410, "Gone"},
	{LENGTH_REQUIRED, 411, "Length Required"},
	{PRECONDITION_FAILED, 412, "Precondition Failed"},
	{PAYLOAD_TOO_LARGE, 413, "Payload Too Large"},
	{URI_TOO_LONG, 414, "URI Too Long"},
	{UNSUPPORTED_MEDIA_TYPE, 415, "Unsupported Media Type"},
	{RANGE_NOT_SATISFIABLE, 416, "Range Not Satisfiable"},
	{EXPECTATION_FAILED, 417, "Expectation Failed"},
	{UPGRADE_REQUIRED, 426, "Upgrade Required"},
	{INTERNAL_SERVER_ERROR, 500, "Internal Server Error"},
	{NOT_IMPLEMENTED, 501, "Not Implemented"},
	{BAD_GATEWAY, 502, "Bad Gateway"},
	{SERVICE_UNAVAILABLE, 503, "Service Unavailable"},
	{GATEWAY_TIMEOUT, 504, "Gateway Timeout"},
	{HTTP_VERSION_NOT_SUPPORTED, 505, "HTTP Version Not Supported"}
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