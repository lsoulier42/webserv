/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AConfig.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/07 16:51:00 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/07 16:51:01 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AConfig.hpp"

AConfig::AConfig() : _autoindex(false) {

}

AConfig::~AConfig() {

}

std::string AConfig::getRoot() const {
	return _root;
}

void AConfig::setRoot(const std::string &root) {
	_root = root;
}

bool AConfig::isAutoindex() const {
	return _autoindex;
}

void AConfig::setAutoindex(bool autoindex) {
	_autoindex = autoindex;
}

std::list<std::string> AConfig::getIndex() const {
	return _index;
}

void AConfig::setIndex(const std::list<std::string> &index) {
	_index = index;
}

std::list<std::string> AConfig::getMethods() const {
	return _methods;
}

void AConfig::setMethods(const std::list<std::string> &methods) {
	_methods = methods;
}

std::string AConfig::getCgiExtension() const {
	return _cgi_extension;
}

void AConfig::setCgiExtension(const std::string &cgiExtension) {
	_cgi_extension = cgiExtension;
}

std::string AConfig::getCgiPath() const {
	return _cgi_path;
}

void AConfig::setCgiPath(const std::string &cgiPath) {
	_cgi_path = cgiPath;
}

void AConfig::showCommonConfig() const {
	std::list<std::string>::const_iterator it;
	std::string next;

	std::cout << "root: `" << this->_root << "'" << std::endl;
	std::cout << "autoindex: `" << std::string(AUTOINDEX(this->_autoindex)) << "'" << std::endl;
	std::cout << "index: `";
	it = this->_index.begin();
	while (it != this->_index.end()) {
		std::cout << *it++;
		next = it != this->_index.end() ? ", " : "'\n";
		std::cout << next;
	}
	std::cout << "methods: `";
	it = this->_methods.begin();
	while (it != this->_methods.end()) {
		std::cout << *it++;
		next = it != this->_methods.end() ? ", " : "'\n";
		std::cout << next;
	}
	std::cout << "cgi_path: `" << this->_cgi_path << "'" << std::endl;
	std::cout << "cgi_extension: `" << this->_cgi_extension << "'" << std::endl;
}
