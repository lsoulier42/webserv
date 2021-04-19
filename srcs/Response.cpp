/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 20:00:24 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/10 22:53:07 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(void) :
	AHTTPMessage(),
	_status_line(),
	_target_path(),
	_content_type() {}

Response::Response(const Response &x) :
	AHTTPMessage(x),
	_status_line(x._status_line),
	_target_path(x._target_path),
	_content_type(x._content_type) {}

Response::~Response(void) {}

Response
&Response::operator=(const Response &x) {
	if (this != &x) {
		AHTTPMessage::operator=(x);
		_status_line = x._status_line;
		_target_path = x._target_path;
		_content_type = x._content_type;
	}
	return (*this);
}

Response::StatusLine
&Response::get_status_line(void) {
	return (_status_line);
}

const Response::StatusLine
&Response::get_status_line(void) const {
	return (_status_line);
}

void
Response::reset(void) {
	AHTTPMessage::reset();
	_status_line.reset();
}

Response::StatusLine::StatusLine(void) :
	AStartLine(),
	_status_code(TOTAL_STATUS_CODE) {}

Response::StatusLine::StatusLine(const StatusLine &x) :
	AStartLine(x),
	_status_code(x._status_code) {}

Response::StatusLine::~StatusLine(void) {}

Response::StatusLine
&Response::StatusLine::operator=(const StatusLine &x) {
	AStartLine::operator=(x);
	_status_code = x._status_code;
	return (*this);
}

status_code_t
Response::StatusLine::get_status_code(void) const {
	return (_status_code);
}

void
Response::StatusLine::set_status_code(status_code_t status_code) {
	_status_code = status_code;
}

void
Response::StatusLine::reset(void) {
	AStartLine::reset();
	_status_code = TOTAL_STATUS_CODE;
}

std::string
Response::get_target_path(void) const {
	return _target_path;
}

void
Response::set_target_path(const std::string& target_path) {
	_target_path = target_path;
}

std::string
Response::get_content_type(void) const {
	return _content_type;
}

void
Response::set_content_type(const std::string& content_type) {
	_content_type = content_type;
}
