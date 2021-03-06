/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chris <chris@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/05/01 11:01:29 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "RequestParsing.hpp"
#include "ResponseHandling.hpp"
#include "CGI.hpp"
#include "WebServer.hpp"

Client::Client(void) :
	_sd(),
	_fd(),
	_cgi_output_fd(),
	_addr(),
	_socket_len(),
	_virtual_servers(),
	_input(),
	_cgi_output(),
	_cgi_response(),
	_exchanges(),
	_closing(false),
	_connection_refused(false) {}

Client::Client(int sd, struct sockaddr addr, socklen_t socket_len,
	const std::list<const VirtualServer*> &virtual_servers, bool connection_refused) :
	_sd(sd),
	_fd(),
	_cgi_output_fd(),
	_addr(addr),
	_socket_len(socket_len),
	_virtual_servers(virtual_servers),
	_input(),
	_cgi_output(),
	_cgi_response(),
	_exchanges(),
	_closing(false),
	_connection_refused(connection_refused) {}

Client::Client(const Client &x) :
	_sd(x._sd),
	_fd(x._fd),
	_cgi_output_fd(x._cgi_output_fd),
	_addr(x._addr),
	_socket_len(x._socket_len),
	_virtual_servers(x._virtual_servers),
	_input(x._input),
	_cgi_output(x._cgi_output),
	_cgi_response(x._cgi_response),
	_exchanges(x._exchanges),
	_closing(x._closing),
	_connection_refused(x._connection_refused) {}

Client::~Client(void) {}

Client
&Client::operator=(const Client &x) {
	if (this != &x) {
		_fd = x._fd;
		_cgi_output_fd = x._cgi_output_fd;
		_exchanges = x._exchanges;
		_input = x._input;
		_cgi_output = x._cgi_output;
		_cgi_response = x._cgi_response;
		_closing = x._closing;
		_connection_refused = x._connection_refused;
	}
	return (*this);
}

int
Client::get_sd(void) const {
	return (_sd);
}

int
Client::get_fd(void) const {
	return (_fd);
}

int
Client::get_cgi_output_fd(void) const {
	return (_cgi_output_fd);
}

/*
 * REQUEST RECEPTION
 */

int
Client::read_socket(void) {
	char	buffer[WebServer::buffer_size];
	int		ret;

	if (_connection_refused)
		return(_process_connection_refused());
	if (0 >= (ret = read(_sd, buffer, WebServer::buffer_size))) {
		if (0 == ret) {
			DEBUG_COUT("Client closed the connection" << "(" << this->get_ident() << ")");
		}
		else
			DEBUG_COUT("Error during reading the socket: " << std::strerror(errno) << "(" << this->get_ident() << ")");
		_closing = true;
		return (FAILURE);
	}
	_input.append(buffer, ret);
	RequestParsing::parsing(*this);
	return (SUCCESS);
}

int
Client::write_socket(void) {
	if (_exchanges.empty())
		return (SUCCESS);

	exchange_t	&exchange(_exchanges.front());
	Request		&request(exchange.first);
	Response	&response(exchange.second);

	/* Sending the head (status line + response headers)
	 */

	if (response.get_status() == Response::START && !response.get_head().empty()) {

		size_t		buffer_size(std::min(response.get_head().size(), WebServer::buffer_size));
		ssize_t		ret;

		ret = write(_sd, response.get_head().c_str(), buffer_size);
		if (ret < 0) {
			DEBUG_COUT("Error during writing on the socket: " << std::strerror(errno) << "(" << this->get_ident() << ")");
			return (FAILURE);
		}

		response.get_head().pop_front(ret);
		if (response.get_head().empty()) {

			if (request.get_request_line().get_method() != HEAD && (response.get_length() > 0 || response.get_chunked())) {
				response.set_status(Response::HEAD_SENT);
			} else {
				response.set_status(Response::RESPONSE_SENT);
				DEBUG_COUT("Response sent successfully (" << this->get_ident() << ")");
				_exchanges.pop_front();
				if (_closing)
					return (FAILURE);
			}

		}

	/* Sending the content
	 */

	} else if (response.get_status() == Response::HEAD_SENT && !response.get_content().empty()) {

		size_t		buffer_size(std::min(response.get_content().size(), WebServer::buffer_size));
		ssize_t		ret;

		ret = write(_sd, response.get_content().c_str(), buffer_size);
		if (ret < 0) {
			DEBUG_COUT("Error during writing on the socket: " << std::strerror(errno) << "(" << this->get_ident() << ")");
			return (FAILURE);
		}

		response.get_content().pop_front(ret);
		if (response.get_content().empty()
				&& ((response.get_chunked() && response.get_sending_indicator() == 0)
					|| (!response.get_chunked() && response.get_sending_indicator() >= response.get_length()))) {
			response.set_status(Response::RESPONSE_SENT);
			DEBUG_COUT("Response sent successfully (" << this->get_ident() << ")");
			_exchanges.pop_front();
			if (_closing)
				return (FAILURE);
		}

	}

	return (SUCCESS);
}

