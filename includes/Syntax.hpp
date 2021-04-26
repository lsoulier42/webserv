/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Syntax.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 21:57:42 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/26 09:55:22 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SYNTAX_HPP
# define SYNTAX_HPP
# include <iostream>
# include <string>
# include <sstream>
# include <cstring>
# include <list>
# include <vector>
# include <map>
# include <cstdlib>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include "ByteArray.hpp"

# define WHITESPACES " \n\r\t\f\v"
# define TOTAL_REQUEST_HEADERS 10
# define TOTAL_RESPONSE_HEADERS 12
# define SUCCESS 1
# define FAILURE 0
# define DELAY_RETRY_AFTER 10
# define PROGRAM_VERSION "webserv/1.0"
# define OUR_HTTP_VERSION "HTTP/1.1"

enum path_type_t {
	REGULAR_FILE,
	DIRECTORY,
	INVALID_PATH
};

enum method_t {
	GET,
	HEAD,
	POST,
	PUT,
	DELETE,
	CONNECT,
	OPTIONS,
	TRACE,
	DEFAULT_METHOD
};

enum server_instruction_t {
	LISTEN,
	SERVER_NAME,
	ERROR_PAGE,
	LOCATION_INSTRUCTION,
	TOTAL_SERVER_INSTRUCTIONS
};

enum location_instruction_t {
	ROOT,
	METHODS,
	INDEX,
	CGI,
	AUTOINDEX,
	CLIENT_MAX_BODY_SIZE,
	UPLOAD_DIR,
	TOTAL_LOCATION_INSTRUCTIONS
};

enum status_code_t {
	CONTINUE,
	SWITCHING_PROTOCOLS,
	OK,
	CREATED,
	ACCEPTED,
	NON_AUTHORITATIVE_INFORMATION,
	NO_CONTENT,
	RESET_CONTENT,
	PARTIAL_CONTENT,
	MULTIPLE_CHOICES,
	MOVED_PERMANENTLY,
	FOUND,
	SEE_OTHER,
	NOT_MODIFIED,
	USE_PROXY,
	TEMPORARY_REDIRECT,
	BAD_REQUEST,
	UNAUTHORIZED,
	PAYMENT_REQUIRED,
	FORBIDDEN,
	NOT_FOUND,
	METHOD_NOT_ALLOWED,
	NOT_ACCEPTABLE,
	PROXY_AUTHENTICATION_REQUIRED,
	REQUEST_TIMEOUT,
	CONFLICT,
	GONE,
	LENGTH_REQUIRED,
	PRECONDITION_FAILED,
	PAYLOAD_TOO_LARGE,
	URI_TOO_LONG,
	UNSUPPORTED_MEDIA_TYPE,
	RANGE_NOT_SATISFIABLE,
	EXPECTATION_FAILED,
	UPGRADE_REQUIRED,
	INTERNAL_SERVER_ERROR,
	NOT_IMPLEMENTED,
	BAD_GATEWAY,
	SERVICE_UNAVAILABLE,
	GATEWAY_TIMEOUT,
	HTTP_VERSION_NOT_SUPPORTED,
	TOTAL_STATUS_CODE
};

enum header_name_t {
	ACCEPT_CHARSET,
	ACCEPT_LANGUAGE,
	ALLOW,
	AUTHORIZATION,
	CONTENT_LANGUAGE,
	CONTENT_LENGTH,
	CONTENT_LOCATION,
	CONTENT_TYPE,
	DATE,
	HOST,
	LAST_MODIFIED,
	LOCATION,
	REFERER,
	RETRY_AFTER,
	SERVER,
	TRANSFER_ENCODING,
	USER_AGENT,
	WWW_AUTHENTICATE,
	TRAILER,
	TOTAL_HEADER_NAMES
};

enum cgi_header_name_t {
	CGI_CONTENT_TYPE,
	CGI_CONTENT_LENGTH,
	CGI_LOCATION,
	CGI_STATUS
};

enum accepted_charsets_t {
	UTF_8,
	ISO_8859_1,
	UNICODE_1_1,
	US_ASCII,
	TOTAL_ACCEPTED_CHARSETS
};

