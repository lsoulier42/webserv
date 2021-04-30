/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParsing.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/19 11:42:38 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/23 09:06:11 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTPARSING_HPP
# define REQUESTPARSING_HPP

# include <string>
# include <list>
# include "Client.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "VirtualServer.hpp"
# include "Syntax.hpp"
# include "Debugger.hpp"

class RequestParsing {

	public:
		static void parsing(Client &client);
		static bool body_expected(const Request &request);
		static bool is_valid_language_tag(const std::string& language_tag);
		static bool is_valid_http_date(const std::string& date_str);

	private:
		static void _failure(Response &response, status_code_t status_code);
		static bool _request_line_received(const Request &request, const ByteArray &input_str);
		static bool _header_received(const Request &request, const ByteArray &input_str);
		static bool _headers_received(const Request &request, const ByteArray &input_str);
		static bool _trailer_received(const Request &request, const ByteArray &input_str);
		static bool _trailers_received(const Request &request, const ByteArray &input_str);
		static bool _trailer_expected(const Request &request);
		static int _collect_request_line_elements(Request &request, ByteArray &input_str);
		static void _collect_header(Request &request, ByteArray &input_str);
		static int _check_headers(Client &client, Request &request);
		static int _check_trailer(Request &request, ByteArray &input_str);
		static void _collect_chunked(Request &request, ByteArray &input);
		static void _collect_unchunked(Request &request, ByteArray &input);
		static void _pick_virtual_server(Client &client, Request &request);
		static void _pick_location(Request &request);
		static int _process_request_headers(Client &client, Request &request);
		static int _request_accept_charset_parser(Request &request);
		static int _request_accept_language_parser(Request &request);
		static int _request_authorization_parser(Request &request);
		static int _request_content_length_parser(Request &request);
		static int _request_content_type_parser(Request &request);
		static int _request_date_parser(Request &request);
		static int _request_host_parser(Request &request);
		static int _request_referer_parser(Request &request);
		static int _request_transfer_encoding_parser(Request &request);
		static int _request_user_agent_parser(Request &request);
		static std::list<std::string> _parse_coma_q_factor(const std::string& unparsed_value);
		static bool _comp_q_factor(const std::pair<std::string, float> & a, const std::pair<std::string, float> & b);
		static bool _is_allowed_method(const std::list<std::string>& allowed_methods, const std::string& method);
		static void _handle_body(Client::exchange_t &exchange, ByteArray& input);
		static void _handle_request_line(Client::exchange_t &exchange, ByteArray &input);
		static void _handle_headers(Client &client, Client::exchange_t &exchange, ByteArray &input);
		static void _handle_trailers(Client::exchange_t &exchange, ByteArray &input);

		static size_t _uri_max_size;
		static size_t _header_max_size;
};

#endif
