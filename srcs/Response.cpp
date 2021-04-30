/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 20:00:24 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/30 05:54:19 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(void) :
	AHTTPMessage(),
	_status(START),
	_sending_indicator(),
	_length(),
	_chunked(false),
	_status_line(),
	_head(),
	_content(),
	_target_path(),
	_content_type() {}

Response::Response(const Response &x) :
	AHTTPMessage(x),
	_status(x._status),
	_sending_indicator(x._sending_indicator),
	_length(x._length),
	_chunked(x._chunked),
	_status_line(x._status_line),
	_head(x._head),
	_content(x._content),
	_target_path(x._target_path),
	_content_type(x._content_type) {}

Response::~Response(void) {}

Response
&Response::operator=(const Response &x) {
	if (this != &x) {
		AHTTPMessage::operator=(x);
		_status = x._status;
		_sending_indicator = x._sending_indicator;
		_length = x._length;
		_chunked = x._chunked;
		_status_line = x._status_line;
		_head = x._head;
		_content = x._content;
		_target_path = x._target_path;
		_content_type = x._content_type;
	}
	return (*this);
}

Response::response_status_t
Response::get_status(void) const {
	return (_status);
}

size_t
Response::get_sending_indicator(void) const {
	return (_sending_indicator);
}

size_t
Response::get_length(void) const {
	return (_length);
}

bool
Response::get_chunked(void) const {
	return (_chunked);
}

Response::StatusLine
&Response::get_status_line(void) {
	return (_status_line);
}

const Response::StatusLine
&Response::get_status_line(void) const {
	return (_status_line);
}

ByteArray
&Response::get_head(void) {
	return (_head);
}

const ByteArray
&Response::get_head(void) const {
	return (_head);
}

ByteArray
&Response::get_content(void) {
	return (_content);
}

const ByteArray
&Response::get_content(void) const {
	return (_content);
}

std::string
Response::get_target_path(void) const {
	return _target_path;
}

std::string
Response::get_content_type(void) const {
	return _content_type;
}

void
Response::set_status(response_status_t status) {
	_status = status;
}

void
Response::set_sending_indicator(size_t sending_indicator) {
	_sending_indicator = sending_indicator;
}

void
Response::set_length(size_t length) {
	_length = length;
}

void
Response::set_chunked(bool chunked) {
	_chunked = chunked;
}

void
Response::set_content(const ByteArray &content) {
	_content = content;
}

void
Response::set_target_path(const std::string& target_path) {
	_target_path = target_path;
}

void
Response::set_content_type(const std::string& content_type) {
	_content_type = content_type;
}

void
Response::append_content_chunk(const char *buffer, ssize_t size) {
	std::stringstream	size_ss;
	std::string			size_str;

	size_ss << std::hex << size;
	size_str = size_ss.str();
	_content.append(size_str.c_str(), size_str.size());
	_content.append("\r\n", 2);
	_content.append(buffer, size);
	_content.append("\r\n", 2);
	_sending_indicator = size;
}

void
Response::reset(void) {
	AHTTPMessage::reset();
	_status = START;
	_sending_indicator = 0;
	_length = 0;
	_chunked = false;
	_status_line.reset();
	_head.clear();
	_content.clear();
	_target_path.clear();
	_content_type.clear();
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
