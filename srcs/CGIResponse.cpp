/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResponse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/16 20:39:01 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/19 03:26:28 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIResponse.hpp"

CGIResponse::CGIResponse(void) :
	_headers(),
	_body() {}

CGIResponse::CGIResponse(const CGIResponse &x) :
	_headers(x._headers),
	_body(x._body) {}

CGIResponse::~CGIResponse(void) {}

CGIResponse
&CGIResponse::operator=(const CGIResponse &x) {
	_headers = x._headers;
	_body = x._body;
	return (*this);
}

CGIResponse::CGIHeaders
&CGIResponse::get_headers(void) {
	return (_headers);
}

const CGIResponse::CGIHeaders
&CGIResponse::get_headers(void) const {
	return (_headers);
}

const std::string
&CGIResponse::get_body(void) const {
	return (_body);
}

void
CGIResponse::set_body(const std::string &body) {
	_body = body;
}

CGIResponse::CGIHeaders::CGIHeaders(void) :
	Headers() {}

CGIResponse::CGIHeaders::CGIHeaders(const CGIHeaders &x) :
	Headers(x) {}

CGIResponse::CGIHeaders::~CGIHeaders(void) {}

CGIResponse::CGIHeaders
&CGIResponse::CGIHeaders::operator=(const CGIHeaders &x) {
	Headers::operator=(x);
	return (*this);
}

bool
CGIResponse::CGIHeaders::key_exists(cgi_header_name_t key) const {
	return (Headers::key_exists(Syntax::cgi_headers_tab[key].name));
}

const std::string
&CGIResponse::CGIHeaders::get_unparsed_value(cgi_header_name_t key) const throw(std::invalid_argument) {
	return (Headers::get_unparsed_value(Syntax::cgi_headers_tab[key].name));
}

const std::list<std::string>&
CGIResponse::CGIHeaders::get_value(cgi_header_name_t key) const throw (std::invalid_argument) {
	return (Headers::get_value(Syntax::cgi_headers_tab[key].name));
}
