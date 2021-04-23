/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chris <chris@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/22 21:21:26 by chris            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "RequestParsing.hpp"
#include "ResponseHandling.hpp"
#include "WebServer.hpp"
#include <sys/wait.h> /* added sys/ for MAC_OS compatibility */

const size_t	Client::_buffer_size(8192);

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
	refused_exchange.first.set_compromising(true);
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
		if (0 == ret)
			std::cerr << "the client closed the connection." << std::endl;
		else
			std::cerr << "error during reading the socket: " << strerror(errno) << std::endl;
		return (SUCCESS);
	}
	_input.append(buffer, ret);
	RequestParsing::parsing(*this);
	if (!_exchanges.empty() && _exchanges.front().first.get_status() == Request::REQUEST_RECEIVED)
		return (_process(_exchanges.front()));
	return (SUCCESS);
}

int
Client::write_socket(void) throw(ClientError) {
	size_t		to_write, output_size = _output.size();
	ssize_t 	write_return;

	if (output_size == 0)
		return (SUCCESS);
	to_write = output_size > _buffer_size ? _buffer_size : output_size;
	write_return = write(_sd, _output.c_str(), to_write);
	_output.pop_front(write_return);
	if (_output.empty()) {
		status_code_t status_code = _exchanges.front().second.get_status_line().get_status_code();
		if (status_code >= BAD_REQUEST)
			throw (ClientError(status_code));
		_exchanges.pop_front();
	}
	return (SUCCESS);
}

/*
 * RESPONSE SENDING
 */

int
Client::_process(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);
	int (Client::*process_functions[])(exchange_t &exchange) = {&Client::_process_GET,
		&Client::_process_HEAD, &Client::_process_POST, &Client::_process_PUT,
		&Client::_process_DELETE, &Client::_process_CONNECT,
		&Client::_process_OPTIONS, &Client::_process_TRACE};

	request.set_status(Request::REQUEST_PROCESSED);
	response.get_status_line().set_http_version(OUR_HTTP_VERSION);
	if (response.get_status_line().get_status_code() != TOTAL_STATUS_CODE)
		return (_process_error(exchange));
	if (_is_cgi_related(request))
		return (_handle_cgi(exchange));
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

int
Client::_get_default_index(exchange_t &exchange) {
	Request& request = exchange.first;
	Response& response = exchange.second;
	std::string dir_path = response.get_target_path(), definite_path;
	std::list<std::string> index_list = request.get_location()->get_index();

	if (index_list.empty()) {
		definite_path = _format_index_path(dir_path, "index.html");
	} else {
		for(std::list<std::string>::iterator it = index_list.begin(); it != index_list.end(); it++) {
			definite_path = _format_index_path(dir_path, *it);
			if (REGULAR_FILE == Syntax::get_path_type(definite_path))
				break ;
		}
	}
	if (Syntax::get_path_type(definite_path) == INVALID_PATH)
		return (FAILURE);
	response.set_target_path(definite_path);
	return (SUCCESS);
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

	std::string	path(_build_resource_path(request));
	path_type_t path_type = Syntax::get_path_type(path);

	if (path_type == INVALID_PATH) {
		response.get_status_line().set_status_code(NOT_FOUND);
		return (_process_error(exchange));
	}
	response.set_target_path(path);
	if (path_type == DIRECTORY) {
		if (_get_default_index(exchange) == FAILURE) {
			if (request.get_location()->is_autoindex())
				return (_generate_autoindex(exchange));
			response.get_status_line().set_status_code(FORBIDDEN);
			return (_process_error(exchange));
		}
	}
	response.get_status_line().set_status_code(OK);
	return (_open_file_to_read(response.get_target_path()));
}


