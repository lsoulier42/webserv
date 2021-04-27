/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chris <chris@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/26 13:58:05 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "RequestParsing.hpp"
#include "ResponseHandling.hpp"
#include <sys/wait.h> /* added sys/ for MAC_OS compatibility */

const size_t	Client::_buffer_size(1000000);

Client::Client(void) :
	_sd(),
	_fd(),
	_cgi_input_fd(),
	_cgi_output_fd(),
	_file_write_fd(),
	_addr(),
	_socket_len(),
	_virtual_servers(),
	_input(),
	_output(),
	_cgi_input(),
	_cgi_output(),
	_cgi_response(),
	_file_write_str(),
	_exchanges(),
	_closing(false),
	_connection_refused(false) {}

Client::Client(int sd, struct sockaddr addr, socklen_t socket_len,
	const std::list<const VirtualServer*> &virtual_servers, bool connection_refused) :
	_sd(sd),
	_fd(),
	_cgi_input_fd(),
	_cgi_output_fd(),
	_file_write_fd(),
	_addr(addr),
	_socket_len(socket_len),
	_virtual_servers(virtual_servers),
	_input(),
	_output(),
	_cgi_input(),
	_cgi_output(),
	_cgi_response(),
	_file_write_str(),
	_exchanges(),
	_closing(false),
	_connection_refused(connection_refused) {}

Client::Client(const Client &x) :
	_sd(x._sd),
	_fd(x._fd),
	_cgi_input_fd(x._cgi_input_fd),
	_cgi_output_fd(x._cgi_output_fd),
	_file_write_fd(x._file_write_fd),
	_addr(x._addr),
	_socket_len(x._socket_len),
	_virtual_servers(x._virtual_servers),
	_input(x._input),
	_output(x._output),
	_cgi_input(x._cgi_input),
	_cgi_output(x._cgi_output),
	_cgi_response(x._cgi_response),
	_file_write_str(x._file_write_str),
	_exchanges(x._exchanges),
	_closing(x._closing),
	_connection_refused(x._connection_refused) {}

Client::~Client(void) {}

Client
&Client::operator=(const Client &x) {
	if (this != &x) {
		_fd = x._fd;
		_cgi_input_fd = x._cgi_input_fd;
		_cgi_output_fd = x._cgi_output_fd;
		_file_write_fd = x._file_write_fd;
		_exchanges = x._exchanges;
		_input = x._input;
		_output = x._output;
		_cgi_input = x._cgi_input;
		_cgi_output = x._cgi_output;
		_cgi_response = x._cgi_response;
		_file_write_str = x._file_write_str;
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
Client::get_cgi_input_fd(void) const {
	return (_cgi_input_fd);
}

int
Client::get_cgi_output_fd(void) const {
	return (_cgi_output_fd);
}

int
Client::get_file_write_fd(void) const {
	return (_file_write_fd);
}

/*
 * REQUEST RECEPTION
 */

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
Client::read_socket(void) {
	char	buffer[_buffer_size];
	int		ret;

	if (_connection_refused)
		return(_process_connection_refused());
	if (0 >= (ret = read(_sd, buffer, _buffer_size))) {
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
	if (!_exchanges.empty() && _exchanges.front().first.get_status() == Request::REQUEST_RECEIVED)
		return (_process(_exchanges.front()));
	return (SUCCESS);
}

int
Client::write_socket(void) {
	size_t		output_size = _output.size();
	size_t		to_write = std::min(output_size, _buffer_size);
	ssize_t 	write_return;

	if (output_size == 0)
		return (SUCCESS);
	write_return = write(_sd, _output.c_str(), to_write);
	if (write_return < 0) {
		DEBUG_COUT("Error during writing on the socket: " << std::strerror(errno) << "(" << this->get_ident() << ")");
		_closing = true;
		return (FAILURE);
	}
	_output.pop_front(write_return);
	if (_output.empty()) {
		DEBUG_COUT("Response sent successfully (" << this->get_ident() << ")");
		_exchanges.pop_front();
		if (_closing)
			return (FAILURE);
	}
	return (SUCCESS);
}

void
Client::_rebuild_request_target(exchange_t &exchange, const std::string& path) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	std::string definite_path, dir_path, index;
	std::string request_target = request.get_request_line().get_request_target();
	std::list<std::string> index_list = request.get_location()->get_index();

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
			_format_index_path(request_target, index));
		DEBUG_COUT("Request target was directory, request target reset to : " <<
			request.get_request_line().get_request_target() << " (" << request.get_ident() << ")");
		response.set_target_path(definite_path);
	}
}

