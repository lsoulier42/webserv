/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 12:25:50 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/08 21:57:07 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

//TODO:: implementer le controle du risque d'overflow
const size_t	Request::_limit_request_line_size(8000);
const size_t	Request::_limit_header_size(8000);
const size_t	Request::_limit_headers_size(8000);

Request::Request(void) :
	_status(START),
	_str(),
	_request_line(),
	_headers(),
	_body() {}

Request::Request(const Request &x) :
	_status(x._status),
	_str(x._str),
	_request_line(x._request_line),
	_headers(x._headers),
	_body(x._body) {}

Request::~Request(void) {}

Request
&Request::operator=(const Request &x) {
	_status = x._status;
	_str = x._str;
	_request_line = x._request_line;
	_headers = x._headers;
	_body = x._body;
	return (*this);
}

const Request::RequestLine
&Request::get_request_line(void) const {
	return (_request_line);
}

const Request::Headers
&Request::get_headers(void) const {
	return (_headers);
}

const std::string
&Request::get_body(void) const {
	return (_body);
}

size_t
Request::get_limit_body_size() const {
	return _limit_body_size;
}

void
Request::set_limit_body_size(size_t size) {
	_limit_body_size = size;
}

//TODO:: char* plutot que std::string pour la reconnaissance du pattern CRLF (RFC 7230 p.20)
int
Request::append(const std::string &data) {
	_str += data;
	return (_request_parsing());
}

void
Request::reset(void) {
	_status = START;
	_request_line.reset();
	_headers.reset();
	_body.clear();
}

void
Request::render(void) const {
	std::cout << "---REQUEST LINE---" << std::endl;
	_request_line.render();
	std::cout << std::endl << "---HEADERS---" << std::endl;
	_headers.render();
	std::cout << std::endl << "---BODY---" << std::endl;
	std::cout << _body << "$" << std::endl << std::endl;
}

bool
Request::_request_line_received(void) const {
	return (_status == START && std::string::npos != _str.find("\r\n"));
}

bool
Request::_header_received(void) const {
	return (_status == REQUEST_LINE_RECEIVED
			&& !_headers_received()
			&& std::string::npos != _str.find("\r\n"));
}

bool
Request::_headers_received(void) const {
	return (_status == REQUEST_LINE_RECEIVED
			&& !_str.compare(0, 2, "\r\n"));
}

//TODO:: ameliorer : chunked devrait etre le dernier element de Transfer-Encoding
bool
Request::_body_expected(void) const {
	return ((_headers.key_exists("Transfer-Encoding")
				&& (_headers.get_value("Transfer-Encoding")).find("chunked") != std::string::npos)
			|| _headers.key_exists("Content-Length"));
}

//TODO:: ameliorer : chunked devrait etre le dernier element de Transfer-Encoding
bool
Request::_body_received(void) const {
	return (_status == HEADERS_RECEIVED
			&& ((_headers.key_exists("Transfer-Encoding")
						&& _headers.get_value("Transfer-Encoding").find("chunked") != std::string::npos
						&& _str.find("0\r\n\r\n") != std::string::npos)
				|| (_headers.key_exists("Content-Length")
						&& _str.size() >= static_cast<unsigned long>(std::atol(_headers.get_value("Content-Length").c_str())))));
}

//TODO:: tout
bool
Request::_trailer_expected(void) const {
	return (false);
}

//TODO:: tout
bool
Request::_trailer_received(void) const {
	return (true);
}

int
Request::_request_parsing(void) {
	int	ret(CONTINUE_PARSING);
	while (ret == CONTINUE_PARSING) {
		switch (_status) {
			case START :
				ret = _started_request_parsing();
				break ;
			case REQUEST_LINE_RECEIVED :
				ret = _request_line_received_parsing();
				break ;
			case HEADERS_RECEIVED :
				ret = _headers_received_parsing();
				break ;
			case BODY_RECEIVED :
				ret = _body_received_parsing();
				break ;
			case REQUEST_RECEIVED :
				ret = RECEIVED;
				break ;
		}
	}
	return (ret);
}

int
Request::_started_request_parsing(void) {
	if (_request_line_received()) {
		_status = REQUEST_LINE_RECEIVED;
		return (_collect_request_line_elements());
	}
	return (CONTINUE_READING);
}

