/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 22:16:28 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/20 10:49:21 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "RequestParsing.hpp"
#include "ResponseHandling.hpp"
#include "WebServer.hpp"

const size_t	Client::_buffer_size(2);

Client::Client(void) :
	_sd(),
	_fd(),
	_cgi_fd(),
	_addr(),
	_socket_len(),
	_virtual_servers(),
	_input_str(),
	_output_str(),
	_cgi_output_str(),
	_exchanges(),
	_closing(false),
	_connection_refused(false) {}

Client::Client(int sd, struct sockaddr addr, socklen_t socket_len,
	const std::list<const VirtualServer*> &virtual_servers, bool connection_refused) :
	_sd(sd),
	_fd(),
	_cgi_fd(),
	_addr(addr),
	_socket_len(socket_len),
	_virtual_servers(virtual_servers),
	_input_str(),
	_output_str(),
	_cgi_output_str(),
	_exchanges(),
	_closing(false),
	_connection_refused(connection_refused) {}

Client::Client(const Client &x) :
	_sd(x._sd),
	_fd(x._fd),
	_cgi_fd(x._cgi_fd),
	_addr(x._addr),
	_socket_len(x._socket_len),
	_virtual_servers(x._virtual_servers),
	_input_str(x._input_str),
	_output_str(x._output_str),
	_cgi_output_str(x._output_str),
	_exchanges(x._exchanges),
	_closing(x._closing),
	_connection_refused(x._connection_refused) {}

Client::~Client(void) {}

