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