int
Client::_process(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);
	std::string	path(_build_resource_path(request));
	path_type_t path_type = Syntax::get_path_type(path);
	method_t	method = request.get_request_line().get_method();
	int (Client::*process_functions[])(exchange_t &exchange) = {&Client::_process_GET,
		&Client::_process_HEAD, &Client::_process_POST, &Client::_process_PUT,
		&Client::_process_DELETE, &Client::_process_CONNECT,
		&Client::_process_OPTIONS, &Client::_process_TRACE};

	request.set_status(Request::REQUEST_PROCESSED);
	response.get_status_line().set_http_version(OUR_HTTP_VERSION);
	if (response.get_status_line().get_status_code() != TOTAL_STATUS_CODE) {
		return (_process_error(exchange));
	}
	response.set_target_path(path);
	if (path_type == DIRECTORY && (method == GET || method == HEAD)
		&& !request.get_location()->get_index().empty()) {
		_rebuild_request_target(exchange, path);
	}
	if (_is_cgi_related(request)) {
		DEBUG_COUT("CGI request processing (" << request.get_ident() << ")");
		return (_prepare_cgi(exchange));
	}
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
	response.set_body(ByteArray(_format_autoindex_page(exchange, directory_names, file_names)));
	ResponseHandling::generate_basic_headers(exchange);
	closedir(directory);
	return (_build_output(exchange));
}

int
Client::_process_GET(exchange_t &exchange) {
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
	Request		&request(exchange.first);
	Response	&response(exchange.second);
	std::string target_path = response.get_target_path();
	path_type_t path_type = Syntax::get_path_type(target_path);
	status_code_t error_opening, status_created;

	if (path_type == DIRECTORY) {
		response.get_status_line().set_status_code(NOT_FOUND);
		return (_process_error(exchange));
	}
	error_opening = request.get_request_line().get_method() == PUT ? NOT_FOUND : INTERNAL_SERVER_ERROR;
	_file_write_fd = open(response.get_target_path().c_str(), O_WRONLY | O_CREAT| O_TRUNC| O_NONBLOCK, 0666);
	if (_file_write_fd < 0) {
		DEBUG_COUT("Error during opening PUT/POST request target :" << strerror(errno) << "(" << this->get_ident() << ")");
		response.get_status_line().set_status_code(error_opening);
		return (_process_error(exchange));
	}
	status_created = path_type == INVALID_PATH ? CREATED : NO_CONTENT;
	_file_write_str = request.get_body();
	response.get_status_line().set_status_code(status_created);
	return (SUCCESS);
}

int
Client::_process_POST(exchange_t &exchange) {
	return (_process_PUT(exchange));
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
	response.get_headers().clear();
	for (std::list<status_code_t>::iterator it(error_codes.begin()) ; it != error_codes.end() ; it++) {
		if (error_code == *it) {
			error_page_path = request.get_virtual_server()->get_error_page_path();
			response.set_target_path(error_page_path);
			return(_open_file_to_read(error_page_path));
		}
	}
	response.set_body(ByteArray(Syntax::body_error_code(error_code)));
	ResponseHandling::generate_basic_headers(exchange);
	return(_build_output(exchange));
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
	Response &response = exchange.second;
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
	char		buffer[_buffer_size];
	int			ret;

	ret = read(_fd, buffer, _buffer_size);
	if (ret < 0) {
		DEBUG_COUT("Error during reading target resource: " << std::strerror(errno) << "(" << request.get_ident() << ")");
		close(_fd);
		_closing = true;
		return (FAILURE);
	}
	if (ret == 0) {
		close(_fd);
		_fd = 0;
		if (ResponseHandling::process_response_headers(exchange) == FAILURE)
			return (_process_error(exchange));
		return (_build_output(exchange));
	}
	if (request.get_request_line().get_method() == GET)
		response.set_body(response.get_body() + ByteArray(buffer, ret));
	return (SUCCESS);
}