Client
&Client::operator=(const Client &x) {
	_fd = x._fd;
	_cgi_fd = x._cgi_fd;
	_exchanges = x._exchanges;
	_input_str = x._input_str;
	_output_str = x._output_str;
	_cgi_output_str = x._cgi_output_str;
	_closing = x._closing;
	_connection_refused = x._connection_refused;
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
Client::get_cgi_fd(void) const {
	return (_cgi_fd);
}

/*
 * REQUEST RECEPTION
 */

int
Client::_process_connection_refused() {
	exchange_t refused_exchange = std::make_pair(Request(*this), Response());
	refused_exchange.second.get_status_line().set_http_version("HTTP/1.1");
	refused_exchange.second.get_status_line().set_status_code(SERVICE_UNAVAILABLE);
	refused_exchange.first.set_compromising(true);
	_closing = true;
	_process_error(refused_exchange);
	return (FAILURE);
}

int
Client::read_socket(void) {
	char	buffer[_buffer_size + 1];
	int		ret;

	if (_connection_refused)
		return (_process_connection_refused());
	if (0 >= (ret = read(_sd, buffer, _buffer_size))) {
		if (0 == ret)
			std::cout << "the client closed the connection." << std::endl;
		else
			std::cerr << "error during reading the socket: " << strerror(errno) << std::endl;
		return (FAILURE);
	}
	buffer[ret] = '\0';
	_input_str += std::string(buffer);
	RequestParsing::parsing(*this);
	if (!_exchanges.empty() && _exchanges.front().first.get_status() == Request::REQUEST_RECEIVED)
		return (_process(_exchanges.front()));
	return (SUCCESS);
}

int
Client::write_socket(void) {
	exchange_t	&exchange = _exchanges.front();
	Request		&request(exchange.first);
	size_t		to_write, output_size = _output_str.size();
	ssize_t 	write_return;

	if (output_size == 0)
		return SUCCESS;
	to_write = output_size > _buffer_size ? _buffer_size : output_size;
	write_return = write(_sd, _output_str.c_str(), to_write);
	if (write_return == -1)
		return FAILURE;
	_output_str.erase(0, write_return);
	if (_output_str.empty()) {
		if (request.get_compromising())
			return (FAILURE);
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

	request.set_status(Request::REQUEST_PROCESSED);
	response.get_status_line().set_http_version("HTTP/1.1");
	if (response.get_status_line().get_status_code() != TOTAL_STATUS_CODE)
		return (_process_error(exchange));
	if (_is_cgi_related(request))
		return (_handle_cgi(exchange));
	if (request.get_request_line().get_method() == GET)
		return (_process_GET(exchange));
	return (FAILURE);
}


std::string
Client::_format_index_path(const std::string& dir_path, const std::string& index_file) {
	std::string definite_path = dir_path;
	std::string definite_index_file = index_file;

	if (*(--dir_path.end()) != '/')
		definite_path += "/";
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
		return FAILURE;
	response.set_target_path(definite_path);
	return SUCCESS;
}

std::string
Client::_format_autoindex_page(exchange_t& exchange, const std::set<std::string>& directory_names,
	const std::set<std::string>& file_names) {
	std::stringstream ss;
	std::string		target_path = exchange.second.get_target_path();
	std::string		request_target = exchange.first.get_request_line().get_request_target();
	std::string		dir_name(request_target.substr(0, request_target.find('?')));

	ss << "<html>" << std::endl << "<head>" << std::endl;
	ss << "<title>Index of </title></head>" << std::endl;
	ss << "<body bgcolor=\"white\">" << std::endl;
	ss << "<h1>Index of " << dir_name << "</h1><hr><pre>";
	ss << "<table><tr><th>Name</th><th>Last modification</th><th>Size</th></tr>";
	for (std::set<std::string>::iterator it = directory_names.begin(); it != directory_names.end(); it++)
		_format_autoindex_entry(ss, *it, target_path, true);
	for (std::set<std::string>::iterator it = file_names.begin(); it != file_names.end(); it++)
		_format_autoindex_entry(ss, *it, target_path, false);
	ss << "</table></pre><hr></body>" << std::endl << "</html>" << std::endl;
	return ss.str();
}

void
Client::_format_autoindex_entry(std::stringstream& ss, const std::string& filename,
	const std::string& target_path, bool is_dir) {
	std::string		definite_filename, fullpath, size_str;
	struct stat		stat_buf;
	char 			time_buf[64];
	time_t			last_modification;
	struct tm		*tm;

	fullpath = target_path + filename;
	if (stat(fullpath.c_str(), &stat_buf) == -1) {
		return;
	}
	last_modification = stat_buf.st_mtim.tv_sec;
	tm = localtime(&last_modification);
	strftime(time_buf, sizeof(time_buf), "%d-%b-%Y %H:%M", tm);
	definite_filename = is_dir ? filename + "/" : filename;
	ss << "<tr><td><a href=\"" << definite_filename << "\">";
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
		return FAILURE;
	}
	while ((file_listing = readdir(directory))) {
		if (file_listing->d_type == DT_DIR
			&& strcmp(file_listing->d_name, ".") == 0)
			continue;
		if (file_listing->d_type == DT_DIR)
			directory_names.insert(file_listing->d_name);
		else if (file_listing->d_type == DT_REG)
			file_names.insert(file_listing->d_name);
	}
	response.set_body(_format_autoindex_page(exchange, directory_names, file_names));
	ResponseHandling::generate_basic_headers(exchange);
	closedir(directory);
	return (_build_output_str(exchange));
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

void
Client::_generate_error_page(exchange_t &exchange) {
	Response& response = exchange.second;
	status_code_t error_code = response.get_status_line().get_status_code();
	std::stringstream ss;

	ss << "<html>" << std::endl << "<head>" << std::endl;
	ss << "<title>Error " << Syntax::status_codes_tab[error_code].code_str;
	ss << " - " << Syntax::status_codes_tab[error_code].reason_phrase;
	ss << "</title>" << std::endl << "</head>" << std::endl;
	ss << "<body bgcolor=\"white\">" << std::endl << "<center>" << std::endl;
	ss << "<h1>Error " << Syntax::status_codes_tab[error_code].code_str;
	ss << " - " << Syntax::status_codes_tab[error_code].reason_phrase;
	ss << "</h1>" << std::endl << "</center>" << std::endl;
	ss << "<hr><center>webserv/1.0</center>" << std::endl;
	ss << "</body>" << std::endl << "</html>" << std::endl;
	response.set_body(ss.str());
	ResponseHandling::generate_basic_headers(exchange);
}

int
Client::_process_error(exchange_t &exchange) {
	Request		&request(exchange.first);
	Response	&response(exchange.second);
	std::string error_page_path;
	std::list<status_code_t>	error_codes(request.get_virtual_server()->get_error_page_codes());

	request.get_headers().clear();
	response.get_headers().clear();
	if (!request.get_location())
		request.set_location(&request.get_virtual_server()->get_locations().back());
	for (std::list<status_code_t>::iterator it(error_codes.begin()) ; it != error_codes.end() ; it++) {
		if (response.get_status_line().get_status_code() == *it) {
			error_page_path = request.get_virtual_server()->get_error_page_path();
			response.set_target_path(error_page_path);
			return(_open_file_to_read(error_page_path));
		}
	}
	_generate_error_page(exchange);
	return (_build_output_str(exchange));
}


std::string
Client::_build_resource_path(Request &request) {
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	absolute_path(request_target.substr(0, request_target.find('?')));
	std::string	location_root(request.get_location()->get_root());

	return (location_root + absolute_path);
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

int
Client::read_file(void) {
	exchange_t	&exchange(_exchanges.front());
	Response	&response(exchange.second);
	char		buffer[_buffer_size + 1];
	int			ret;

	ret = read(_fd, buffer, _buffer_size);
	if (ret < 0) {
		close(_fd);
		return (FAILURE);
	}
	if (ret == 0) {
		close(_fd);
		_fd = 0;
		if (ResponseHandling::process_response_headers(exchange) == FAILURE)
			return (_process_error(exchange));
		return (_build_output_str(exchange));
	}
	buffer[ret] = '\0';
	response.set_body(response.get_body() + std::string(buffer));
	return (SUCCESS);
}

int
Client::_build_output_str(exchange_t &exchange) {
	Response	&response(exchange.second);
	AHTTPMessage::HTTPHeaders& headers = response.get_headers();

	_output_str.clear();
	_output_str += response.get_status_line().get_http_version();
	_output_str += " ";
	_output_str += Syntax::status_codes_tab[response.get_status_line().get_status_code()].code_str;
	_output_str += " ";
	_output_str += Syntax::status_codes_tab[response.get_status_line().get_status_code()].reason_phrase;
	_output_str += "\r\n";
	for(size_t i = 0; i < TOTAL_RESPONSE_HEADERS; i++) {
		if (headers.key_exists(Syntax::response_headers_tab[i].header_index)) {
			_output_str += Syntax::response_headers_tab[i].name + ": ";
			_output_str += headers.get_unparsed_value(Syntax::response_headers_tab[i].header_index);
			_output_str += "\r\n";
		}
	}
	_output_str += "\r\n";
	_output_str += response.get_body();
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
	return (request.get_location()->get_cgi_path() + path);
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
	pipe(req_pipe);
	pipe(res_pipe);
	if (-1 == (pid = _create_cgi_child_process()))
		return (FAILURE);
	if (!pid) {
		close(req_pipe[1]);
		close(res_pipe[0]);
		dup2(req_pipe[0], STDIN_FILENO);
		dup2(res_pipe[1], STDOUT_FILENO);
		if (0 > execve(_build_cgi_script_path(request).c_str(), mv.get_tab(), mv.get_tab()))
			perror("execve");
		return (FAILURE);
	}
	close(req_pipe[0]);
	close(res_pipe[1]);
	write(req_pipe[1], request.get_body().c_str(), request.get_body().size());
	close(req_pipe[1]);
	_cgi_fd = res_pipe[0];
	return (SUCCESS);
}

int
Client::read_cgi(void) {
	char		buffer[_buffer_size + 1];
	int			ret;

	ret = read(_cgi_fd, buffer, _buffer_size);
	if (ret < 0) {
		close(_cgi_fd);
		return (FAILURE);
	}
	if (ret == 0) {
		close(_cgi_fd);
		_cgi_fd = 0;
		std::cout << "end of file" << std::endl;
		return (_cgi_output_str_parsing());
	}
	buffer[ret] = '\0';
	std::cout << buffer;
	_cgi_output_str += buffer;
	return (SUCCESS);
}

int
Client::_cgi_output_str_parsing(void) {
	CGIResponse	cgi_response;

	while (std::string::npos != _cgi_output_str.find("\n")
			&& _cgi_output_str.compare(0, 1, "\n"))
		_collect_cgi_header(cgi_response);
	_cgi_output_str.erase(0, _cgi_output_str.find("\n") + 1);
	cgi_response.set_body(_cgi_output_str);
	_cgi_output_str.clear();
	cgi_response.get_headers().render();
	return (SUCCESS);
}

void
Client::_collect_cgi_header(CGIResponse &cgi_response) {
	size_t				col(0);
	size_t				end_header(_cgi_output_str.find("\n"));
	header_t			current_header;

	if (std::string::npos != (col = _cgi_output_str.find_first_of(':'))) {
		current_header.name = _cgi_output_str.substr(0, col);
		current_header.unparsed_value = Syntax::trim_whitespaces(_cgi_output_str.substr(col + 1, (end_header - col - 1)));
		cgi_response.get_headers().insert(current_header);
	}
	_cgi_output_str.erase(0, end_header + 1);
}
