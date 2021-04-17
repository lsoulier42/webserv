/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResponse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/16 20:39:01 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/16 20:41:23 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIResponse.hpp"

CGIResponse::CGIResponse(void) :
	_body() {}

CGIResponse::CGIResponse(const CGIResponse &x) :
	_body(x._body) {}

CGIResponse::~CGIResponse(void) {}

CGIResponse
&CGIResponse::operator=(const CGIResponse &x) {
	_body = x._body;
	return (*this);
}

const std::string
&CGIResponse::get_body(void) const {
	return (_body);
}

void
CGIResponse::set_body(const std::string &body) {
	_body = body;
}