int
Client::write_target_resource(void) {
	exchange_t	&exchange(_exchanges.front());
	Request		&request(exchange.first);
	size_t		file_write_size = _file_write_str.size();
	size_t		to_write = std::min(file_write_size, _buffer_size);
	ssize_t 	write_return;

	if (file_write_size == 0)
		return (SUCCESS);
	write_return = write(_file_write_fd, _file_write_str.c_str(), to_write);
	if (write_return < 0) {
		DEBUG_COUT("Error during writing target resource: " << std::strerror(errno) << "(" << request.get_ident() << ")");
		close(_file_write_fd);
		_closing = true;
		return (FAILURE);
	}
	_file_write_str.pop_front(write_return);
	if (_file_write_str.empty()) {
		close(_file_write_fd);
		_file_write_fd = 0;
		if (ResponseHandling::process_response_headers(exchange) == FAILURE)
			return (_process_error(exchange));
		return (_build_output(exchange));
	}
	return (SUCCESS);
}

int
Client::_build_output(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);
	Response::StatusLine status_line = response.get_status_line();
	AHTTPMessage::HTTPHeaders& headers = response.get_headers();

	_output = ByteArray(Syntax::format_status_line(status_line.get_http_version(), status_line.get_status_code()));
	DEBUG_COUT("Generating response with status: \"" << _output.substr(0, _output.size() - 2) << "\" (" << request.get_ident() << ")");
	for(size_t i = 0; i < TOTAL_RESPONSE_HEADERS; i++) {
		if (headers.key_exists(Syntax::response_headers_tab[i].header_index)) {
			_output += Syntax::format_header_field(Syntax::response_headers_tab[i].header_index,
				headers.get_unparsed_value(Syntax::response_headers_tab[i].header_index));
		}
	}
	_output += "\r\n";
	if (request.get_request_line().get_method() != HEAD)
		_output += response.get_body();
	return (SUCCESS);
}

bool
Client::_is_cgi_related(const Request &request) {
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	path(request_target.substr(0, request_target.find('?')));
	std::string extension(request.get_location()->get_cgi_extension());
	return (path.find('.') != std::string::npos && !extension.empty()
		&& !(path.substr(path.rfind('.'))).compare(0, extension.size(), extension));
}

int
Client::_prepare_cgi(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);

	if (0 > (_cgi_input_fd = open("/tmp/cgi_input_webserver", O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, S_IRUSR | S_IWUSR))) {
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		return (_process_error(exchange));
	}
	_cgi_input = request.get_body();
	return (SUCCESS);
}

int
Client::write_cgi_input(void) {
	exchange_t	&exchange(_exchanges.front());
	Response	&response(exchange.second);
	size_t		buffer_size(std::min(_buffer_size, _cgi_input.size()));
	ssize_t		ret;

	ret = write(_cgi_input_fd, _cgi_input.c_str(), buffer_size);
	if (ret < 0) {
		close(_cgi_input_fd);
		_cgi_input_fd = 0;
		_cgi_input.clear();
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		return (_process_error(exchange));
	}
	_cgi_input.pop_front(ret);
	if (_cgi_input.empty()) {
		close(_cgi_input_fd);
		_cgi_input_fd = 0;
		return (_handle_cgi(exchange));
	}
	return (SUCCESS);
}

int
Client::_create_cgi_child_process(void) {
	pid_t	pid;

	pid = fork();
	while (pid == -1 && errno == EAGAIN)
		pid = fork();
	return (pid);
}