void
Client::_rebuild_request_target(exchange_t &exchange, const std::string& path) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	std::string definite_path, dir_path, index;
	std::string request_target = request.get_request_line().get_request_target();
	std::string query_string;
	std::list<std::string> index_list = request.get_location()->get_index();
	size_t qs_pos = request_target.find('?');

	if (qs_pos != std::string::npos) {
		query_string = request_target.substr(qs_pos);
		request_target = request_target.substr(0, qs_pos);
	}
	for(std::list<std::string>::iterator it = index_list.begin();
		it != index_list.end(); it++) {
		definite_path = _format_index_path(path, *it);
		if (REGULAR_FILE == Syntax::get_path_type(definite_path)) {
			index = *it;
			break;
		}
	}
	if (Syntax::get_path_type(definite_path) != INVALID_PATH) {
		request.get_request_line().set_request_target(
			_format_index_path(request_target, index) + query_string);
		DEBUG_COUT("Request target was a directory, request target reset to : " <<
			request.get_request_line().get_request_target() << " (" << request.get_ident() << ")");
		response.set_target_path(definite_path);
	}
}

int
Client::process(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);
	std::string	path(_build_resource_path(request));
	path_type_t path_type = Syntax::get_path_type(path);
	method_t	method = request.get_request_line().get_method();
	int (Client::*process_functions[])(exchange_t &exchange) = {&Client::_process_GET,
		&Client::_process_HEAD, &Client::_process_POST, &Client::_process_PUT,
		&Client::_process_DELETE, &Client::_process_CONNECT,
		&Client::_process_OPTIONS, &Client::_process_TRACE};

	if (request.body_is_expected() && request.get_body_size_received() != request.get_tmp_file_size())
		return (TMP_FILE_NOT_DONE_WRITING);
	request.set_status(Request::REQUEST_PROCESSED);
	response.get_status_line().set_http_version(OUR_HTTP_VERSION);
	if (response.get_status_line().get_status_code() != TOTAL_STATUS_CODE)
		return (_process_error(exchange));
	response.set_target_path(path);
	if (path_type == DIRECTORY && (method == GET || method == HEAD)
		&& !request.get_location()->get_index().empty())
		_rebuild_request_target(exchange, path);
	if (request.body_is_expected() && _check_tmp_file(exchange) == FAILURE)
		return (_process_error(exchange));
	if (CGI::is_cgi_related(request))
		return (CGI::init_CGI(*this));
	DEBUG_COUT("Non CGI request processing (" << request.get_ident() << ")");
	return ((this->*process_functions[request.get_request_line().get_method()])(exchange));
}

std::string
Client::_format_index_path(const std::string& dir_path, const std::string& index_file) {
	std::string definite_path = dir_path;
	std::string definite_index_file = index_file;

	Syntax::format_directory_name(definite_path);
	if (*index_file.begin() == '/')
		definite_index_file = index_file.substr(1);
	return definite_path + definite_index_file;
}

std::string
Client::_format_autoindex_page(exchange_t& exchange, const std::set<std::string>& directory_names,
	const std::set<std::string>& file_names) {
	std::stringstream ss;
	std::string		target_path = exchange.second.get_target_path();
	std::string		request_target = exchange.first.get_request_line().get_request_target();
	std::string		dir_name(request_target.substr(0, request_target.find('?')));

	ss << "<html>" << std::endl << "<head>" << std::endl;
	ss << "<title>Index of " << dir_name << " </title></head>" << std::endl;
	ss << "<body bgcolor=\"white\">" << std::endl;
	ss << "<h1>Index of " << dir_name << "</h1><hr><pre>";
	ss << "<table><tr><th>Name</th><th>Last modification</th><th>Size</th></tr>";
	for (std::set<std::string>::iterator it = directory_names.begin(); it != directory_names.end(); it++)
		_format_autoindex_entry(ss, *it, exchange, true);
	for (std::set<std::string>::iterator it = file_names.begin(); it != file_names.end(); it++)
		_format_autoindex_entry(ss, *it, exchange, false);
	ss << "</table></pre><hr></body>" << std::endl << "</html>" << std::endl;
	return ss.str();
}

