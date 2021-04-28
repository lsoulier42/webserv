/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/27 11:59:14 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/28 11:54:56 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# include <algorithm>
# include <cstring>
# include <unistd.h>
# include <cstdio>
# include <cerrno>
# include <sys/wait.h>
# include "CGIMetaVariables.hpp"
# include "CGIScriptArgs.hpp"
# include "CGIResponse.hpp"
# include "ResponseHandling.hpp"
# include "Client.hpp"
# include "Request.hpp"
# include "Syntax.hpp"
# include "Debugger.hpp"

class CGI {

	public:

		enum cgi_read_ret_t {
			AGAIN,
			COMPLETE,
			REDIRECT,
			SERVER_ERROR,
			SCRIPT_ERROR
		};

		static bool is_cgi_related(const Request &request);
		static int init(Client &client);
		static int write_input(Client &client);
		static cgi_read_ret_t read_output(Client &client);

	private:

		CGI(void);

		static int _create_child_process(void);
		static int _launch_script(Client &client);

		static void _collect_header(CGIResponse &cgi_response, ByteArray &output);
		static int _check_headers(CGIResponse &cgi_response, ByteArray &output);
		static void _collect_body(CGIResponse &cgi_response, ByteArray &output);

		static cgi_read_ret_t _handle_cgi_response(Client &client);
		static cgi_read_ret_t _handle_local_redirect_response(Client &client);
		static cgi_read_ret_t _handle_client_redirect_response(Client &client);
		static cgi_read_ret_t _handle_client_redirect_doc_response(Client &client);
		static cgi_read_ret_t _handle_document_response(Client &client);

		/* Status predicates
		 */
		static bool _header_received(const CGIResponse &cgi_response, const ByteArray &output);
		static bool _headers_received(const CGIResponse &cgi_response, const ByteArray &output);
		static bool _body_received(const CGIResponse &cgi_response, const ByteArray &output);
		static bool _body_expected(const CGIResponse &cgi_response);

		/* Type predicates
		 */
		static bool _is_local_redirection(const std::string &location);
		static bool _is_client_redirection(const std::string &location);
		static bool _is_redirection_status(const std::string &status_line);
		static bool _is_document_response(const CGIResponse &cgi_response);
		static bool _is_local_redirect_response(const CGIResponse &cgi_response);
		static bool _is_client_redirect_response(const CGIResponse &cgi_response);
		static bool _is_client_redirect_response_with_document(const CGIResponse &cgi_response);

};

#endif