int
Request::_request_line_received_parsing(void) {
	if (_headers_received()) {
		if (_body_expected())
			_status = HEADERS_RECEIVED;
		else
			_status = REQUEST_RECEIVED;
		return (_check_headers());
	}
	if (_header_received())
		return (_collect_header());
	return (CONTINUE_READING);
}

int
Request::_headers_received_parsing(void) {
	if (_body_received()) {
		if (_trailer_expected())
			_status = BODY_RECEIVED;
		else
			_status = REQUEST_RECEIVED;
		return (_collect_body());
	}
	return (CONTINUE_READING);
}

int
Request::_body_received_parsing(void) {
	if (_trailer_received()) {
		_status = REQUEST_RECEIVED;
		return (RECEIVED);
	}
	return (CONTINUE_READING);
}

int
Request::_collect_request_line_elements(void) {
	size_t		first_sp(0);
	size_t		scnd_sp(0);
	size_t		end_rl(_str.find("\r\n"));
	if (std::string::npos == (first_sp = _str.find_first_of(" "))
			|| std::string::npos == (scnd_sp = _str.find_first_of(" ", first_sp + 1))) {
		_str.erase(0, end_rl + 2);
		return (BAD_REQUEST);
	}
	_request_line.set_method(_str.substr(0, first_sp));
	_request_line.set_request_target(_str.substr(first_sp + 1, scnd_sp - first_sp - 1));
	_request_line.set_http_version(_str.substr(scnd_sp + 1, (end_rl - scnd_sp - 1)));
	_str.erase(0, end_rl + 2);
	if (DEFAULT_METHOD == _request_line.get_method())
		return (NOT_IMPLEMENTED);
	if (_header_received() || _headers_received())
		return (CONTINUE_PARSING);
	return (CONTINUE_READING);
}

int
Request::_collect_header(void) {
	size_t		col(0);
	size_t		end_header(_str.find("\r\n"));
	if (std::string::npos != (col = _str.find_first_of(":"))) {
		std::string	header_name(_str.substr(0, col));
		std::string	header_value(_str.substr(col + 1, (end_header - col - 1)));
		_headers.insert(header_name, header_value);
	}
	_str.erase(0, end_header + 2);
	if (_header_received() || _headers_received())
		return (CONTINUE_PARSING);
	return (CONTINUE_READING);
}

//TODO:: ameliorer
int
Request::_check_headers(void) {
	_str.erase(0, _str.find("\r\n") + 2);
	if (!_headers.key_exists("Host"))
		return (BAD_REQUEST);
	if (_status == REQUEST_RECEIVED)
		return (RECEIVED);
	if (_body_received())
		return (CONTINUE_PARSING);
	return (CONTINUE_READING);
}

//TODO:: s'assurer que chunked est le dernier element du champ Transfer-Encoding
//TODO:: decoder le body si besoin
int
Request::_collect_body(void) {
	size_t	body_length(0);
	if (_headers.key_exists("Transfer-Encoding")
			&& _headers.get_value("Transfer-Encoding").find("chunked") != std::string::npos) {
		body_length = _str.find("0\r\n\r\n") + 5;
	} else if (_headers.key_exists("Content-Length"))
		body_length = static_cast<unsigned long>(std::atol(_headers.get_value("Content-Length").c_str()));
	_body = _str.substr(0, body_length);
	_str.erase(0, body_length);
	if (_status == REQUEST_RECEIVED)
		return (RECEIVED);
	if (_trailer_received())
		return (CONTINUE_PARSING);
	return (CONTINUE_READING);
}

/*
 * REQUEST_LINE_T RELATED FUNCTIONS
 */

Request::RequestLine::RequestLine(void) :
	_method(DEFAULT_METHOD),
	_request_target(),
	_http_version() {}

Request::RequestLine::RequestLine(const RequestLine &x) :
	_method(x._method),
	_request_target(x._request_target),
	_http_version(x._http_version) {}

Request::RequestLine::~RequestLine(void) {}

Request::RequestLine
&Request::RequestLine::operator=(const RequestLine &x) {
	_method = x._method;
	_request_target = x._request_target;
	_http_version = x._http_version;
	return (*this);
}

method_t
Request::RequestLine::get_method(void) const {
	return (_method);
}

const std::string
&Request::RequestLine::get_request_target(void) const {
	return (_request_target);
}

const std::string
&Request::RequestLine::get_http_version(void) const {
	return (_http_version);
}

