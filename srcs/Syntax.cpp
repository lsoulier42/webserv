/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Syntax.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 21:57:25 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/14 23:23:55 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Syntax.hpp"

const Syntax::method_tab_entry_t
Syntax::method_tab[] = {
	{GET, "GET"},
	{HEAD, "HEAD"},
	{POST, "POST"},
	{PUT, "PUT"},
	{DELETE, "DELETE"},
	{CONNECT, "CONNECT"},
	{OPTIONS, "OPTIONS"},
	{TRACE, "TRACE"},
};

const Syntax::status_code_tab_entry_t
Syntax::status_codes_tab[] = {
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
	{HTTP_VERSION_NOT_SUPPORTED, 505, "505", "HTTP Version Not Supported"},
	{TOTAL_STATUS_CODE, 0, "0", ""}
};

const Syntax::server_instruction_tab_entry_t
Syntax::server_instructions_tab[] = {
	{LISTEN, "listen"},
	{SERVER_NAME, "server_name"},
	{ERROR_PAGE, "error_page"},
	{CLIENT_MAX_BODY_SIZE, "client_max_body_size"},
	{UPLOAD_DIR, "upload_dir"},
	{LOCATION_INSTRUCTION, "location"}
};

const Syntax::location_instruction_tab_entry_t
Syntax::location_instructions_tab[] = {
	{ROOT, "root"},
	{METHODS, "methods"},
	{INDEX, "index"},
	{CGI, "cgi"},
	{AUTOINDEX, "autoindex"}
};

const Syntax::header_tab_entry_t
Syntax::headers_tab[] = {
	{ACCEPT_CHARSET, "Accept-Charset"},
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
	{WWW_AUTHENTICATE, "WWW-Authenticate"},
	{TRAILER, "Trailer"}
};

const Syntax::header_tab_entry_t
Syntax::request_headers_tab[] = {
	headers_tab[ACCEPT_CHARSET],
	headers_tab[ACCEPT_LANGUAGE],
	headers_tab[AUTHORIZATION],
	headers_tab[CONTENT_LENGTH],
	headers_tab[CONTENT_LANGUAGE],
	headers_tab[DATE],
	headers_tab[HOST],
	headers_tab[REFERER],
	headers_tab[TRANSFER_ENCODING],
	headers_tab[USER_AGENT]
};

const Syntax::header_tab_entry_t
Syntax::response_headers_tab[] = {
		headers_tab[ALLOW],
		headers_tab[CONTENT_LANGUAGE],
		headers_tab[CONTENT_LENGTH],
		headers_tab[CONTENT_LOCATION],
		headers_tab[CONTENT_TYPE],
		headers_tab[DATE],
		headers_tab[LAST_MODIFIED],
		headers_tab[LOCATION],
		headers_tab[RETRY_AFTER],
		headers_tab[SERVER],
		headers_tab[TRANSFER_ENCODING],
		headers_tab[WWW_AUTHENTICATE]
};


const Syntax::accepted_charsets_entry_t
Syntax::charsets_tab[] = {
	{UTF_8, "utf-8"},
	{ISO_8859_1, "iso-8859-1"},
	{UNICODE_1_1, "unicode-1-1"},
	{US_ASCII, "US-ASCII"}
};