void
Client::_format_autoindex_entry(std::stringstream& ss, const std::string& filename,
	exchange_t& exchange, bool is_dir) {
	std::string		definite_filename, fullpath, target_path = exchange.second.get_target_path();
	std::string		request_target = exchange.first.get_request_line().get_request_target();
	struct stat		stat_buf;
	char 			time_buf[64];
	time_t			last_modification;
	struct tm		*tm;

	Syntax::format_directory_name(target_path);
	fullpath = target_path + filename;
	if (stat(fullpath.c_str(), &stat_buf) == -1)
		return;
	last_modification = stat_buf.st_mtim.tv_sec;
	tm = localtime(&last_modification);
	strftime(time_buf, sizeof(time_buf), "%d-%b-%Y %H:%M", tm);
	definite_filename = filename;
	if (is_dir)
		Syntax::format_directory_name(definite_filename);
	Syntax::format_directory_name(request_target);
	ss << "<tr><td><a href=\"" << request_target + filename << "\">";
	ss << definite_filename << "</a></td>";
	ss << "<td>--" << time_buf << "--</td>";
	ss << "<td>";
	if (is_dir)
		ss << "-";
	else
		ss << stat_buf.st_size;
	ss << "</td></tr>" << std::endl;
}

int
Client::_generate_autoindex(exchange_t &exchange) {
	Response		&response(exchange.second);
	DIR       		*directory;
	std::set<std::string> directory_names, file_names;
	struct dirent	*file_listing;

	directory = opendir(response.get_target_path().c_str());
	if (!directory) {
		response.get_status_line().set_status_code(FORBIDDEN);
		return (FAILURE);
	}
	response.get_status_line().set_status_code(OK);
	while ((file_listing = readdir(directory))) {
		if (file_listing->d_type == DT_DIR
			&& strcmp(file_listing->d_name, ".") == 0)
			continue;
		if (file_listing->d_type == DT_DIR)
			directory_names.insert(file_listing->d_name);
		else if (file_listing->d_type == DT_REG)
			file_names.insert(file_listing->d_name);
	}
	response.set_content(ByteArray(_format_autoindex_page(exchange, directory_names, file_names)));
	response.set_length(response.get_content().size());
	response.set_sending_indicator(response.get_length());
	ResponseHandling::generate_basic_headers(exchange);
	_build_head_response(exchange);
	closedir(directory);
	return (SUCCESS);
}

int
Client::_process_GET(exchange_t &exchange) {
	if (_fd != 0)
		return (SUCCESS);

	Request		&request(exchange.first);
	Response	&response(exchange.second);
	std::string target_path = response.get_target_path();
	path_type_t path_type = Syntax::get_path_type(target_path);

	if (path_type == INVALID_PATH) {
		response.get_status_line().set_status_code(NOT_FOUND);
		return (_process_error(exchange));
	}
	if (path_type == DIRECTORY) {
		if (request.get_location()->is_autoindex()) {
			DEBUG_COUT("No index found, generating autoindex (" << request.get_ident() << ")");
			return (_generate_autoindex(exchange));
		}
		response.get_status_line().set_status_code(NOT_FOUND);
		return (_process_error(exchange));
	}
	response.get_status_line().set_status_code(OK);
	return (_open_file_to_read(target_path));
}

int
Client::_process_PUT(exchange_t &exchange) {
	Request				&request(exchange.first);
	Response			&response(exchange.second);
	std::string 		target_path = response.get_target_path();
	std::string			tmp_filename = request.get_tmp_filename();
	std::string			filename = target_path.substr(target_path.rfind('/') + 1);
	std::string			upload_dir = request.get_location()->get_upload_dir();
	path_type_t 		path_type = Syntax::get_path_type(target_path);
	status_code_t		status_created;

	if (path_type == DIRECTORY) {
		response.get_status_line().set_status_code(NOT_FOUND);
		unlink(tmp_filename.c_str());
		return (_process_error(exchange));
	}
	if (upload_dir.empty())
		upload_dir = target_path.substr(0, target_path.rfind('/'));
	Syntax::format_directory_name(upload_dir);
	target_path = upload_dir + filename;
	if (rename(tmp_filename.c_str(), target_path.c_str()) == -1) {
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		DEBUG_COUT("Error during renaming of tmp file : " << strerror(errno) << " (" << request.get_ident() << ")");
		unlink(tmp_filename.c_str());
		return (_process_error(exchange));
	}
	status_created = path_type == INVALID_PATH ? CREATED : NO_CONTENT;
	response.get_status_line().set_status_code(status_created);
	ResponseHandling::process_response_headers(exchange);
	_build_head_response(exchange);
	return (SUCCESS);
}

