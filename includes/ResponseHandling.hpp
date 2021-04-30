/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandling.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: louise <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/19 17:53:28 by louise            #+#    #+#             */
/*   Updated: 2021/04/19 17:53:29 by louise           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HANDLING_HPP
# define RESPONSE_HANDLING_HPP
# include <iostream>
# include <string>
# include <list>
# include "Client.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Syntax.hpp"

class ResponseHandling {
	public:
		ResponseHandling();
		~ResponseHandling();

		static int process_response_headers(Client::exchange_t &exchange);
		static int process_cgi_response_headers(Client::exchange_t &exchange);
		static void generate_basic_headers(Client::exchange_t &exchange);
		static std::string get_current_HTTP_date(void);

	private:
		ResponseHandling(const ResponseHandling& src);
		ResponseHandling& operator=(const ResponseHandling& rhs);

		static int _response_allow_handler(Client::exchange_t &exchange);
		static int _response_content_language_handler(Client::exchange_t &exchange);
		static int _response_content_length_handler(Client::exchange_t &exchange);
		static int _response_content_location_handler(Client::exchange_t &exchange);
		static int _response_content_type_handler(Client::exchange_t &exchange);
		static int _response_date_handler(Client::exchange_t &exchange);
		static int _response_last_modified_handler(Client::exchange_t &exchange);
		static int _response_location_handler(Client::exchange_t &exchange);
		static int _response_retry_after_handler(Client::exchange_t &exchange);
		static int _response_server_handler(Client::exchange_t &exchange);
		static int _response_transfer_encoding_handler(Client::exchange_t &exchange);
		static int _response_www_authenticate_handler(Client::exchange_t &exchange);
		static void _pick_content_type(Client::exchange_t &exchange);
};

#endif
