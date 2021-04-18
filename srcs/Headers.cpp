/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/18 05:07:54 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/18 20:35:44 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Headers.hpp"

header_iterator::header_iterator(void) :
	_cur(),
	_first(),
	_last(),
	_cell() {}

header_iterator::header_iterator(std::list<header_t>::iterator x, std::vector<std::list<header_t> >::iterator y) :
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
		std::vector<std::list<header_t> >::iterator new_cell(_cell + 1);
		while (new_cell->empty())
			++new_cell;
		_set_cell(new_cell);
		_cur = _first;
	}
	return (*this);
}

header_iterator
header_iterator::operator++(int) {
	header_iterator	tmp(*this);
	++(*this);
	return (tmp);
}

header_iterator
&header_iterator::operator--(void) {
	if (_cur == _first) {
		std::vector<std::list<header_t> >::iterator new_cell(_cell - 1);
		while (new_cell->empty())
			--new_cell;
		_set_cell(new_cell);
		_cur = _last;
	}
	--_cur;
	return (*this);
}

header_iterator
header_iterator::operator--(int) {
	header_iterator	tmp(*this);
	--(*this);
	return (tmp);
}

void
header_iterator::_set_cell(std::vector<std::list<header_t> >::iterator cell) {
	_cell = cell;
	_first = cell->begin();
	_last = cell->end();
}

bool
operator==(const header_iterator &lhs, const header_iterator &rhs) {
	return (lhs._cur == rhs._cur);
}

bool
operator!=(const header_iterator &lhs, const header_iterator &rhs) {
	return (!(lhs == rhs));
}

const_header_iterator::const_header_iterator(void) :
	_cur(),
	_first(),
	_last(),
	_cell() {}

const_header_iterator::const_header_iterator(const header_iterator &it) :
	_cur(it._cur),
	_first(it._first),
	_last(it._last),
	_cell(it._cell) {}

const_header_iterator::const_header_iterator(std::list<header_t>::const_iterator x, std::vector<std::list<header_t> >::const_iterator y) :
	_cur(x),
	_first(y->begin()),
	_last(y->end()),
	_cell(y) {}

const header_t
&const_header_iterator::operator*(void) {
	return (*_cur);
}

const header_t
*const_header_iterator::operator->(void) {
	return (&(*_cur));
}

const_header_iterator
&const_header_iterator::operator++(void) {
	++_cur;
	if (_cur == _last) {
		std::vector<std::list<header_t> >::const_iterator new_cell(_cell + 1);
		while (new_cell->empty())
			++new_cell;
		_set_cell(new_cell);
		_cur = _first;
	}
	return (*this);
}

const_header_iterator
const_header_iterator::operator++(int) {
	const_header_iterator	tmp(*this);
	++(*this);
	return (tmp);
}

const_header_iterator
&const_header_iterator::operator--(void) {
	if (_cur == _first) {
		std::vector<std::list<header_t> >::const_iterator new_cell(_cell - 1);
		while (new_cell->empty())
			--new_cell;
		_set_cell(new_cell);
		_cur = _last;
	}
	--_cur;
	return (*this);
}

const_header_iterator
const_header_iterator::operator--(int) {
	const_header_iterator	tmp(*this);
	--(*this);
	return (tmp);
}

void
const_header_iterator::_set_cell(std::vector<std::list<header_t> >::const_iterator cell) {
	_cell = cell;
	_first = cell->begin();
	_last = cell->end();
}

bool
operator==(const const_header_iterator &lhs, const const_header_iterator &rhs) {
	return (lhs._cur == rhs._cur);
}

bool
operator!=(const const_header_iterator &lhs, const const_header_iterator &rhs) {
	return (!(lhs == rhs));
}

const size_t Headers::_tab_size(30);

Headers::Headers(void) :
	_tab(_tab_size),
	_start(_tab.front().begin(), _tab.begin()),
	_finish(_tab.front().end(), _tab.begin()) {}

