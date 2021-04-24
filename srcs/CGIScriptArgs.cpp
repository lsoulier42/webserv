/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIScriptArgs.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/24 10:20:13 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/24 11:01:38 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIScriptArgs.hpp"

const size_t
CGIScriptArgs::size_tab(2);

CGIScriptArgs::CGIScriptArgs(void) :
	tab() {}

CGIScriptArgs::CGIScriptArgs(const Request &request) throw(std::bad_alloc) :
	tab(new char*[size_tab + 1]) {
	tab[0] = CGIScriptArgs::_build_script_path(request);
	tab[1] = CGIScriptArgs::_build_requested_file_path(request);
	tab[2] = 0;
}

CGIScriptArgs::CGIScriptArgs(const CGIScriptArgs &x) throw(std::bad_alloc) :
	tab(new char*[size_tab + 1]) {
	for (size_t i(0) ; i < size_tab ; i++) {
		tab[i] = new char[strlen(x.tab[i]) + 1];
		strcpy(tab[i], x.tab[i]);
	}
	tab[size_tab] = 0;
}

CGIScriptArgs::~CGIScriptArgs(void) {
	if (!tab)
		return ;
	for (size_t i(0) ; i < size_tab ; i++)
		delete[] tab[i];
	delete[] tab;
}

CGIScriptArgs
&CGIScriptArgs::operator=(const CGIScriptArgs &x) throw(std::bad_alloc) {
	if (tab) {
		for (size_t i(0) ; i < size_tab ; i++)
		delete[] tab[i];
	} else
		tab = new char*[size_tab + 1];
	for (size_t i(0) ; i < size_tab ; i++) {
		tab[i] = new char[strlen(x.tab[i]) + 1];
		strcpy(tab[i], x.tab[i]);
	}
	tab[size_tab] = 0;
	return (*this);
}

char
*CGIScriptArgs::_build_script_path(const Request &request) throw(std::bad_alloc) {
	char		*path;
	std::string	path_str(request.get_location()->get_cgi_path());
	path = new char[path_str.size() + 1];
	strcpy(path, path_str.c_str());
	return (path);
}

char
*CGIScriptArgs::_build_requested_file_path(const Request &request) throw(std::bad_alloc) {
	char		*path;
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	path_str(request_target.substr(0, request_target.find("?")));
	if (request.get_location()->get_path().compare("/"))
		path_str.erase(0, request.get_location()->get_path().size());
	path_str.insert(0, request.get_location()->get_root());
	path = new char[path_str.size() + 1];
	strcpy(path, path_str.c_str());
	return (path);
}
