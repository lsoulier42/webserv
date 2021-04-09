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
	LOCATION,
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

class Syntax {
	public:
		Syntax();
		~Syntax();

		struct method_tab_entry_t {
			method_t		method;
			std::string		str;
		};
		struct instructions_tab_entry_t {
			instruction_t	instruction;
			std::string 	str;
		};
		struct status_code_tab_entry_t {
			status_code_t	index;
			size_t 			code;
			std::string		str;
		};

		static const method_tab_entry_t	method_tab[];
		static const instructions_tab_entry_t server_instructions_tab[];
		static const instructions_tab_entry_t location_instructions_tab[];
		static const status_code_tab_entry_t status_codes_tab[];

		static bool is_informational_code(int code);
		static bool is_successful_code(int code);
		static bool is_redirection_code(int code);
		static bool is_client_error_code(int code);
		static bool is_server_error_code(int code);
		static bool is_error_code(int code);
		static int method_index(const std::string& method);

	private:
		Syntax(const Syntax& src);
		Syntax& operator=(const Syntax& rhs);};

#endif
