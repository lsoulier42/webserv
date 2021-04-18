/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/18 05:07:54 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/18 07:15:19 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Headers.hpp"

header_iterator::header_iterator(void) :
	_header() {}

header_iterator::header_iterator(std::list<header_t>::iterator x, std::vector<std::list<header_t>*>::iterator y) :
	_cur(x),
	_first(y->begin()),
	_last(y->end()),
	_cell(y) {}

header_t
&header_iterator::operator*(void) {
	return (*_cur);
}

header_t
*header_iterator::operator->(void) {
	return (&(*_cur));
}

header_iterator
&header_iterator::operator++(void) {
	++_cur;
	if (_cur == _last) {
		std::vector<std::list<header_t>*>::iterator new_cell(_cell + 1);
		while (!*new_cell)
			++new_cell;
		_set_cell(new_cell);
		_cur = _first;
	}
	return (*this);
}

header_iterator
header_iterator::operator++(int) {
}

void
header_iterator::_set_cell(std::vector<std::list<header_t>*>::iterator cell) {
	_cell = cell;
	_first = cell->begin();
	_last = cell->end();
}

const size_t Headers::_tab_size(30);

Headers::Headers(void) :
	_tab(_tab_size) {}

Headers::Headers(const Headers &x) :
	_tab(_tab_size) {
	for (size_t i(0) ; i < _tab_size ; i++)
		if (x._tab[i])
			_tab[i] = new std::list<header_t>(*(x._tab[i]));
}

Headers::~Headers(void) {
	reset();
}

Headers
&Headers::operator=(const Headers &x) {
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
Headers::reset(void) {
	for (size_t i(0) ; i < _tab_size ; i++)
		if (_tab[i]) {
			delete _tab[i];
			_tab[i] = 0;
		}
}

void
Headers::render(void) const {
	for (size_t i(0) ; i < _tab_size ; i++) {
		if (_tab[i]) {
			for (std::list<header_t>::iterator it(_tab[i]->begin()) ; it != _tab[i]->end() ; it++) {
				std::cout << "*" << std::endl;
				std::cout << "KEY : " << it->name << "$" << std::endl;
				std::cout << "HASH : " << _hash(it->name.c_str()) << "$" << std::endl;
				std::cout << "VALUE : " << it->unparsed_value << "$" << std::endl;
				std::cout << "*" << std::endl;
			}
		}
	}
}

void
Headers::insert(const header_t &header) {
	unsigned long	index(_hash(header.name.c_str()));
	if (!_tab[index])
		_tab[index] = new std::list<header_t>;
	(_tab[index])->push_front(header);
}

void
Headers::insert(const std::string &key, const std::string &unparsed_value) {
	header_t new_header;

	new_header.name = key;
	new_header.unparsed_value = unparsed_value;
	insert(new_header);
}

bool
Headers::key_exists(const std::string &key) const {
	std::list<header_t>	*entry_list(_tab[_hash(key.c_str())]);
	if (!entry_list)
		return (false);
	for (std::list<header_t>::iterator it(entry_list->begin()) ; it != entry_list->end() ; it++)
		if (it->name == key)
			return (true);
	return (false);
}

const std::string
&Headers::get_unparsed_value(const std::string &key) const throw(std::invalid_argument) {
	std::list<header_t>	*entry_list(_tab[_hash(key.c_str())]);
	if (entry_list) {
		for (std::list<header_t>::const_iterator it(entry_list->begin()) ; it != entry_list->end() ; it++)
			if (it->name == key)
				return (it->unparsed_value);
	}
	throw (std::invalid_argument("Headers::get_unparsed_value : invalid argument"));
}

const std::list<std::string>&
Headers::get_value(const std::string &key) const throw (std::invalid_argument) {
	std::list<header_t>	*entry_list(_tab[_hash(key.c_str())]);
	if (entry_list) {
		for (std::list<header_t>::const_iterator it(entry_list->begin()) ; it != entry_list->end() ; it++)
			if (it->name == key)
				return (it->value);
	}
	throw (std::invalid_argument("Headers::get_value : invalid argument"));
}

void
Headers::set_value(const std::string &key, const std::list<std::string>& parsed_value) throw (std::invalid_argument) {
	std::list<header_t>	*entry_list(_tab[_hash(key.c_str())]);
	if (entry_list) {
		for (std::list<header_t>::iterator it(entry_list->begin()); it != entry_list->end(); it++)
			if (it->name == key) {
				it->value = parsed_value;
				return ;
			}
	}
	throw (std::invalid_argument("Headers::set_value : invalid argument"));
}

unsigned long
Headers::_hash(const char *buffer) const {
	unsigned long	hash(5381);
	char			c;
	size_t			i(0);

	while ((c = buffer[i++]))
		hash = ((hash << 5) + hash) + c;
	return (hash % _tab_size);
}
