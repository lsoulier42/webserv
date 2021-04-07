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

Location::Location() : AConfig() {

}

Location::Location(const Location& src) {
	*this = src;
}

Location& Location::operator=(const Location& rhs) {
	if (this != &rhs) {
		this->_root = rhs._root;
		this->_autoindex = rhs._autoindex;
		this->_index = rhs._index;
		this->_methods = rhs._methods;
		this->_cgi = rhs._cgi;
		_path = rhs._path;
	}
	return *this;
}

Location::~Location() {

}

std::string Location::getConfigType() const {
	return "Location";
}

std::string Location::getPath() const {
	return _path;
}

void Location::setPath(const std::string& path) {
	_path = path;
}