/* processing elements needed for performing the PUT request: creating/opening the file demanded
through URI. Actual writing will take place in Client::write_file function which will be called in
WebServer::write_socks function */
int
Client::_process_PUT(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);

	std::string path(_build_resource_path(request));
	path_type_t path_type = Syntax::get_path_type(path);

	DEBUG_COUT("process PUT entered");
	/* we do not support creating a directory through put */
	if (path_type == DIRECTORY) {
		response.get_status_line().set_status_code(NOT_FOUND);
		return (_process_error(exchange));
	}
	response.set_target_path(path);
	/* if the path is invalid, it means there are no file, thus we try to create the file*/
	if (path_type == INVALID_PATH) {
		/* setting as non blocking, not using open_file_to_read or modifying it as would create to many
		options for just a few lines */
		_file_write_fd = open(response.get_target_path().c_str(), O_CREAT|O_WRONLY|O_NONBLOCK, 0666);
		if (_file_write_fd < 0) {
			std::cerr << "error during opening a file :";
			std::cerr << strerror(errno) << std::endl;
			response.get_status_line().set_status_code(NOT_FOUND);
			return (_process_error(exchange));
		}
		response.get_status_line().set_status_code(CREATED);
		return (_file_write_fd);
	}
	else { // path_type == REGULAR_FILE
		_file_write_fd = open(response.get_target_path().c_str(), O_WRONLY|O_NONBLOCK, 0666);
		if (_file_write_fd < 0) {
			std::cerr << "error during opening a file :";
			std::cerr << strerror(errno) << std::endl;
			response.get_status_line().set_status_code(NOT_FOUND);
			return (_process_error(exchange));
		}
		/* If the target resource does have a current representation and that representation is successfully
		modified in accordance with the state of the enclosed representation, then the origin server must send
		either a 200 (OK) or a 204 (No Content) response to indicate successful completion of the request. */
		if (!request.get_body().empty()) {
			response.get_status_line().set_status_code(OK);
			_file_write_str = request.get_body();
		}
		else {
			response.get_status_line().set_status_code(NO_CONTENT);
			_file_write_str.clear();
		}
		return (_file_write_fd);
	}
}

/* implementation for NON-CGI POST calls follows _process_PUT. Error codes differ */
int
Client::_process_POST(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);

	std::string path(_build_resource_path(request));
	path_type_t path_type = Syntax::get_path_type(path);

	DEBUG_COUT("process POST entered");
	if (path_type == DIRECTORY) {
		response.get_status_line().set_status_code(NOT_FOUND);
		return (_process_error(exchange));
	}
	response.set_target_path(path);
	/* if the path is invalid, it means there are no file, thus we try to create the file*/
	if (path_type == INVALID_PATH) {
		/* setting as non blocking, not using open_file_to_read or modifying it as would create to many
		options for just a few lines */
		_file_write_fd = open(response.get_target_path().c_str(), O_CREAT|O_WRONLY|O_NONBLOCK, 0666);
		if (_file_write_fd < 0) {
			std::cerr << "error during opening a file :";
			std::cerr << strerror(errno) << std::endl;
			response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
			return (_process_error(exchange));
		}
		response.get_status_line().set_status_code(CREATED);
		return (_file_write_fd);
	}
	else { // path_type == REGULAR_FILE
		_file_write_fd = open(response.get_target_path().c_str(), O_WRONLY|O_NONBLOCK, 0666);
		if (_file_write_fd < 0) {
			std::cerr << "error during opening a file :";
			std::cerr << strerror(errno) << std::endl;
			response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
			return (_process_error(exchange));
		}
		/* If the target resource does have a current representation and that representation is successfully
		modified in accordance with the state of the enclosed representation, then the origin server must send
		either a 200 (OK) or a 204 (No Content) response to indicate successful completion of the request. */
		if (!request.get_body().empty()) {
			response.get_status_line().set_status_code(OK);
			_file_write_str = request.get_body();
		}
		else {
			response.get_status_line().set_status_code(NO_CONTENT);
			_file_write_str.clear();
		}
		return (_file_write_fd);
	}
}

int
Client::_process_error(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);
	status_code_t error_code = response.get_status_line().get_status_code();
	std::string error_page_path;
	std::list<status_code_t>	error_codes(request.get_virtual_server()->get_error_page_codes());

	DEBUG_COUT(std::endl << "_process_error entered");
	DEBUG_COUT("---> Error status code sent: " << 
	Syntax::status_codes_tab[response.get_status_line().get_status_code()].code_str);
	response.get_headers().clear();
	if (!request.get_location())
		request.set_location(&request.get_virtual_server()->get_locations().back());
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
	if (0 > (_fd = open(path.c_str(), O_RDONLY))) {
		std::cerr << "error during opening a file: ";
		std::cerr << strerror(errno) << std::endl;
		return (FAILURE);
	}
	WebServer::set_non_blocking(_fd);
	return (SUCCESS);
}

