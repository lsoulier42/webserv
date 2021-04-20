/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIMetaVariables.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/16 20:24:45 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/20 15:50:08 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIMetaVariables.hpp"

const size_t CGIMetaVariables::_size(17);
const CGIMetaVariables::_mv_builder_t CGIMetaVariables::_builder_tab[] =
{
	&CGIMetaVariables::_build_auth_type,
	&CGIMetaVariables::_build_content_length,
	&CGIMetaVariables::_build_content_type,
	&CGIMetaVariables::_build_gateway_interface,
	&CGIMetaVariables::_build_path_info,
	&CGIMetaVariables::_build_path_translated,
	&CGIMetaVariables::_build_query_string,
	&CGIMetaVariables::_build_remote_addr,
	&CGIMetaVariables::_build_remote_ident,
	&CGIMetaVariables::_build_remote_user,
	&CGIMetaVariables::_build_request_method,
	&CGIMetaVariables::_build_request_uri,
	&CGIMetaVariables::_build_script_name,
	&CGIMetaVariables::_build_server_name,
	&CGIMetaVariables::_build_server_port,
	&CGIMetaVariables::_build_server_protocol,
	&CGIMetaVariables::_build_server_software
};
const std::string CGIMetaVariables::_auth_type("AUTH_TYPE");
const std::string CGIMetaVariables::_content_length("CONTENT_LENGTH");
const std::string CGIMetaVariables::_content_type("CONTENT_TYPE");
const std::string CGIMetaVariables::_gateway_interface("GATEWAY_INTERFACE");
const std::string CGIMetaVariables::_path_info("PATH_INFO");
const std::string CGIMetaVariables::_path_translated("PATH_TRANSLATED");
const std::string CGIMetaVariables::_query_string("QUERY_STRING");
const std::string CGIMetaVariables::_remote_addr("REMOTE_ADDR");
const std::string CGIMetaVariables::_remote_ident("REMOTE_IDENT");
const std::string CGIMetaVariables::_remote_user("REMOTE_USER");
const std::string CGIMetaVariables::_request_method("REQUEST_METHOD");
const std::string CGIMetaVariables::_request_uri("REQUEST_URI");
const std::string CGIMetaVariables::_script_name("SCRIPT_NAME");
const std::string CGIMetaVariables::_server_name("SERVER_NAME");
const std::string CGIMetaVariables::_server_port("SERVER_PORT");
const std::string CGIMetaVariables::_server_protocol("SERVER_PROTOCOL");
const std::string CGIMetaVariables::_server_software("SERVER_SOFTWARE");

CGIMetaVariables::CGIMetaVariables(void) :
	_tab() {}

CGIMetaVariables::CGIMetaVariables(const Request &request) throw(std::bad_alloc) :
	_tab(new char*[_size + 1]) {
	for (size_t i(0) ; i < _size ; i++)
		_tab[i] = (*(_builder_tab[i]))(request);
	_tab[_size] = 0;
}

CGIMetaVariables::CGIMetaVariables(const CGIMetaVariables &x) throw(std::bad_alloc) :
	_tab(new char *[_size + 1]) {
	for (size_t i(0) ; i < _size ; i++)
		if (!(_tab[i] = strdup(x._tab[i])))
			throw (std::bad_alloc());
	_tab[_size] = 0;
}

CGIMetaVariables::~CGIMetaVariables(void) {
	if (!_tab)
		return ;
	for (size_t i(0) ; i < _size ; i++)
		delete[] _tab[i];
	delete[] _tab;
}

CGIMetaVariables
&CGIMetaVariables::operator=(const CGIMetaVariables &x) throw(std::bad_alloc) {
	if (_tab) {
		for (size_t i(0) ; i < _size ; i++)
			delete _tab[i];
	} else
		_tab = new char*[_size + 1];
	for (size_t i(0) ; i < _size ; i++)
		if (!(_tab[i] = strdup(x._tab[i])))
			throw (std::bad_alloc());
	_tab[_size] = 0;
	return (*this);
}

size_t
CGIMetaVariables::get_size(void) {
	return (_size);
}

char
**CGIMetaVariables::get_tab(void) const {
	return (_tab);
}