int
Client::_handle_cgi(exchange_t &exchange) {
	Request				&request(exchange.first);
	Response			&response(exchange.second);
	pid_t				pid;
	int					input_fd;
	int					output_fd;
	CGIMetaVariables	mv(request);
	CGIScriptArgs		args(request);

	if (0 > (input_fd = open("/tmp/cgi_input_webserver", O_RDONLY, S_IRUSR | S_IWUSR))) {
		DEBUG_COUT("Error during opening of tmp cgi input file :" << strerror(errno) << "(" << request.get_ident() << ")");
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		return (_process_error(exchange));
	}
	if (0 > (output_fd = open("/tmp/cgi_output_webserver", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR))) {
		close(input_fd);
		DEBUG_COUT("Error during opening of tmp cgi output file (for writing) :" << strerror(errno) << "(" << request.get_ident() << ")");
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		return (_process_error(exchange));
	}
	if (-1 == (pid = _create_cgi_child_process())) {
		DEBUG_COUT("Error creating child process: " << strerror(errno) << "(" << request.get_ident() << ")");
		close(input_fd);
		close(output_fd);
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		return (_process_error(exchange));
	}
	if (!pid) {
		dup2(input_fd, STDIN_FILENO);
		dup2(output_fd, STDOUT_FILENO);
		dup2(output_fd, STDERR_FILENO);
		execve(request.get_location()->get_cgi_path().c_str(), args.tab, mv.get_tab());
		write(STDOUT_FILENO, "Status: 500 Internal Server Error\n", 33);
		close(input_fd);
		close(output_fd);
		exit(EXIT_FAILURE);
	}
	close(input_fd);
	close(output_fd);
	waitpid(-1, NULL, 0);
	if (0 > (_cgi_output_fd = open("/tmp/cgi_output_webserver", O_RDONLY | O_NONBLOCK, S_IRUSR | S_IWUSR))) {
		DEBUG_COUT("Error during opening of tmp cgi output file (for reading) :" << strerror(errno) << "(" << request.get_ident() << ")");
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		return (_process_error(exchange));
	}
	DEBUG_COUT("CGI processing went well through execve (" << request.get_ident() << ")");
	_cgi_output.clear();
	_cgi_response.reset();
	return (SUCCESS);
}

int
Client::read_cgi_output(void) {
	exchange_t	&exchange(_exchanges.front());
	Request		&request(exchange.first);
	Response	&response(exchange.second);
	char		buffer[_buffer_size];
	ssize_t		ret;

	ret = read(_cgi_output_fd, buffer, _buffer_size);
	if (ret < 0) {
		DEBUG_COUT("Error during reading of CGI output: " << strerror(errno) << "(" << request.get_ident() << ")");
		close(_cgi_output_fd);
		_cgi_output_fd = 0;
		_cgi_output.clear();
		_cgi_response.reset();
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		return (_process_error(exchange));
	}
	_cgi_output.append(buffer, ret);
	//_cgi_output = (_cgi_output + ByteArray(buffer, ret));
	if (SUCCESS != _cgi_output_parsing(ret)) {
		DEBUG_COUT("Error during parsing of cgi output (" << request.get_ident() << ")");
		close(_cgi_output_fd);
		_cgi_output_fd = 0;
		_cgi_output.clear();
		_cgi_response.reset();
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		return (_process_error(exchange));
	}
	if (_cgi_response.get_status() == CGIResponse::RESPONSE_RECEIVED) {
		close(_cgi_output_fd);
		_cgi_output_fd = 0;
		DEBUG_COUT("CGI output parsing went well (" << request.get_ident() << ")");
		return (_handle_cgi_response());
	}
	return (SUCCESS);
}

int
Client::_cgi_output_parsing(int ret) {
	while (_cgi_header_received())
		_collect_cgi_header();
	if (_cgi_headers_received() && SUCCESS != _check_cgi_headers()) {
		DEBUG_COUT("Error during parsing of CGI headers (" << this->get_ident() << ")");
		return (FAILURE);
	}
	if (_cgi_body_received() || ret == 0)
		_collect_cgi_body();
	return (SUCCESS);
}

