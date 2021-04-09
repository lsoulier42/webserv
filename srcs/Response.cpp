/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 20:00:24 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/09 20:16:21 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(void) :
	AHTTPMessage(),
	_status_line() {}

Response::Response(const Response &x) :
	AHTTPMessage(x),
	_status_line(x._status_line) {}

Response::~Response(void) {}

Response
&Response::operator=(const Response &x) {
	AHTTPMessage::operator=(x);
	_status_line = x._status_line;
	return (*this);
}

const StatusLine
&Response::get_status_line(void) const {
	return (_status_line);
}

void
Response::reset(void) {
	AHTTPMessage::reset();
	_status_line.reset();
}

void
Response::render(void) const {
	std::cout << "---STATUS LINE---" << std::endl;
	_status_line.render();
}

Response::StatusLine::StatusLine(void) :
	AStartLine(),
	_status_code(),
	_reason_phrase() {}

Response::StatusLine::StatusLine(const StatusLine &x) :
	AStartLine(x),
	_status_code(x._status_code),
	_reason_phrase(x._reason_phrase) {}

Response::StatusLine
&Response::StatusLine::operator=(const StatusLine &x) {
	AStartLine::operator=(x);
	_status_code = x._status_code;
	_reason_phrase = x._reason_phrase;
	return (*this);
}

int
Response::StatusLine::get_status_code(void) const {
	return (_status_code);
}

const std::string
&Response::StatusLine::get_reason_phrase(void) const {
	return (_reason_phrase);
}

void
Response::StatusLine::reset(void) {
	AStartLine::reset();
	_status_code = 0;
	_reason_phrase.clear();
}

void
Response::StatusLine::render(void) {
	std::cout << "HTTP VERSION : " << get_http_version() << "$" << std::endl;
	std::cout << "STATUS CODE : " << _status_code << "$" << std::endl;
	std::cout << "REASON PHRASE : " << _reason_phrase << "$" << std::endl;
}
