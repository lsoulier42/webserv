/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResponse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/16 20:39:01 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/30 06:20:59 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIResponse.hpp"

CGIResponse::CGIResponse(void) :
	_type(NO_TYPE),
	_headers(),
	_content_reception(false) {}

CGIResponse::CGIResponse(const CGIResponse &x) :
	_type(x._type),
	_headers(x._headers),
	_content_reception(x._content_reception) {}

CGIResponse::~CGIResponse(void) {}

CGIResponse
&CGIResponse::operator=(const CGIResponse &x) {
	_type = x._type;
	_headers = x._headers;
	_content_reception = x._content_reception;
	return (*this);
}

CGIResponse::type_t
CGIResponse::get_type(void) const {
	return (_type);
}

CGIResponse::CGIHeaders
&CGIResponse::get_headers(void) {
	return (_headers);
}

const CGIResponse::CGIHeaders
&CGIResponse::get_headers(void) const {
	return (_headers);
}

bool
CGIResponse::get_content_reception(void) const {
	return (_content_reception);
}

void
CGIResponse::set_type(type_t type) {
	_type = type;
}

void
CGIResponse::set_content_reception(bool content_reception) {
	_content_reception = content_reception;
}

void
CGIResponse::reset(void) {
	_type = NO_TYPE;
	_headers.clear();
	_content_reception = false;
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

void
CGIResponse::CGIHeaders::erase(cgi_header_name_t key) {
	Headers::erase(Syntax::cgi_headers_tab[key].name);
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
