/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/27 11:59:14 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/27 16:15:20 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# include <algorithm>
# include <cstring>
# include <unistd.h>
# include <cerrno>
# include "CGIMetaVariables.hpp"
# include "CGIScriptArgs.hpp"
# include "CGIResponse.hpp"
# include "Client.hpp"
# include "Request.hpp"
# include "Syntax.hpp"

class CGI {

	public:

		static bool is_cgi_related(const Request &request);
		static int init(Client &client);
		static int write_input(Client &client);
		static int read_output(Client &client);

	private:

		CGI(void);

		static int _create_child_process(void);
		static int _launch_script(Client &client);

		static void _collect_header(CGIResponse &cgi_response, ByteArray &output);
		static int _check_headers(CGIResponse &cgi_response);
		static void _collect_body(CGIResponse &cgi_response);

		static int _handle_cgi_response(Client &client);
		static int _handle_local_redirect_response(Client &client);
		static int _handle_client_redirect_response(Client &client);
		static int _handle_client_redirect_doc_response(Client &client);
		static int _handle_document_response(Client &client);

		/* Status predicates
		 */
		static bool _cgi_header_received(const CGIResponse &cgi_response, const ByteArray &output);
		static bool _cgi_headers_received(const CGIResponse &cgi_response, const ByteArray &output);
		static bool _cgi_body_received(const CGIResponse &cgi_response, const ByteArray &output);
		static bool _cgi_body_expected(const CGIResponse &cgi_response);

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