void
Client::_collect_cgi_header(void) {
	Request				&request = _exchanges.front().first;
	size_t				col(0);
	size_t				end_header(_cgi_output.find("\n"));
	header_t			current_header;

	if (ByteArray::npos != (col = _cgi_output.find_first_of(':'))) {
		DEBUG_COUT("CGI header received: \"" << _cgi_output.substr(0, end_header - 1) << "\" (" << request.get_ident() << ")");
		current_header.name = _cgi_output.substr(0, col);
		current_header.unparsed_value = Syntax::trim_whitespaces(_cgi_output.substr(col + 1, (end_header - col - 1)));
		_cgi_response.get_headers().insert(current_header);
	}
	_cgi_output.pop_front(end_header + 1);
}

int
Client::_check_cgi_headers(void) {
	_cgi_output.pop_front(_cgi_output.find("\n") + 1);
	if (_is_local_redirect_response())
		_cgi_response.set_type(CGIResponse::LOCAL_REDIRECT);
	else if (_is_client_redirect_response())
		_cgi_response.set_type(CGIResponse::CLIENT_REDIRECT);
	else if (_is_client_redirect_response_with_document())
		_cgi_response.set_type(CGIResponse::CLIENT_REDIRECT_DOC);
	else if (_is_document_response())
		_cgi_response.set_type(CGIResponse::DOCUMENT);
	else
		return (FAILURE);
	if (_cgi_body_expected())
		_cgi_response.set_status(CGIResponse::HEADERS_RECEIVED);
	else {
		_cgi_output.clear();
		_cgi_response.set_status(CGIResponse::RESPONSE_RECEIVED);
	}
	return (SUCCESS);
}

void
Client::_collect_cgi_body(void) {
	Request &request = _exchanges.front().first;
	_cgi_response.set_status(CGIResponse::RESPONSE_RECEIVED);
	DEBUG_COUT("CGI body received (" << request.get_ident() << ")");
	if (_cgi_response.get_headers().key_exists(CGI_CONTENT_LENGTH)) {
		size_t	body_size(static_cast<unsigned long>(std::atol(_cgi_response.get_headers().get_unparsed_value(CGI_CONTENT_LENGTH).c_str())));
		_cgi_response.set_body(ByteArray(_cgi_output.c_str(), body_size));
	} else
		_cgi_response.set_body(_cgi_output);
	_cgi_output.clear();
}

bool
Client::_cgi_header_received(void) {
	return (_cgi_response.get_status() == CGIResponse::START
			&& !_cgi_headers_received()
			&& ByteArray::npos != _cgi_output.find("\n"));
}

bool
Client::_cgi_headers_received(void) {
	return (_cgi_response.get_status() == CGIResponse::START
			&& _cgi_output[0] == '\n');
}

bool
Client::_cgi_body_received(void) {
	return (_cgi_response.get_status() == CGIResponse::HEADERS_RECEIVED
			&& _cgi_response.get_headers().key_exists(CGI_CONTENT_LENGTH)
			&& _cgi_output.size() >= static_cast<unsigned long>(std::atol(_cgi_response.get_headers().get_unparsed_value(CGI_CONTENT_LENGTH).c_str())));
}

bool
Client::_cgi_body_expected(void) {
	return (_cgi_response.get_type() == CGIResponse::DOCUMENT
			|| _cgi_response.get_type() == CGIResponse::CLIENT_REDIRECT_DOC);
}

bool
Client::_is_local_redirection(const std::string &location) {
	return (!location.empty()
			&& !location.compare(0, 1, "/"));
}

bool
Client::_is_client_redirection(const std::string &location) {
	return (!location.empty()
			&& std::string::npos != location.find(":")
			&& std::isalpha(location[0]));
}

bool
Client::_is_redirection_status(const std::string &status_line) {
	std::string status_code(status_line.substr(0, status_line.find(" ")));
	int			status_code_int(std::atol(status_code.c_str()));
	return (Syntax::is_redirection_code(status_code_int));
}