enum mime_type_t {
	AUDIO_AAC,
	APPLICATION_X_ABIWORD,
	APPLICATION_OCTET_STREAM_ARC,
	VIDEO_X_MSVIDEO,
	APPLICATION_VND_AMAZON_EBOOK,
	APPLICATION_OCTET_STREAM,
	IMAGE_BMP,
	APPLICATION_X_BZIP,
	APPLICATION_X_BZIP2,
	APPLICATION_X_CSH,
	TEXT_CSS,
	TEXT_CSV,
	TEXT_PLAIN,
	APPLICATION_MSWORD,
	APPLICATION_VND_OPENXMLFORMATS_OFFICEDOCUMENT_WORDPROCESSINGML_DOCUMENT,
	APPLICATION_VND_MS_FONTOBJECT,
	APPLICATION_EPUB_ZIP,
	IMAGE_GIF,
	TEXT_HTM,
	TEXT_HTML,
	IMAGE_X_ICON,
	TEXT_CALENDAR,
	APPLICATION_JAVA_ARCHIVE,
	IMAGE_JPG,
	IMAGE_JPEG,
	APPLICATION_JAVASCRIPT,
	APPLICATION_JSON,
	AUDIO_MID,
	AUDIO_MIDI,
	VIDEO_MPEG,
	APPLICATION_VND_APPLE_INSTALLER_XML,
	APPLICATION_VND_OASIS_OPENDOCUMENT_PRESENTATION,
	APPLICATION_VND_OASIS_OPENDOCUMENT_SPREADSHEET,
	APPLICATION_VND_OASIS_OPENDOCUMENT_TEXT,
	AUDIO_OGG,
	VIDEO_OGG,
	APPLICATION_OGG,
	FONT_OTF,
	IMAGE_PNG,
	APPLICATION_PDF,
	APPLICATION_VND_MS_POWERPOINT,
	APPLICATION_VND_OPENXMLFORMATS_OFFICEDOCUMENT_PRESENTATIONML_PRESENTATION,
	APPLICATION_X_RAR_COMPRESSED,
	APPLICATION_RTF,
	APPLICATION_X_SH,
	IMAGE_SVG_XML,
	APPLICATION_X_SHOCKWAVE_FLASH,
	APPLICATION_X_TAR,
	IMAGE_TIF,
	IMAGE_TIFF,
	APPLICATION_TYPESCRIPT,
	FONT_TTF,
	APPLICATION_VND_VISIO,
	AUDIO_X_WAV,
	AUDIO_WEBM,
	VIDEO_WEBM,
	IMAGE_WEBP,
	FONT_WOFF,
	FONT_WOFF2,
	APPLICATION_XHTML_XML,
	APPLICATION_VND_MS_EXCEL,
	APPLICATION_VND_OPENXMLFORMATS_OFFICEDOCUMENT_SPREADSHEETML_SHEET,
	APPLICATION_XML,
	APPLICATION_VND_MOZILLA_XUL_XML,
	APPLICATION_ZIP,
	VIDEO_3GPP,
	VIDEO_3GPP2,
	APPLICATION_X_7Z_COMPRESSED,
	TOTAL_MIME_TYPES
};

enum URI_form_t {
	ABSOLUTE_URI,
	PARTIAL_URI,
	ASTERISK_FORM,
	AUTHORITY_FORM,
	INVALID_URI_FORM
};

enum encoding_type_t {
	CHUNKED,
	COMPRESS,
	DEFLATE,
	GZIP,
	IDENTITY,
	TOTAL_ENCODING_TYPES
};

class Syntax {
	public:
		~Syntax();

		struct method_tab_entry_t {
			method_t		method_index;
			std::string		name;
		};

		struct server_instruction_tab_entry_t {
			server_instruction_t	instruction_index;
			std::string 	name;
		};

		struct location_instruction_tab_entry_t {
			location_instruction_t	instruction_index;
			std::string 	name;
		};

		struct status_code_tab_entry_t {
			status_code_t	code_index;
			int				code_int;
			std::string		code_str;
			std::string		reason_phrase;
		};

		struct header_tab_entry_t {
			header_name_t	header_index;
			std::string		name;
		};

		struct cgi_header_tab_entry_t {
			cgi_header_name_t	header_index;
			std::string			name;
		};

		struct accepted_charsets_entry_t {
			accepted_charsets_t charset_index;
			std::string			name;
		};

		struct mime_type_entry_t {
			mime_type_t	mime_type_index;
			std::string ext;
			std::string	name;
		};

		struct encoding_type_entry_t {
			encoding_type_t encoding_type_t;
			std::string 	name;
		};

		static const method_tab_entry_t	method_tab[];
		static const server_instruction_tab_entry_t server_instructions_tab[];
		static const location_instruction_tab_entry_t location_instructions_tab[];
		static const status_code_tab_entry_t status_codes_tab[];
		static const header_tab_entry_t headers_tab[];
		static const header_tab_entry_t request_headers_tab[];
		static const header_tab_entry_t response_headers_tab[];
		static const cgi_header_tab_entry_t cgi_headers_tab[];
		static const accepted_charsets_entry_t charsets_tab[];
		static const mime_type_entry_t mime_types_tab[];
		static const encoding_type_entry_t encoding_types_tab[];

		static bool is_informational_code(int code);
		static bool is_successful_code(int code);
		static bool is_redirection_code(int code);
		static bool is_client_error_code(int code);
		static bool is_server_error_code(int code);
		static bool is_error_code(int code);

		static int fetch_method_index(const std::string& method);

		static bool str_is_num(const std::string& str);
		static bool str_is_alpha(const std::string& str);
		static bool str_is_alnum(const std::string& str);
		static int check_ip_format(const std::string& ip);
		static int trim_semicolon(std::vector<std::string>& str);
		static std::string	trim_comments(const std::string &str);
		static std::string trim_whitespaces(const std::string& str);
		static std::vector<std::string> split(const std::string& str, const std::string& charset);
		static std::string str_to_lower(const std::string& str);

		static URI_form_t get_URI_form(const std::string& uri_str);
		static path_type_t get_path_type(const std::string &path);

		static std::string body_error_code(status_code_t error_code);
		static std::string format_status_line(const std::string& http_version, status_code_t status_code);
		static std::string format_header_field(header_name_t header_code, const std::string& header_value);
		static void format_directory_name(std::string& directory_name);
		static void trail_begining_backslash(std::string& path);

		template<typename T>
		static bool is_accepted_value(const std::string& value, const T* accepted_value, size_t accepted_size) {
			if (value == "*")
				return true;
			for (size_t i = 0; i < accepted_size; i++) {
				if(accepted_value[i].name == value)
					return true;
			}
			return false;
		}


	private:
		Syntax();
		Syntax(const Syntax& src);
		Syntax& operator=(const Syntax& rhs);};

#endif