const Syntax::mime_type_entry_t
Syntax::mime_types_tab[] = {
	{AUDIO_AAC, ".aac", "audio/aac"},
	{APPLICATION_X_ABIWORD, ".abw", "application/x-abiword"},
	{APPLICATION_OCTET_STREAM_ARC, ".arc", "application/octet-stream"},
	{VIDEO_X_MSVIDEO, ".avi", "video/x-msvideo"},
	{APPLICATION_VND_AMAZON_EBOOK, ".azw", "application/vnd.amazon.ebook"},
	{APPLICATION_OCTET_STREAM, ".bin", "application/octet-stream"},
	{IMAGE_BMP, ".bmp", "image/bmp"},
	{APPLICATION_X_BZIP, ".bz", "application/x-bzip"},
	{APPLICATION_X_BZIP2, ".bz2", "application/x-bzip2"},
	{APPLICATION_X_CSH, ".csh", "application/x-csh"},
	{TEXT_CSS, ".css", "text/css"},
	{TEXT_CSV, ".csv", "text/csv"},
	{TEXT_PLAIN, ".txt", "text/plain"},
	{APPLICATION_MSWORD, ".doc", "application/msword"},
	{APPLICATION_VND_OPENXMLFORMATS_OFFICEDOCUMENT_WORDPROCESSINGML_DOCUMENT, ".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
	{APPLICATION_VND_MS_FONTOBJECT, ".eot", "application/vnd.ms-fontobject"},
	{APPLICATION_EPUB_ZIP, ".epub", "application/epub+zip"},
	{IMAGE_GIF, ".gif", "image/gif"},
	{TEXT_HTM, ".htm", "text/html"},
	{TEXT_HTML, ".html", "text/html"},
	{IMAGE_X_ICON, ".ico", "image/x-icon"},
	{TEXT_CALENDAR, ".ics", "text/calendar"},
	{APPLICATION_JAVA_ARCHIVE, ".jar", "application/java-archive"},
	{IMAGE_JPEG, ".jpeg", "image/jpeg"},
	{IMAGE_JPG, ".jpg", "image/jpeg"},
	{APPLICATION_JAVASCRIPT, ".js", "application/javascript"},
	{APPLICATION_JSON, ".json", "application/json"},
	{AUDIO_MID, ".mid", "audio/midi"},
	{AUDIO_MIDI, ".midi", "audio/midi"},
	{VIDEO_MPEG, ".mpeg", "video/mpeg"},
	{APPLICATION_VND_APPLE_INSTALLER_XML, ".mpkg", "application/vnd.apple.installer+xml"},
	{APPLICATION_VND_OASIS_OPENDOCUMENT_PRESENTATION, ".odp", "application/vnd.oasis.opendocument.presentation"},
	{APPLICATION_VND_OASIS_OPENDOCUMENT_SPREADSHEET, ".ods", "application/vnd.oasis.opendocument.spreadsheet"},
	{APPLICATION_VND_OASIS_OPENDOCUMENT_TEXT, ".odt", "application/vnd.oasis.opendocument.text"},
	{AUDIO_OGG, ".oga", "audio/ogg"},
	{VIDEO_OGG, ".ogv", "video/ogg"},
	{APPLICATION_OGG, ".ogx", "application/ogg"},
	{FONT_OTF, ".otf", "font/otf"},
	{IMAGE_PNG, ".png", "image/png"},
	{APPLICATION_PDF, ".pdf", "application/pdf"},
	{APPLICATION_VND_MS_POWERPOINT, ".ppt", "application/vnd.ms-powerpoint"},
	{APPLICATION_VND_OPENXMLFORMATS_OFFICEDOCUMENT_PRESENTATIONML_PRESENTATION, ".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
	{APPLICATION_X_RAR_COMPRESSED, ".rar", "application/x-rar-compressed"},
	{APPLICATION_RTF, ".rtf", "application/rtf"},
	{APPLICATION_X_SH, ".sh", "application/x-sh"},
	{IMAGE_SVG_XML, ".svg", "image/svg+xml"},
	{APPLICATION_X_SHOCKWAVE_FLASH, ".swf", "application/x-shockwave-flash"},
	{APPLICATION_X_TAR, ".tar", "application/x-tar"},
	{IMAGE_TIF, ".tif", "image/tiff"},
	{IMAGE_TIFF, ".tiff", "image/tiff"},
	{APPLICATION_TYPESCRIPT, ".ts", "application/typescript"},
	{FONT_TTF, ".ttf", "font/ttf"},
	{APPLICATION_VND_VISIO, ".vsd", "application/vnd.visio"},
	{AUDIO_X_WAV, ".wav", "audio/x-wav"},
	{AUDIO_WEBM, ".weba", "audio/webm"},
	{VIDEO_WEBM, ".webm", "video/webm"},
	{IMAGE_WEBP, ".webp", "image/webp"},
	{FONT_WOFF, ".woff", "font/woff"},
	{FONT_WOFF2, ".woff2", "font/woff2"},
	{APPLICATION_XHTML_XML, ".xhtml", "application/xhtml+xml"},
	{APPLICATION_VND_MS_EXCEL, ".xls", "application/vnd.ms-excel"},
	{APPLICATION_VND_OPENXMLFORMATS_OFFICEDOCUMENT_SPREADSHEETML_SHEET, ".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
	{APPLICATION_XML, ".xml", "application/xml"},
	{APPLICATION_VND_MOZILLA_XUL_XML, ".xul", "application/vnd.mozilla.xul+xml"},
	{APPLICATION_ZIP, ".zip", "application/zip"},
	{VIDEO_3GPP, ".3gp", "video/3gpp"},
	{VIDEO_3GPP2, ".3g2", "video/3gpp2"},
	{APPLICATION_X_7Z_COMPRESSED, ".7z", "application/x-7z-compressed"}
};

const Syntax::encoding_type_entry_t
Syntax::encoding_types_tab[] = {
	{CHUNKED, "chunked"},
	{COMPRESS, "compress"},
	{DEFLATE, "deflate"},
	{GZIP, "gzip"},
	{IDENTITY, "identity"}
};

bool
Syntax::is_informational_code(int code) {
	return code == 100 || code == 101;
}

bool
Syntax::is_successful_code(int code) {
	return code >= 200 && code <= 206;
}

bool
Syntax::is_redirection_code(int code) {
	return (code >= 300 && code <= 305) || code == 307;
}

bool
Syntax::is_client_error_code(int code) {
	return (code >= 400 && code <= 417) || code == 426;
}

bool
Syntax::is_server_error_code(int code) {
	return code >= 500 && code <= 505;
}

bool
Syntax::is_error_code(int code) {
	return is_server_error_code(code) || is_client_error_code(code);
}

int
Syntax::fetch_method_index(const std::string& method) {
	for(int i = 0; i < DEFAULT_METHOD; i++) {
		if (method == Syntax::method_tab[i].name)
			return i;
	}
	return -1;
}

bool
Syntax::is_implemented_header(const std::string &header_name) {
	for(int i = 0; i < TOTAL_HEADER_NAMES; i++) {
		if (header_name == Syntax::headers_tab[i].name)
			return true;
	}
	return false;
}

std::string
Syntax::trim_comments(const std::string &str) {
	std::string new_line;
	size_t hash_char_pos;

	new_line = str;
	hash_char_pos = new_line.find('#');
	if (hash_char_pos == std::string::npos)
		return new_line;
	return new_line.substr(0, hash_char_pos);
}

std::string
Syntax::trim_whitespaces(const std::string& str) {
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

std::vector<std::string>
Syntax::split(const std::string& str, const std::string& charset) {
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

int
Syntax::trim_semicolon(std::vector<std::string>& str) {
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

bool
Syntax::str_is_num(const std::string& str) {
	for(size_t i = 0; str[i]; i++) {
		if (!std::isdigit(str[i]))
			return false;
	}
	return true;
}

bool
Syntax::str_is_alpha(const std::string& str) {
	for(size_t i = 0; str[i]; i++) {
		if (!std::isalpha(str[i]))
			return false;
	}
	return true;
}

bool
Syntax::str_is_alnum(const std::string& str) {
	for(size_t i = 0; str[i]; i++) {
		if (!std::isalpha(str[i]) && !std::isdigit(str[i]))
			return false;
	}
	return true;
}

int
Syntax::check_ip_format(const std::string& ip) {
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

URI_form_t
Syntax::get_URI_form(const std::string& uri_str) {
	if (uri_str.find_first_of(WHITESPACES) != std::string::npos)
		return INVALID_URI_FORM;
	if (uri_str == "*")
		return ASTERISK_FORM;
	if (uri_str.find_first_of('@') != std::string::npos)
		return AUTHORITY_FORM;
	if (uri_str.find("http://") != std::string::npos
		|| uri_str.find("https://") != std::string::npos)
		return ABSOLUTE_URI;
	return PARTIAL_URI;
}

bool
Syntax::is_valid_path(const std::string &path) {
	struct stat	buf;
	return stat(path.c_str(), &buf) != -1;
}

std::string
Syntax::str_to_lower(const std::string& str) {
	std::string return_str(str);
	size_t size = return_str.size();

	for(size_t i = 0; i < size; i++)
		return_str[i] = tolower(return_str[i]);
	return return_str;
}
