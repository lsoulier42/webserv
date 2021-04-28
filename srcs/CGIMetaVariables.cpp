/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIMetaVariables.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/16 20:24:45 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/28 17:30:01 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIMetaVariables.hpp"

const size_t CGIMetaVariables::_default_size(18);
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
	&CGIMetaVariables::_build_server_software,
	&CGIMetaVariables::_build_redirect_status
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
const std::string CGIMetaVariables::_redirect_status("REDIRECT_STATUS");

CGIMetaVariables::CGIMetaVariables(void) :
	_tab() {}

CGIMetaVariables::CGIMetaVariables(const Request &request) throw(std::bad_alloc) {
	size_t	i(0);

	_size = _default_size;
	for (Headers::const_iterator it(request.get_headers().begin()) ; it != request.get_headers().end() ; it++)
		if (_is_metavariable_material(*it))
			_size++;

	_tab = new char*[_size + 1];

	for ( ; i < _default_size ; i++)
		_tab[i] = (*(_builder_tab[i]))(request);
	for (Headers::const_iterator it(request.get_headers().begin()) ; it != request.get_headers().end() ; it++)
		if (_is_metavariable_material(*it))
			_tab[i++] = _build_http_metavariable(*it);
	_tab[i] = 0;
}

CGIMetaVariables::CGIMetaVariables(const CGIMetaVariables &x) throw(std::bad_alloc) :
	_size(x._size),
	_tab(new char *[_size + 1]) {
	for (size_t i(0) ; i < _size ; i++) {
		_tab[i] = new char[strlen(x._tab[i]) + 1];
		strcpy(_tab[i], x._tab[i]);
	}
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
		delete[] _tab;
	}
	_size = x._size;
	_tab = new char*[_size + 1];
	for (size_t i(0) ; i < _size ; i++) {
		_tab[i] = new char[strlen(x._tab[i]) + 1];
		strcpy(_tab[i], x._tab[i]);
	}
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

bool
CGIMetaVariables::_is_metavariable_material(const header_t &header) {
	std::string	header_name(Syntax::str_to_lower(header.name));
	if (header_name == Syntax::str_to_lower(Syntax::headers_tab[AUTHORIZATION].name)
			|| header_name == Syntax::str_to_lower(Syntax::headers_tab[CONTENT_TYPE].name)
			|| header_name == Syntax::str_to_lower(Syntax::headers_tab[CONTENT_LENGTH].name)
			|| header_name == Syntax::str_to_lower(Syntax::headers_tab[HOST].name))
		return (false);
	return (true);
}

std::string
CGIMetaVariables::_build_http_metavariable_name(const header_t &header) {
	std::string	mv_name("HTTP_" + Syntax::str_to_upper(header.name));

	for (std::string::iterator it(mv_name.begin()) ; it != mv_name.end() ; it++)
		if (*it == '-')
			*it = '_';
	return (mv_name);
}

char
*CGIMetaVariables::_build_http_metavariable(const header_t &header) throw(std::bad_alloc) {
	char		*mv;
	std::string	mv_str(_build_http_metavariable_name(header) + "=" + header.unparsed_value);

	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
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
	std::string path(request_target.substr(0, request_target.find("?")));
	mv_str += path;
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
	std::string path(request_target.substr(0, request_target.find("?")));
	if (request.get_location()->get_path().compare("/"))
		path.erase(0, request.get_location()->get_path().size());
	mv_str += (request.get_location()->get_root() + path);
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

	if (request.get_headers().key_exists(AUTHORIZATION))
		mv_str += request.get_headers().get_value(AUTHORIZATION).back();
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
	mv_str += request.get_location()->get_cgi_path();
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

	mv_str += OUR_HTTP_VERSION;
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_server_software(const Request &request) throw(std::bad_alloc) {
	(void)request;
	char		*mv;
	std::string	mv_str(_server_software + "=");

	mv_str += PROGRAM_VERSION;
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}

char
*CGIMetaVariables::_build_redirect_status(const Request &request) throw(std::bad_alloc) {
	(void)request;
	char		*mv;
	std::string	mv_str(_redirect_status + "=");

	mv_str += "200";
	mv = new char[mv_str.size() + 1];
	strcpy(mv, mv_str.c_str());
	return (mv);
}