char
*CGIMetaVariables::_build_auth_type(const Request &request) throw(std::bad_alloc) {
	char		*mv;
	std::string	mv_str(_auth_type + "=");

	if (request.get_headers().key_exists(AUTHORIZATION))
		mv_str += request.get_headers().get_value(AUTHORIZATION).front();
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_content_length(const Request &request) throw(std::bad_alloc) {
	char		*mv;
	std::string	mv_str(_content_length + "=");

	if (request.get_headers().key_exists(CONTENT_LENGTH))
		mv_str += request.get_headers().get_unparsed_value(CONTENT_LENGTH);
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_content_type(const Request &request) throw(std::bad_alloc) {
	char		*mv;
	std::string	mv_str(_content_type + "=");

	if (request.get_headers().key_exists(CONTENT_TYPE))
		mv_str += request.get_headers().get_unparsed_value(CONTENT_TYPE);
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_gateway_interface(const Request &request) throw(std::bad_alloc) {
	(void)request;
	char		*mv;
	std::string	mv_str(_gateway_interface + "=");

	mv_str += "CGI/1.1";
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

//TODO:: URL-decode
char
*CGIMetaVariables::_build_path_info(const Request &request) throw(std::bad_alloc) {
	char		*mv;
	std::string	mv_str(_path_info + "=");
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	cgi_extension(request.get_location()->get_cgi_extension());
	size_t		size_extra_path(request_target.find("?") - (request_target.find(cgi_extension) + cgi_extension.size()));
	std::string	extra_path(request_target.substr(request_target.find(cgi_extension) + cgi_extension.size(), size_extra_path));

	mv_str += extra_path;
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

//TODO:: URL-decode
char
*CGIMetaVariables::_build_path_translated(const Request &request) throw(std::bad_alloc) {
	char		*mv;
	std::string	mv_str(_path_translated + "=");
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	cgi_extension(request.get_location()->get_cgi_extension());
	size_t		size_extra_path(request_target.find("?") - (request_target.find(cgi_extension) + cgi_extension.size()));
	std::string	extra_path(request_target.substr(request_target.find(cgi_extension) + cgi_extension.size(), size_extra_path));

	if (!extra_path.empty())
		mv_str += request.get_location()->get_root() + extra_path;
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_query_string(const Request &request) throw(std::bad_alloc) {
	char		*mv;
	std::string	mv_str(_query_string + "=");
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	query_string("");

	if (request_target.find("?") != std::string::npos)
		query_string = request_target.substr(request_target.find("?") + 1);
	mv_str += query_string;
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_remote_addr(const Request &request) throw(std::bad_alloc) {
	char			*mv;
	std::string		mv_str(_remote_addr + "=");
	in_addr			addr(((const struct sockaddr_in*)(&(request.get_client_addr())))->sin_addr);
	unsigned char	*bytes((unsigned char*)&addr);
	std::stringstream	ss;

	ss << (int)(bytes[0]) << "." << (int)bytes[1] << "." << (int)bytes[2] << "." << (int)bytes[3];
	mv_str += ss.str();
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_remote_ident(const Request &request) throw(std::bad_alloc) {
	(void)request;
	char		*mv;
	std::string	mv_str(_remote_ident + "=");

	mv_str += "pouet";
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_remote_user(const Request &request) throw(std::bad_alloc) {
	char		*mv;
	std::string	mv_str(_remote_user + "=");

	if (request.get_headers().key_exists(AUTHORIZATION))
		mv_str += request.get_headers().get_value(AUTHORIZATION).back();
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_request_method(const Request &request) throw(std::bad_alloc) {
	char		*mv;
	std::string	mv_str(_request_method + "=");

	mv_str += Syntax::method_tab[request.get_request_line().get_method()].name;
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_request_uri(const Request &request) throw(std::bad_alloc) {
	char		*mv;
	std::string	mv_str(_request_uri + "=");
	std::string	request_target(request.get_request_line().get_request_target());

	mv_str += request_target;
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_script_name(const Request &request) throw(std::bad_alloc) {
	char		*mv;
	std::string	mv_str(_script_name + "=");
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	cgi_extension(request.get_location()->get_cgi_extension());
	std::string	script_name(request_target.substr(0, request_target.find(cgi_extension) + cgi_extension.size()));

	mv_str += script_name;
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_server_name(const Request &request) throw(std::bad_alloc) {
	char		*mv;
	std::string	mv_str(_server_name + "=");

	if (request.get_headers().key_exists(HOST))
		mv_str += request.get_headers().get_unparsed_value(HOST);
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_server_port(const Request &request) throw(std::bad_alloc) {
	(void)request;
	char		*mv;
	std::string	mv_str(_server_port + "=");

	mv_str += request.get_virtual_server()->get_port_str();
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_server_protocol(const Request &request) throw(std::bad_alloc) {
	(void)request;
	char		*mv;
	std::string	mv_str(_server_protocol + "=");

	mv_str += "HTTP/1.1";
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_server_software(const Request &request) throw(std::bad_alloc) {
	(void)request;
	char		*mv;
	std::string	mv_str(_server_software + "=");

	mv_str += "WebServer/1.0";
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}