/* cchenot : function name can be a bit misleading, here we read the file through _fd processed by _process_GET;
file is read to build _out_put_str to be sent to client as part of HTTP response */
int
Client::read_file(void) throw(ClientError) {
	exchange_t	&exchange(_exchanges.front());
	Request		&request(exchange.first);
	Response	&response(exchange.second);
	char		buffer[_buffer_size];
	int			ret;

	ret = read(_fd, buffer, _buffer_size);
	if (ret < 0) {
		close(_fd);
		throw(ClientError(INTERNAL_SERVER_ERROR));
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
Client::write_file(void) throw(ClientError) {
	exchange_t	&exchange(_exchanges.front());
	size_t		to_write, file_write_size = _file_write_str.size();
	ssize_t 	write_return;

	if (file_write_size == 0)
		return (SUCCESS);
	to_write = file_write_size > _buffer_size ? _buffer_size : file_write_size;
	write_return = write(_file_write_fd, _file_write_str.c_str(), to_write);
	if (write_return < 0) {
		close(_file_write_fd);
		throw(ClientError(INTERNAL_SERVER_ERROR));
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

void
Client::_send_debug_str(const std::string& str) const {
	std::string to_send = str + "\n";
	size_t size = to_send.size();

	send(_sd, to_send.c_str(), size, 0);
}

bool
Client::_is_cgi_related(const Request &request) const {
	std::string	path(_build_cgi_script_path(request));
	return (path.find(".") != std::string::npos
				&& path.substr(path.rfind(".")) == request.get_location()->get_cgi_extension());
}

std::string
Client::_build_cgi_script_path(const Request &request) const {
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	cgi_extension(request.get_location()->get_cgi_extension());
	std::string	path(request_target.substr(0, request_target.find(cgi_extension) + cgi_extension.size()));
	return (request.get_location()->get_root() + path);
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
	CGIMetaVariables	mv(request);
	pid_t				pid;
	int					req_pipe[2];
	int					res_pipe[2];
	std::string			arg0_str(request.get_location()->get_cgi_path());
	std::string			arg1_str(_build_cgi_script_path(request));
	char				*arg0(new char[arg0_str.size() + 1]);
	char				*arg1(new char[arg1_str.size() + 1]);

	strcpy(arg0, arg0_str.c_str());
	strcpy(arg1, arg1_str.c_str());
	char	*args[0];
	args[0] = arg0;
	args[1] = arg1;
	args[2] = 0;
	pipe(req_pipe);
	pipe(res_pipe);
	if (-1 == (pid = _create_cgi_child_process()))
		return (FAILURE);
	if (!pid) {
		close(req_pipe[1]);
		close(res_pipe[0]);
		dup2(req_pipe[0], STDIN_FILENO);
		dup2(res_pipe[1], STDOUT_FILENO);
		chdir(request.get_location()->get_root().c_str());
		if (0 > execve(request.get_location()->get_cgi_path().c_str(), args, mv.get_tab()))
			perror("execve");
		close(req_pipe[0]);
		close(res_pipe[1]);
		exit(EXIT_FAILURE);
	}
	delete[] arg0;
	delete[] arg1;
	close(req_pipe[0]);
	close(res_pipe[1]);
	_cgi_input = request.get_body();
	_cgi_input_fd = req_pipe[1];
	_cgi_output_fd = res_pipe[0];
	WebServer::set_non_blocking(_cgi_input_fd);
	WebServer::set_non_blocking(_cgi_output_fd);
	return (SUCCESS);
}

int
Client::write_cgi_input(void) {
	size_t		buffer_size(std::min(_buffer_size, _cgi_input.size()));
	ssize_t		ret;

	if (_cgi_input.empty()) {
		close(_cgi_input_fd);
		_cgi_input_fd = 0;
		return (SUCCESS);
	}
	ret = write(_cgi_input_fd, _cgi_input.c_str(), buffer_size);
	if (ret < 0) {
		close(_cgi_input_fd);
		_cgi_input_fd = 0;
		return (FAILURE);
	}
	_cgi_input.pop_front(ret);
	if (ret == 0) {
		close(_cgi_input_fd);
		_cgi_input_fd = 0;
		waitpid(-1, NULL, 0);
	}
	return (SUCCESS);
}

int
Client::read_cgi_output(void) {
	char		buffer[_buffer_size];
	ssize_t		ret;

	ret = read(_cgi_output_fd, buffer, _buffer_size);
	if (ret < 0) {
		close(_cgi_output_fd);
		return (FAILURE);
	}
	if (ret == 0) {
		close(_cgi_output_fd);
		_cgi_output_fd = 0;
		return (_cgi_output_parsing());
	}
	_cgi_output = (_cgi_output + ByteArray(buffer, ret));
	return (SUCCESS);
}

bool
Client::_is_document_response(const CGIResponse &cgi_response) const {
	return (cgi_response.get_headers().key_exists(CGI_CONTENT_TYPE)
			&& !cgi_response.get_body().empty());
}

bool
Client::_is_local_redirect_response(const CGIResponse &cgi_response) const {
	return (cgi_response.get_body().empty()
			&& cgi_response.get_headers().key_exists(CGI_LOCATION)
			&& Path::is_local_path_query(cgi_response.get_headers().get_unparsed_value(CGI_LOCATION))
			&& cgi_response.get_headers().size() == 1);
}

bool
Client::_is_client_redirect_response(const CGIResponse &cgi_response) const {
	return (cgi_response.get_body().empty()
			&& cgi_response.get_headers().key_exists(CGI_LOCATION)
			&& Path::is_fragment_uri(cgi_response.get_headers().get_unparsed_value(CGI_LOCATION))
			&& cgi_response.get_headers().size() == 1);
}

int
Client::_cgi_output_parsing(void) {
	CGIResponse	cgi_response;

	while (ByteArray::npos != _cgi_output.find("\n")
			&& _cgi_output[0] != '\n')
		_collect_cgi_header(cgi_response);
	_cgi_output.pop_front(_cgi_output.find("\n") + 1);
	cgi_response.set_body(_cgi_output);
	_cgi_output.clear();
	return (_build_response_from_cgi_response(cgi_response));
}

int
Client::_build_response_from_cgi_response(const CGIResponse &cgi_response) {
	exchange_t	&exchange(_exchanges.front());
	Response	&response(exchange.second);
	if (cgi_response.get_headers().key_exists(CGI_STATUS))
		response.get_status_line().set_status_code(static_cast<status_code_t>(std::atol(cgi_response.get_headers().get_unparsed_value(CGI_STATUS).c_str())));
	else
		response.get_status_line().set_status_code(OK);
	for (Headers::const_iterator it(cgi_response.get_headers().begin()); it != cgi_response.get_headers().end() ; it++)
		response.get_headers().insert(*it);
	response.set_body(cgi_response.get_body());
	if (ResponseHandling::process_response_headers(exchange) == FAILURE)
		return (_process_error(exchange));
	return (_build_output(exchange));
}

void
Client::_collect_cgi_header(CGIResponse &cgi_response) {
	size_t				col(0);
	size_t				end_header(_cgi_output.find("\n"));
	header_t			current_header;

	if (ByteArray::npos != (col = _cgi_output.find_first_of(':'))) {
		current_header.name = _cgi_output.substr(0, col);
		current_header.unparsed_value = Syntax::trim_whitespaces(_cgi_output.substr(col + 1, (end_header - col - 1)));
		cgi_response.get_headers().insert(current_header);
	}
	_cgi_output.pop_front(end_header + 1);
}

int
Client::_process_HEAD(exchange_t &exchange) {
	DEBUG_COUT("entered _process_Head");
	return (_process_GET(exchange));
}

int
Client::_process_DELETE(exchange_t &exchange) {
	(void)exchange;
	return (SUCCESS);
}

int
Client::_process_CONNECT(exchange_t &exchange) {
	(void)exchange;
	return (SUCCESS);
}

int
Client::_process_OPTIONS(exchange_t &exchange) {
	(void)exchange;
	return (SUCCESS);
}

int
Client::_process_TRACE(exchange_t &exchange) {
	(void)exchange;
	return (SUCCESS);
}