int
Client::_process_POST(exchange_t &exchange) {
	Request		&request = exchange.first;
	Response	&response = exchange.second;
	std::string	tmp_filename = request.get_tmp_filename();

	if (!tmp_filename.empty())
		unlink(tmp_filename.c_str());
	response.get_status_line().set_status_code(NO_CONTENT);
	ResponseHandling::generate_basic_headers(exchange);
	_build_head_response(exchange);
	return (SUCCESS);
}

int
Client::_process_error(exchange_t &exchange) {
	Request						&request(exchange.first);
	Response					&response(exchange.second);
	status_code_t				error_code(response.get_status_line().get_status_code());
	std::string					error_page_path;
	std::list<status_code_t>	error_codes(request.get_virtual_server()->get_error_page_codes());
	std::stringstream 			ss;

	ss << "Request failed with error: " << Syntax::status_codes_tab[error_code].code_str;
	ss << " - " << Syntax::status_codes_tab[error_code].reason_phrase << "(";
	ss << request.get_ident() << ")";
	DEBUG_COUT(ss.str());
	if (!request.get_tmp_filename().empty())
		unlink(request.get_tmp_filename().c_str());
	response.get_headers().clear();
	for (std::list<status_code_t>::iterator it(error_codes.begin()) ; it != error_codes.end() ; it++) {
		if (error_code == *it) {
			error_page_path = request.get_virtual_server()->get_error_page_path();
			response.set_target_path(error_page_path);
			return(_open_file_to_read(error_page_path));
		}
	}

	/* Response content is :
	 * + not chunked
	 * + complete (sending_indicator == length)
	 */
	response.set_content(ByteArray(Syntax::body_error_code(error_code)));
	response.set_chunked(false);
	response.set_length(response.get_content().size());
	response.set_sending_indicator(response.get_length());

	ResponseHandling::generate_basic_headers(exchange);
	_build_head_response(exchange);

	return (SUCCESS);
}

std::string
Client::_build_resource_path(Request &request) {
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	location_root(request.get_location()->get_root());
	std::string location_path(request.get_location()->get_path());

	request_target = request_target.substr(0, request_target.find('?'));
	request_target.erase(0, location_path.size());
	Syntax::format_directory_name(location_root);
	Syntax::trail_begining_backslash(request_target);
	return (location_root + request_target);
}

int
Client::_open_file_to_read(const std::string &path) {
	exchange_t &exchange = _exchanges.front();
	Request &request = exchange.first;
	Response &response = exchange.second;

	response.set_chunked(true);
	ResponseHandling::process_response_headers(exchange);
	_build_head_response(exchange);

	if (request.get_request_line().get_method() == HEAD)
		return (SUCCESS);

	if (0 > (_fd = open(path.c_str(), O_RDONLY | O_NONBLOCK))) {
		DEBUG_COUT("Error during opening a file: " << std::strerror(errno) << "(" << this->get_ident() << ")");
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		return (_process_error(_exchanges.front()));
	}

	return (SUCCESS);
}

int
Client::read_target_resource(void) {
	exchange_t	&exchange(_exchanges.front());
	Request		&request(exchange.first);
	Response	&response(exchange.second);
	char		buffer[WebServer::buffer_size];
	ssize_t 	ret;

	ret = read(_fd, buffer, WebServer::buffer_size);
	if (ret < 0) {
		DEBUG_COUT("Error during reading target resource: " << std::strerror(errno) << "(" << request.get_ident() << ")");
		close(_fd);
		_closing = true;
		return (FAILURE);
	}
	response.append_content_chunk(buffer, ret);
	if (ret == 0) {
		close(_fd);
		_fd = 0;
	}
	return (SUCCESS);
}

/* _buil_head_response()
 * creates the ByteArray containing the response's head (status line + response headers),
 * must be called after the creation of response headers by ResponseHandling::
 */

