/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHTTPMessage.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/09 17:24:38 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/09 18:53:44 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AHTTPMessage.hpp"

AHTTPMessage::AHTTPMessage(void) :
	_headers(),
	_body() {}

AHTTPMessage::AHTTPMessage(const AHTTPMessage &x) :
	_headers(x._headers),
	_body(x._body) {}

AHTTPMessage::~AHTTPMessage(void) {}

AHTTPMessage
&AHTTPMessage::operator=(const AHTTPMessage &x) {
	_headers = x._headers;
	_body = x._body;
	return (*this);
}

const AHTTPMessage::Headers
&AHTTPMessage::get_headers(void) const {
	return (_headers);
}

AHTTPMessage::Headers
&AHTTPMessage::get_headers(void) {
	return (_headers);
}

const std::string
&AHTTPMessage::get_body(void) const {
	return (_body);
}

void
AHTTPMessage::set_body(const std::string &body) {
	_body = body;
}

void
AHTTPMessage::reset(void) {
	_headers.reset();
	_body.clear();
}

/*
 * AStartLine related functions
 */

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

/*
 * Headers related functions
 */

const size_t	AHTTPMessage::Headers::_tab_size(30);

AHTTPMessage::Headers::Headers(void) :
	_tab(_tab_size) {}

AHTTPMessage::Headers::Headers(const Headers &x) :
	_tab(_tab_size) {
	for (size_t i(0) ; i < _tab_size ; i++)
		if (x._tab[i])
			_tab[i] = new std::list<header_t>(*(x._tab[i]));
}

AHTTPMessage::Headers::~Headers(void) {
	reset();
}

AHTTPMessage::Headers
&AHTTPMessage::Headers::operator=(const Headers &x) {
	for (size_t i(0) ; i < _tab_size ; i++) {
		if (_tab[i]) {
			delete _tab[i];
			_tab[i] = 0;
		}
		if (x._tab[i])
			_tab[i] = new std::list<header_t>(*(x._tab[i]));
	}
	return (*this);
}

void
AHTTPMessage::Headers::reset(void) {
	for (size_t i(0) ; i < _tab_size ; i++)
		if (_tab[i]) {
			delete _tab[i];
			_tab[i] = 0;
		}
}

void
AHTTPMessage::Headers::render(void) const {
	for (size_t i(0) ; i < _tab_size ; i++) {
		if (_tab[i]) {
			for (std::list<header_t>::iterator it(_tab[i]->begin()) ; it != _tab[i]->end() ; it++) {
				std::cout << "*" << std::endl;
				std::cout << "KEY : " << it->key << "$" << std::endl;
				std::cout << "HASH : " << _hash(it->key.c_str()) << "$" << std::endl;
				std::cout << "VALUE : " << it->value << "$" << std::endl;
				std::cout << "*" << std::endl;
			}
		}
	}
}

//TODO:: gerer l'insertion d'un header dont la cle est deja presente
void
AHTTPMessage::Headers::insert(const std::string &header_name, const std::string &header_value) {
	header_t	new_entry;
	new_entry.key = header_name;
	new_entry.value = header_value;
	unsigned long	index(_hash(header_name.c_str()));
	if (!_tab[index])
		_tab[index] = new std::list<header_t>;
	(_tab[index])->push_front(new_entry);
}

bool
AHTTPMessage::Headers::key_exists(const std::string &key) const {
	std::list<header_t>	*entry_list(_tab[_hash(key.c_str())]);
	if (!entry_list)
		return (false);
	for (std::list<header_t>::iterator it(entry_list->begin()) ; it != entry_list->end() ; it++)
		if (it->key == key)
			return (true);
	return (false);
}

std::string
&AHTTPMessage::Headers::get_value(const std::string &key) throw(std::invalid_argument) {
	std::list<header_t>	*entry_list(_tab[_hash(key.c_str())]);
	if (entry_list) {
		for (std::list<header_t>::iterator it(entry_list->begin()) ; it != entry_list->end() ; it++)
			if (it->key == key)
				return (it->value);
	}
	throw (std::invalid_argument("Request::Headers::get_value : invalid argument"));
}

const std::string
&AHTTPMessage::Headers::get_value(const std::string &key) const throw(std::invalid_argument) {
	std::list<header_t>	*entry_list(_tab[_hash(key.c_str())]);
	if (entry_list) {
		for (std::list<header_t>::const_iterator it(entry_list->begin()) ; it != entry_list->end() ; it++)
			if (it->key == key)
				return (it->value);
	}
	throw (std::invalid_argument("Request::Headers::get_value : invalid argument"));
}

unsigned long
AHTTPMessage::Headers::_hash(const char *buffer) const {
	unsigned long	hash(5381);
	char			c;
	size_t			i(0);
	while ((c = buffer[i++]))
		hash = ((hash << 5) + hash) + c;
	return (hash % _tab_size);
}