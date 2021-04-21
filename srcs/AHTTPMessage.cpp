/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHTTPMessage.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 17:24:38 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/18 10:06:13 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AHTTPMessage.hpp"

AHTTPMessage::AHTTPMessage(void) :
	_headers(),
	_body() {}

AHTTPMessage::AHTTPMessage(const AHTTPMessage &x) throw(std::bad_alloc) {
	*this = x;
}

AHTTPMessage::~AHTTPMessage(void) {

}

AHTTPMessage
&AHTTPMessage::operator=(const AHTTPMessage &x) throw(std::bad_alloc) {
	if (this != &x) {
		_headers = x._headers;
		_body = x._body;
	}
	return (*this);
}

const AHTTPMessage::HTTPHeaders
&AHTTPMessage::get_headers(void) const {
	return (_headers);
}

AHTTPMessage::HTTPHeaders
&AHTTPMessage::get_headers(void) {
	return (_headers);
}

const ByteArray&
AHTTPMessage::get_body(void) const {
	return (_body);
}

void
AHTTPMessage::set_body(const ByteArray& body) {
	_body = body;
}

void
AHTTPMessage::reset(void) {
	_headers.clear();
	_body.clear();
}

AHTTPMessage::AStartLine::AStartLine(void) :
	_http_version() {}

AHTTPMessage::AStartLine::AStartLine(const AStartLine &x) :
	_http_version(x._http_version) {}

AHTTPMessage::AStartLine::~AStartLine(void) {}

AHTTPMessage::AStartLine
&AHTTPMessage::AStartLine::operator=(const AStartLine &x) {
	_http_version = x._http_version;
	return (*this);
}

const std::string
&AHTTPMessage::AStartLine::get_http_version(void) const {
	return (_http_version);
}

void
AHTTPMessage::AStartLine::set_http_version(const std::string &http_version) {
	_http_version = http_version;
}

void
AHTTPMessage::AStartLine::reset(void) {
	_http_version.clear();
}

AHTTPMessage::HTTPHeaders::HTTPHeaders(void) :
	Headers() {}

AHTTPMessage::HTTPHeaders::HTTPHeaders(const HTTPHeaders &x) :
	Headers(x) {}

AHTTPMessage::HTTPHeaders::~HTTPHeaders(void) {}

AHTTPMessage::HTTPHeaders
&AHTTPMessage::HTTPHeaders::operator=(const HTTPHeaders &x) {
	Headers::operator=(x);
	return (*this);
}

void
AHTTPMessage::HTTPHeaders::insert(const header_t &header) {
	Headers::insert(header);
}

void
AHTTPMessage::HTTPHeaders::insert(header_name_t key, const std::string& unparsed_value) {
	Headers::insert(Syntax::headers_tab[key].name, unparsed_value);
}

bool
AHTTPMessage::HTTPHeaders::key_exists(header_name_t key) const {
	return (Headers::key_exists(Syntax::headers_tab[key].name));
}

const std::string
&AHTTPMessage::HTTPHeaders::get_unparsed_value(header_name_t key) const throw(std::invalid_argument) {
	return (Headers::get_unparsed_value(Syntax::headers_tab[key].name));
}

const std::list<std::string>&
AHTTPMessage::HTTPHeaders::get_value(header_name_t key) const throw (std::invalid_argument) {
	return (Headers::get_value(Syntax::headers_tab[key].name));
}

void
AHTTPMessage::HTTPHeaders::set_value(header_name_t key, const std::list<std::string>& parsed_value) throw (std::invalid_argument) {
	Headers::set_value(Syntax::headers_tab[key].name, parsed_value);
}
