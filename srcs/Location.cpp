/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/07 16:50:50 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/07 16:50:51 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location() : _autoindex(false) {

}

Location::Location(const Location& src) {
	*this = src;
}

Location&
Location::operator=(const Location& rhs) {
	if (this != &rhs) {
		_path = rhs._path;
		_root = rhs._root;
		_autoindex = rhs._autoindex;
		_index = rhs._index;
		_methods = rhs._methods;
		_cgi_path = rhs._cgi_path;
		_cgi_extension = rhs._cgi_extension;
	}
	return *this;
}

Location::~Location() {

}

std::string
Location::get_path() const {
	return _path;
}

void
Location::set_path(const std::string& path) {
	_path = path;
}

const std::string&
Location::get_root() const {
	return _root;
}

void
Location::set_root(const std::string &root) {
	_root = root;
}

std::list<std::string>
Location::get_methods() const {
	return _methods;
}

void
Location::set_methods(const std::list<std::string> &methods) {
	if (!_methods.empty())
		_methods.clear();
	_methods = methods;
}

std::list<std::string>
Location::get_index() const {
	return _index;
}

void
Location::set_index(const std::list<std::string> &index) {
	if (!_index.empty())
		_index.clear();
	_index = index;
}

std::string
Location::get_cgi_extension() const {
	return _cgi_extension;
}

void
Location::set_cgi_extension(const std::string &cgiExtension) {
	_cgi_extension = cgiExtension;
}

std::string
Location::get_cgi_path() const {
	return _cgi_path;
}

void
Location::set_cgi_path(const std::string &cgiPath) {
	_cgi_path = cgiPath;
}

bool
Location::is_autoindex() const {
	return _autoindex;
}

void
Location::set_autoindex(bool autoindex) {
	_autoindex = autoindex;
}
