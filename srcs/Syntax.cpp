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

const Syntax::accepted_charsets_entry_t Syntax::charsets_tab[] =
{
	{UTF_8, "utf-8"},
	{ISO_8859_1, "iso-8859-1"},
	{UNICODE_1_1, "unicode-1-1"}
};

const Syntax::accepted_languages_entry_t Syntax::languages_tab[] =
{
	{FR, "fr"},
	{FR_BE, "fr-BE"},
	{FR_CA, "fr-CA"},
	{FR_CH, "fr-CH"},
	{FR_FR, "fr-FR"},
	{FR_LU, "fr-LU"},
	{EN, "en"},
	{EN_CA, "en-CA"},
	{EN_GB, "en-GB"},
	{EN_US, "en-US"}
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

std::string Syntax::trim_comments(const std::string &line_buffer) {
	std::string new_line;
	size_t hash_char_pos;

	new_line = line_buffer;
	hash_char_pos = new_line.find('#');
	if (hash_char_pos == std::string::npos)
		return new_line;
	return new_line.substr(0, hash_char_pos);
}

std::string Syntax::trim_whitespaces(const std::string& line_buffer) {
	std::string whitespaces;
	std::string new_line;
	size_t start, end;

	whitespaces = " \n\r\t\f\v";
	new_line = line_buffer;
	start = new_line.find_first_not_of(whitespaces);
	if (start != std::string::npos)
		new_line = new_line.substr(start);
	end = new_line.find_last_not_of(whitespaces);
	if (end != std::string::npos)
		new_line = new_line.substr(0, end + 1);
	return new_line;
}

std::vector<std::string> Syntax::split_whitespaces(const std::string& line_buffer) {
	return Syntax::split(line_buffer, " \n\r\t\f\v");
}

std::vector<std::string> Syntax::split(const std::string& line_buffer, const std::string& charset) {
	std::vector<std::string> result;
	std::string token;
	size_t ws_pos, progress_pos = 0, wd_len;;

	while(progress_pos < line_buffer.size()) {
		ws_pos = line_buffer.find_first_of(charset, progress_pos);
		if (ws_pos == std::string::npos)
			break;
		wd_len = ws_pos - progress_pos;
		token = line_buffer.substr(progress_pos, wd_len);
		progress_pos += wd_len + 1;
		if(!token.empty())
			result.push_back(token);
	}
	if (progress_pos < line_buffer.size())
		result.push_back(line_buffer.substr(progress_pos));
	return result;
}

int Syntax::trim_semicolon(std::vector<std::string>& tokens) {
	std::string last_token = tokens.back();
	std::string::iterator ite = tokens.back().end();
	char last_char = *(--ite);

	if (last_token == ";") {
		tokens.pop_back();
		return 1;
	}
	if (last_char != ';')
		return 0;
	tokens.back().erase(ite);
	return 1;
}

bool Syntax::is_num(const char* str) {
	for(int i = 0; str[i]; i++) {
		if (!std::isdigit(str[i]))
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
		if (!is_num(nums[i].c_str()))
			return 0;
		num = std::strtol(nums[i].c_str(), NULL, 10);
		if (num < 0 || num > 255)
			return 0;
	}
	return 1;
}

std::multimap<float, std::string> Syntax::split_weight(const std::vector<std::string>& elements_split) {
	std::multimap<float, std::string> value_weight;
	std::vector<std::string> weight_split;
	std::string effective_value;
	std::string q_weight;
	float effective_weight;

	for(std::vector<std::string>::const_iterator it = elements_split.begin();
		it != elements_split.end(); it++) {
		weight_split = Syntax::split(*it, ";");
		if (weight_split.size() > 2) {
			value_weight.clear();
			return value_weight;
		}
		if (!weight_split.empty() && weight_split.size() != 1) {
			effective_value = weight_split[0];
			q_weight = weight_split[1];
			if (q_weight.compare(0, 2, "q=") != 0)	{
				value_weight.clear();
				return value_weight;
			}
			effective_weight = std::strtol(q_weight.substr(2).c_str(), NULL, 10);
		}
		else {
			effective_value = *it;
			effective_weight = 1;
		}
		value_weight.insert(std::make_pair(effective_weight, effective_value));
	}
	return value_weight;
}

std::list<std::string> Syntax::parse_header_value(const std::string& unparsed_value) {
	std::list<std::string> new_list;
	std::multimap<float, std::string> value_weight;
	std::vector<std::string> elements_split;

	elements_split = Syntax::split(unparsed_value, ", ");
	if (!elements_split.empty()) {
		value_weight = split_weight(elements_split);
		if (value_weight.empty()) {
			return new_list;
		}
	}
	for(std::multimap<float, std::string>::const_reverse_iterator rit = value_weight.rbegin();
		rit != value_weight.rend(); rit++)
		new_list.push_back(rit->second);
	return new_list;
}