bool
Client::_is_document_response(void) {
	return (_cgi_response.get_headers().key_exists(CGI_CONTENT_TYPE)
			&& !_is_client_redirect_response_with_document());
}

bool
Client::_is_local_redirect_response(void) {
	return (_cgi_response.get_headers().key_exists(CGI_LOCATION)
			&& _is_local_redirection(_cgi_response.get_headers().get_unparsed_value(CGI_LOCATION))
			&& _cgi_response.get_headers().size() == 1);
}

bool
Client::_is_client_redirect_response(void) {
	return (_cgi_response.get_headers().key_exists(CGI_LOCATION)
			&& _is_client_redirection(_cgi_response.get_headers().get_unparsed_value(CGI_LOCATION))
			&& _cgi_response.get_headers().size() == 1);
}

bool
Client::_is_client_redirect_response_with_document(void) {
	return (_cgi_response.get_headers().key_exists(CGI_LOCATION)
			&& _is_client_redirection(_cgi_response.get_headers().get_unparsed_value(CGI_LOCATION))
			&& _cgi_response.get_headers().key_exists(CGI_STATUS)
			&& _is_redirection_status(_cgi_response.get_headers().get_unparsed_value(CGI_STATUS))
			&& _cgi_response.get_headers().key_exists(CGI_CONTENT_TYPE));
}

int
Client::_handle_cgi_response(void) {
	if (_cgi_response.get_type() == CGIResponse::LOCAL_REDIRECT)
		return (_handle_local_redirect_cgi_response());
	else if (_cgi_response.get_type() == CGIResponse::CLIENT_REDIRECT)
		return (_handle_client_redirect_cgi_response());
	else if (_cgi_response.get_type() == CGIResponse::CLIENT_REDIRECT_DOC)
		return (_handle_client_redirect_doc_cgi_response());
	else
		return (_handle_document_cgi_response());
}

int
Client::_handle_local_redirect_cgi_response(void) {
	return (SUCCESS);
}

int
Client::_handle_client_redirect_cgi_response(void) {
	return (SUCCESS);
}

int
Client::_handle_client_redirect_doc_cgi_response(void) {
	return (SUCCESS);
}

int
Client::_handle_document_cgi_response(void) {
	exchange_t	&exchange(_exchanges.front());
	Request		&request(exchange.first);
	Response	&response(exchange.second);

	if (_cgi_response.get_headers().key_exists(CGI_STATUS)) {
		std::string	status_line(_cgi_response.get_headers().get_unparsed_value(CGI_STATUS));
		std::string status_code(status_line.substr(0, status_line.find(" ")));
		int			status_code_int(std::atol(status_code.c_str()));
		size_t		i(0);
		while (Syntax::status_codes_tab[i].code_index != TOTAL_STATUS_CODE
			&& Syntax::status_codes_tab[i].code_int != status_code_int)
			i++;
		response.get_status_line().set_status_code(Syntax::status_codes_tab[i].code_index);
	}
	else
		response.get_status_line().set_status_code(OK);
	for (Headers::const_iterator it(_cgi_response.get_headers().begin()); it != _cgi_response.get_headers().end() ; it++)
		response.get_headers().insert(*it);
	response.set_body(_cgi_response.get_body());
	_cgi_response.reset();
	if (ResponseHandling::process_cgi_response_headers(exchange) == FAILURE) {
		DEBUG_COUT("Error during CGI response headers handling (" << request.get_ident() << ")");
		response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
		return (_process_error(exchange));
	}
	DEBUG_COUT("CGI document response handling went well (" << request.get_ident() << ")");
	return (_build_output(exchange));
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
	return (_build_output(exchange));
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
	return (_build_output(exchange));
}

int
Client::_process_TRACE(exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;

	response.get_status_line().set_status_code(OK);
	response.set_content_type("message/http");
	response.set_body(request.get_raw());
	ResponseHandling::generate_basic_headers(exchange);
	return (_build_output(exchange));
}

std::string
Client::get_target_path(void) {
	if (_exchanges.empty())
		return "";
	return _exchanges.front().second.get_target_path();
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
