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
	_body(NULL),
	_body_size(0) {}

AHTTPMessage::AHTTPMessage(const AHTTPMessage &x) throw(std::bad_alloc) {
	*this = x;
}

AHTTPMessage::~AHTTPMessage(void) {
	this->free_body();
}

AHTTPMessage
&AHTTPMessage::operator=(const AHTTPMessage &x) throw(std::bad_alloc) {
	if (this != &x) {
		_headers = x._headers;
		if (x._body) {
			_body = Syntax::buffer_dup(x._body, x._body_size);
			if (_body == NULL)
				throw(std::bad_alloc());
		}
		_body_size = x._body_size;
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

char*
AHTTPMessage::get_body(void) const {
	return (_body);
}

void
AHTTPMessage::set_body(const char* body, size_t body_size) throw(std::bad_alloc) {
	if (_body != NULL) {
		free(_body);
		_body = NULL;
	}
	_body = Syntax::buffer_dup(body, body_size);
	if (_body == NULL)
		throw(std::bad_alloc());
	_body_size = body_size;
}

void
AHTTPMessage::reset(void) {
	_headers.clear();
	if (_body) {
		free(_body);
		_body = NULL;
	}
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

size_t
AHTTPMessage::get_body_size() const {
	return _body_size;
}

void
AHTTPMessage::append_to_body(const char* to_append, size_t size_to_append) throw(std::bad_alloc) {
	if (_body == NULL)
		this->set_body(to_append, size_to_append);
	else {
		_body = Syntax::buffer_append(_body, to_append, _body_size, size_to_append);
		if (_body == NULL)
			throw(std::bad_alloc());
		_body_size += size_to_append;
	}
}

int
AHTTPMessage::free_body() {
	if (_body != NULL && _body_size != 0) {
		free(_body);
		_body = NULL;
		_body_size = 0;
	}
	return (FAILURE);
}
