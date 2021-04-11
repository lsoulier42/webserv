/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Syntax.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 21:57:42 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/09 21:57:43 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SYNTAX_HPP
# define SYNTAX_HPP
# include <string>
# include <list>
# include <vector>
# include <map>
# include <cstdlib>
# include "AHTTPMessage.hpp"

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

enum instruction_t {
	LISTEN,
	SERVER_NAME,
	ERROR_PAGE,
	CLIENT_MAX_BODY_SIZE,
	LOCATION_INSTRUCTION,
	METHODS,
	ROOT,
	AUTOINDEX,
	INDEX,
	UPLOAD_DIR,
	CGI,
	TOTAL_SERVER_INSTRUCTIONS
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

enum header_names_t {
	ACCEPT_CHARSETS,
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
	TOTAL_HEADER_NAMES
};

enum accepted_charsets_t {
	UTF_8,
	ISO_8859_1,
	UNICODE_1_1,
	TOTAL_ACCEPTED_CHARSETS
};

enum accepted_language_t {
	FR,
	FR_BE,
	FR_CA,
	FR_FR,
	FR_LU,
	FR_CH,
	EN,
	EN_CA,
	EN_GB,
	EN_US,
	TOTAL_ACCEPTED_LANGUAGES
};

class Syntax {
	public:
		Syntax();
		~Syntax();

		struct method_tab_entry_t {
			method_t		method_index;
			std::string		name;
		};
		struct instruction_tab_entry_t {
			instruction_t	instruction_index;
			std::string 	name;
		};
		struct status_code_tab_entry_t {
			status_code_t	code_index;
			size_t 			code;
			std::string		reason_phrase;
		};

		struct header_tab_entry_t {
			header_names_t	header_index;
			std::string		name;
		};

		struct accepted_charsets_entry_t {
			accepted_charsets_t charset_index;
			std::string			name;
		};

		struct accepted_languages_entry_t {
			accepted_language_t language_index;
			std::string			name;
		};

		static const method_tab_entry_t	method_tab[];
		static const instruction_tab_entry_t server_instructions_tab[];
		static const instruction_tab_entry_t location_instructions_tab[];
		static const status_code_tab_entry_t status_codes_tab[];
		static const header_tab_entry_t headers_tab[];
		static const accepted_charsets_entry_t charsets_tab[];
		static const accepted_languages_entry_t languages_tab[];

		static bool is_informational_code(int code);
		static bool is_successful_code(int code);
		static bool is_redirection_code(int code);
		static bool is_client_error_code(int code);
		static bool is_server_error_code(int code);
		static bool is_error_code(int code);

		static int fetch_method_index(const std::string& method);

		static std::string	trim_comments(const std::string &line_buffer);
		static bool is_num(const char* str);
		static int check_ip_format(const std::string& ip);
		static std::string trim_whitespaces(const std::string& line_buffer);
		static std::vector<std::string> split_whitespaces(const std::string& line_buffer);
		static std::vector<std::string> split(const std::string& line_buffer, const std::string& charset);
		static int trim_semicolon(std::vector<std::string>& tokens);

		static bool is_implemented_header(const std::string& header_name);
		static std::list<std::string> parse_header_value(const AHTTPMessage::Headers::header_t& header);
		static std::multimap<float, std::string> split_weight(const std::vector<std::string>& elements_split);
		
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
		Syntax(const Syntax& src);
		Syntax& operator=(const Syntax& rhs);};

#endif