void
Request::RequestLine::set_method(const std::string &method_str) {
	size_t	i(0);
	while (i < DEFAULT_METHOD) {
		if (Syntax::method_tab[i].name == method_str) {
			_method = Syntax::method_tab[i].method_index;
			return ;
		}
		i++;
	}
	_method = DEFAULT_METHOD;
}

void
Request::RequestLine::set_request_target(const std::string &request_target) {
	_request_target = request_target;
}

void
Request::RequestLine::set_http_version(const std::string &http_version) {
	_http_version = http_version;
}

void
Request::RequestLine::reset(void) {
	_method = DEFAULT_METHOD;
	_request_target.clear();
	_http_version.clear();
}

void
Request::RequestLine::render(void) const {
	size_t	i(0);
	while (i < DEFAULT_METHOD) {
		if (Syntax::method_tab[i].method_index == _method) {
			std::cout << "METHOD : " << Syntax::method_tab[i].name << std::endl;
			break ;
		}
		i++;
	}
	std::cout << "REQUEST TARGET : " << _request_target << "$" << std::endl;
	std::cout << "HTTP VERSION : " << _http_version << "$" << std::endl;
}

/*
 * HEADERS_T RELATED FUNCTIONS
 */

const size_t	Request::Headers::_tab_size(30);

Request::Headers::Headers(void) :
	_tab(_tab_size) {}

Request::Headers::Headers(const Headers &x) :
	_tab(_tab_size) {
	for (size_t i(0) ; i < _tab_size ; i++)
		if (x._tab[i])
			_tab[i] = new std::list<header_t>(*(x._tab[i]));
}

Request::Headers::~Headers(void) {
	reset();
}

Request::Headers
&Request::Headers::operator=(const Headers &x) {
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
Request::Headers::reset(void) {
	for (size_t i(0) ; i < _tab_size ; i++)
		if (_tab[i]) {
			delete _tab[i];
			_tab[i] = 0;
		}
}

void
Request::Headers::render(void) const {
	for (size_t i(0) ; i < _tab_size ; i++) {
		if (_tab[i]) {
			for (std::list<header_t>::iterator it(_tab[i]->begin()) ; it != _tab[i]->end() ; it++) {
				std::cout << "*" << std::endl;
				std::cout << "KEY : " << it->key << "$" << std::endl;
				std::cout << "HASH : " << _hash(it->key.c_str()) << "$" << std::endl;
				std::cout << "VALUE : " << it->value << "$" << std::endl;
				std::cout << "*" << std::endl;
			}
		}
	}
}

//TODO:: gerer l'insertion d'un header dont la cle est deja presente
void
Request::Headers::insert(const std::string &header_name, const std::string &header_value) {
	header_t	new_entry;
	new_entry.key = header_name;
	new_entry.value = header_value;
	unsigned long	index(_hash(header_name.c_str()));
	if (!_tab[index])
		_tab[index] = new std::list<header_t>;
	(_tab[index])->push_front(new_entry);
}

bool
Request::Headers::key_exists(const std::string &key) const {
	std::list<header_t>	*entry_list(_tab[_hash(key.c_str())]);
	if (!entry_list)
		return (false);
	for (std::list<header_t>::iterator it(entry_list->begin()) ; it != entry_list->end() ; it++)
		if (it->key == key)
			return (true);
	return (false);
}

std::string
&Request::Headers::get_value(const std::string &key) throw(std::invalid_argument) {
	std::list<header_t>	*entry_list(_tab[_hash(key.c_str())]);
	if (entry_list) {
		for (std::list<header_t>::iterator it(entry_list->begin()) ; it != entry_list->end() ; it++)
			if (it->key == key)
				return (it->value);
	}
	throw (std::invalid_argument("Request::Headers::get_value : invalid argument"));
}

const std::string
&Request::Headers::get_value(const std::string &key) const throw(std::invalid_argument) {
	std::list<header_t>	*entry_list(_tab[_hash(key.c_str())]);
	if (entry_list) {
		for (std::list<header_t>::const_iterator it(entry_list->begin()) ; it != entry_list->end() ; it++)
			if (it->key == key)
				return (it->value);
	}
	throw (std::invalid_argument("Request::Headers::get_value : invalid argument"));
}

unsigned long
Request::Headers::_hash(const char *buffer) const {
	unsigned long	hash(5381);
	char			c;
	size_t			i(0);
	while ((c = buffer[i++]))
		hash = ((hash << 5) + hash) + c;
	return (hash % _tab_size);
}