void
Client::_build_head_response(exchange_t &exchange) {
	Request						&request(exchange.first);
	Response					&response(exchange.second);
	std::string					new_header;
	Response::StatusLine		status_line(response.get_status_line());
	AHTTPMessage::HTTPHeaders	&headers(response.get_headers());
	ByteArray					&head(response.get_head());

	head = ByteArray(Syntax::format_status_line(status_line.get_http_version(), status_line.get_status_code()));
	DEBUG_COUT("Generating response with status: \"" << head.substr(0, head.size() - 2) << "\" (" << request.get_ident() << ")");
	for(size_t i = 0; i < TOTAL_RESPONSE_HEADERS; i++) {
		if (headers.key_exists(Syntax::response_headers_tab[i].header_index)) {
			new_header = Syntax::format_header_field(Syntax::response_headers_tab[i].header_index,
				headers.get_unparsed_value(Syntax::response_headers_tab[i].header_index));
			head += new_header;
			DEBUG_COUT("Response header created \"" << new_header.substr(0, new_header.size() - 2) << "\" (" << request.get_ident() << ")");
		}
	}
	head += "\r\n";
}

int
Client::_process_HEAD(exchange_t &exchange) {
	return (_process_GET(exchange));
}

int
Client::_process_DELETE(exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	std::string path(_build_resource_path(request));
	path_type_t path_type = Syntax::get_path_type(path);

	if (path_type == INVALID_PATH) {
		response.get_status_line().set_status_code(NOT_FOUND);
		return (_process_error(exchange));
	}
	if (unlink(path.c_str()) == -1) {
		response.get_status_line().set_status_code(FORBIDDEN);
		return (_process_error(exchange));
	}
	response.get_status_line().set_status_code(NO_CONTENT);
	ResponseHandling::generate_basic_headers(exchange);
	_build_head_response(exchange);
	return (SUCCESS);
}

int
Client::_process_CONNECT(exchange_t &exchange) {
	Response& response = exchange.second;

	response.get_status_line().set_status_code(NOT_IMPLEMENTED);
	return (_process_error(exchange));
}

int
Client::_process_OPTIONS(exchange_t &exchange) {
	Response& response = exchange.second;

	response.get_status_line().set_status_code(OK);
	response.set_content_type(Syntax::mime_types_tab[TEXT_HTML].name);
	ResponseHandling::generate_basic_headers(exchange);
	_build_head_response(exchange);
	return (SUCCESS);
}

int
Client::_process_TRACE(exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;

	response.get_status_line().set_status_code(OK);
	response.set_content_type("message/http");
	response.set_content(request.get_raw());
	response.set_length(response.get_content().size());
	response.set_sending_indicator(response.get_length());
	ResponseHandling::generate_basic_headers(exchange);
	_build_head_response(exchange);
	return (SUCCESS);
}

int
Client::_process_connection_refused() {
	exchange_t refused_exchange = std::make_pair(Request(*this), Response());
	refused_exchange.second.get_status_line().set_http_version(OUR_HTTP_VERSION);
	refused_exchange.second.get_status_line().set_status_code(SERVICE_UNAVAILABLE);
	_closing = true;
	_process_error(refused_exchange);
	return (FAILURE);
}

int
Client::read_cgi_output(void) {
	exchange_t	&exchange(_exchanges.front());
	Response	&response(exchange.second);
	int			ret;

	ret = CGI::read_output(*this);
	if (CGI::SERVER_ERROR == ret) {
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		return (_process_error(exchange));
	}
	else if (CGI::SCRIPT_ERROR == ret) {
		response.get_status_line().set_status_code(BAD_GATEWAY);
		return (_process_error(exchange));
	}
   	else if (CGI::HEAD_COMPLETE == ret)
		_build_head_response(exchange);
	else if (CGI::REDIRECT == ret)
		return (process(exchange));
	return (SUCCESS);
}

char*
Client::get_ip_addr() const {
	struct sockaddr not_const = _addr;
	struct sockaddr_in *client_addr_cast;

	client_addr_cast = reinterpret_cast<struct sockaddr_in*>(&not_const);
	return (inet_ntoa(client_addr_cast->sin_addr));
}

std::string
Client::get_ident() const {
	std::stringstream ss;

	ss << get_ip_addr() << "[client no:" << _sd << "]";
	return (ss.str());
}

std::list<Client::exchange_t>&
Client::get_exchanges(void) {
	return _exchanges;
}

int
Client::_check_tmp_file(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);
	struct stat buf;

	if (stat(request.get_tmp_filename().c_str(), &buf) == -1) {
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		DEBUG_COUT("Error retrieving tmp file " << request.get_tmp_filename() << " (" << request.get_ident() << ")");
		return (FAILURE);
	}
	if (buf.st_size > static_cast<long>(request.get_location()->get_client_max_body_size())) {
		response.get_status_line().set_status_code(PAYLOAD_TOO_LARGE);
		return (FAILURE);
	}
	return (SUCCESS);
}
