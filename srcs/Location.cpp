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
		this->_cgi_path = rhs._cgi_path;
		this->_cgi_extension = rhs._cgi_extension;
		_path = rhs._path;
	}
	return *this;
}

Location::~Location() {

}

void Location::showConfig() const {
	std::cout << "Location configuration: " << std::endl;
	std::cout << "path: `" << _path << "'" << std::endl;
	this->showCommonConfig();
	std::cout << "End of location configuration" << std::endl;
}

std::string Location::getPath() const {
	return _path;
}

void Location::setPath(const std::string& path) {
	_path = path;
}