Headers::Headers(const Headers &x) :
	_tab(_tab_size) {
	for (const_iterator it(x.begin()) ; it != x.end() ; it++)
		insert(*it);
}

Headers::~Headers(void) {
	clear();
}

Headers
&Headers::operator=(const Headers &x) {
	clear();
	for (const_iterator it(x.begin()) ; it != x.end() ; it++)
		insert(*it);
	return (*this);
}

Headers::iterator
Headers::begin(void) {
	return (_start);
}

Headers::const_iterator
Headers::begin(void) const {
	return (const_iterator(_start));
}

Headers::iterator
Headers::end(void) {
	return (_finish);
}

Headers::const_iterator
Headers::end(void) const {
	return (const_iterator(_finish));
}

void
Headers::clear(void) {
	for (std::vector<std::list<header_t> >::iterator it(_tab.begin()) ; it != _tab.end() ; it++)
		it->clear();
	_start._set_cell(_tab.begin());
	_start._cur = _start._first;
	_finish._set_cell(_tab.begin());
	_finish._cur = _finish._last;
}

void
Headers::render(void) const {
	for (const_iterator it(begin()) ; it != end() ; it++) {
		std::cout << "*" << std::endl;
		std::cout << "KEY : " << it->name << "$" << std::endl;
		std::cout << "HASH : " << _hash(it->name.c_str()) << "$" << std::endl;
		std::cout << "VALUE : " << it->unparsed_value << "$" << std::endl;
		std::cout << "*" << std::endl;
	}
}

bool
Headers::empty(void) const {
	return (_start == _finish);
}

void
Headers::insert(const header_t &header) {
	unsigned long	index(_hash(header.name.c_str()));
	(_tab[index]).push_front(header);
	if (empty()) {
		_tab[index].push_back(header_t());
		_start._set_cell(_tab.begin() + index);
		_start._cur = _start._first;
		_finish._set_cell(_tab.begin() + index);
		_finish._cur = --_finish._last;
	}
	if (_start._cell > _tab.begin() + index) {
		_start._set_cell(_tab.begin() + index);
		_start._cur = _start._first;
	}
	if (_finish._cell <= _tab.begin() + index) {
		_finish._cell->erase(_finish._cur);
		_tab[index].push_back(header_t());
		_finish._set_cell(_tab.begin() + index);
		_finish._cur = --_finish._last;
	}
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
	std::list<header_t>	entry_list(_tab[_hash(key.c_str())]);
	if (entry_list.empty())
		return (false);
	for (std::list<header_t>::iterator it(entry_list.begin()) ; it != entry_list.end() ; it++)
		if (it->name == key)
			return (true);
	return (false);
}

const std::string
&Headers::get_unparsed_value(const std::string &key) const throw(std::invalid_argument) {
	const std::list<header_t>	&entry_list(_tab[_hash(key.c_str())]);
	for (std::list<header_t>::const_iterator it(entry_list.begin()) ; it != entry_list.end() ; it++)
		if (it->name == key)
			return (it->unparsed_value);
	throw (std::invalid_argument("Headers::get_unparsed_value : invalid argument"));
}

const std::list<std::string>&
Headers::get_value(const std::string &key) const throw (std::invalid_argument) {
	const std::list<header_t>	&entry_list(_tab[_hash(key.c_str())]);
	for (std::list<header_t>::const_iterator it(entry_list.begin()) ; it != entry_list.end() ; it++)
		if (it->name == key)
			return (it->value);
	throw (std::invalid_argument("Headers::get_value : invalid argument"));
}

void
Headers::set_value(const std::string &key, const std::list<std::string>& parsed_value) throw (std::invalid_argument) {
	std::list<header_t>	&entry_list(_tab[_hash(key.c_str())]);
	for (std::list<header_t>::iterator it(entry_list.begin()); it != entry_list.end(); it++)
		if (it->name == key) {
			it->value = parsed_value;
			return ;
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
