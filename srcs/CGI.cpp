/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/27 15:15:10 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/30 06:27:42 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"
#include "WebServer.hpp"

CGI::CGI(void) {}

bool
CGI::is_cgi_related(const Request &request) {
	std::string	request_target(request.get_request_line().get_request_target());
	std::string	path(request_target.substr(0, request_target.find("?")));
	std::string extension(request.get_location()->get_cgi_extension());

	return (path.find('.') != std::string::npos
			&& !extension.empty()
			&& !(path.substr(path.rfind("."))).compare(0, extension.size(), extension));
}

std::string
CGI::_build_output_file_name(const Request &request) {
	return ("/tmp/cgi_output_" + request.get_ident());
}

CGI::cgi_output_ret_t
CGI::read_output(Client &client) {
	Client::exchange_t	&exchange(client._exchanges.front());
	Request				&request(exchange.first);
	Response			&response(exchange.second);
	char				buffer[Client::read_buffer_size];
	ssize_t				ret;

	ret = read(client._cgi_output_fd, buffer, Client::read_buffer_size);
	if (ret < 0) {
		DEBUG_COUT("Error during reading of CGI output: " << strerror(errno) << "(" << request.get_ident() << ")");
		close(client._cgi_output_fd);
		client._cgi_output_fd = 0;
		unlink(_build_output_file_name(request).c_str());
		client._cgi_output.clear();
		client._cgi_response.reset();
		return (SERVER_ERROR);
	}

	if (!client._cgi_response.get_content_reception()) {

		client._cgi_output.append(buffer, ret);
		while (_header_received(client._cgi_output))
			_collect_header(client._cgi_response, client._cgi_output);
		if (_headers_received(client._cgi_output)) {
			client._cgi_output.pop_front(client._cgi_output.find("\n") + 1);
			if (SUCCESS != _pick_response_type(client._cgi_response)) {
				DEBUG_COUT("Error during parsing of cgi output (" << request.get_ident() << ")");
				close(client._cgi_output_fd);
				client._cgi_output_fd = 0;
				unlink(_build_output_file_name(request).c_str());
				client._cgi_output.clear();
				client._cgi_response.reset();
				return (SCRIPT_ERROR);
			}
			if (_body_expected(client._cgi_response)) {
				client._cgi_response.set_content_reception(true);
				lseek(client._cgi_output_fd, (-1) * client._cgi_output.size(), SEEK_CUR);
			} else {
				close(client._cgi_output_fd);
				client._cgi_output_fd = 0;
				unlink(_build_output_file_name(request).c_str());
			}
			client._cgi_output.clear();
			return (_handle_cgi_response(client));
		}

	} else {

		if (response.get_chunked()) {
			response.append_content_chunk(buffer, ret);
			if (response.get_sending_indicator() == 0) {
				close(client._cgi_output_fd);
				client._cgi_output_fd = 0;
				unlink(_build_output_file_name(request).c_str());
				client._cgi_response.reset();
			}
		} else {
			response.get_content().append(buffer, ret);
			response.set_sending_indicator(response.get_sending_indicator() + ret);
			if (response.get_sending_indicator() >= response.get_length()) {
				close(client._cgi_output_fd);
				client._cgi_output_fd = 0;
				unlink(_build_output_file_name(request).c_str());
				client._cgi_response.reset();
			}
		}

	}

	return (AGAIN);
}

int
CGI::init_CGI(Client &client) {
	Client::exchange_t	&exchange(client._exchanges.front());
	Response			&response(exchange.second);
	Request				&request(exchange.first);
	std::stringstream	ss;
	int 				fd;

	DEBUG_COUT("CGI request processing (" << request.get_ident() << ")");
	if (request.get_tmp_filename().empty()) {
		ss << "./tmp/i" << request.get_id();
		fd = open(ss.str().c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		if (fd < 0) {
			response.get_status_line().set_status_code(INTERNAL_SERVER_ERROR);
			DEBUG_COUT("Error during opening of empty tmp file (" <<
				ss.str() << ") for CGI (" << request.get_id() << ")");
			return (FAILURE);
		}
		request.set_tmp_filename(ss.str());
		close(fd);
	}
	return (_launch_script(client));
}

int
CGI::_create_child_process(void) {
	pid_t	pid;

	pid = fork();
	while (pid == -1 && errno == EAGAIN)
		pid = fork();
	return (pid);
}

int
CGI::_launch_script(Client &client) {
	Client::exchange_t	&exchange(client._exchanges.front());
	Request				&request(exchange.first);
	std::string			tmp_filename = request.get_tmp_filename();
	pid_t				pid;
	int					input_fd;
	int					output_fd;
	CGIMetaVariables	mv(request);
	CGIScriptArgs		args(request);

	if (0 > (input_fd = open(tmp_filename.c_str(), O_RDONLY, S_IRUSR | S_IWUSR))) {
		DEBUG_COUT("Error during opening of tmp cgi input file :" << strerror(errno) << "(" << request.get_ident() << ")");
		return (FAILURE);
	}

	if (0 > (output_fd = open(_build_output_file_name(request).c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR))) {
		DEBUG_COUT("Error during opening of tmp cgi output file (for writing) :" << strerror(errno) << "(" << request.get_ident() << ")");
		close(input_fd);
		unlink(tmp_filename.c_str());
		return (FAILURE);
	}
	if (-1 == (pid = _create_child_process())) {
		DEBUG_COUT("Error creating child process: " << strerror(errno) << "(" << request.get_ident() << ")");
		close(input_fd);
		close(output_fd);
		unlink(tmp_filename.c_str());
		unlink(_build_output_file_name(request).c_str());
		return (FAILURE);
	}
	if (!pid) {
		dup2(input_fd, STDIN_FILENO);
		dup2(output_fd, STDOUT_FILENO);
		dup2(output_fd, STDERR_FILENO);
		execve(request.get_location()->get_cgi_path().c_str(), args.tab, mv.get_tab());
		write(STDOUT_FILENO, "Status: 500 Internal Server Error\n\n", 34);
		close(input_fd);
		close(output_fd);
		exit(EXIT_FAILURE);
	}
	close(input_fd);
	close(output_fd);
	waitpid(-1, NULL, 0);
	unlink(tmp_filename.c_str());
	if (0 > (client._cgi_output_fd = open(_build_output_file_name(request).c_str(), O_RDONLY | O_NONBLOCK, S_IRUSR | S_IWUSR))) {
		DEBUG_COUT("Error during opening of tmp cgi output file (for reading) :" << strerror(errno) << "(" << request.get_ident() << ")");
		unlink(_build_output_file_name(request).c_str());
		return (FAILURE);
	}
	client._cgi_output.clear();
	client._cgi_response.reset();
	DEBUG_COUT("CGI processing went well through execve (" << request.get_ident() << ")");
	return (SUCCESS);
}

void
CGI::_collect_header(CGIResponse &cgi_response, ByteArray &output) {
	size_t		col(0);
	size_t		end_header(output.find("\n"));
	header_t	current_header;

	if (ByteArray::npos != (col = output.find_first_of(':'))) {
		current_header.name = output.substr(0, col);
		current_header.unparsed_value = Syntax::trim_whitespaces(output.substr(col + 1, (end_header - col - 1)));
		cgi_response.get_headers().insert(current_header);
	}
	output.pop_front(end_header + 1);
}

int
CGI::_pick_response_type(CGIResponse &cgi_response) {
	if (_is_local_redirect_response(cgi_response))
		cgi_response.set_type(CGIResponse::LOCAL_REDIRECT);
	else if (_is_client_redirect_response(cgi_response))
		cgi_response.set_type(CGIResponse::CLIENT_REDIRECT);
	else if (_is_client_redirect_response_with_document(cgi_response))
		cgi_response.set_type(CGIResponse::CLIENT_REDIRECT_DOC);
	else if (_is_document_response(cgi_response))
		cgi_response.set_type(CGIResponse::DOCUMENT);
	else
		return (FAILURE);
	return (SUCCESS);
}

bool
CGI::_header_received(const ByteArray &output) {
	return (!_headers_received(output)
			&& ByteArray::npos != output.find("\n"));
}

bool
CGI::_headers_received(const ByteArray &output) {
	return ((!output.empty() && output[0] == '\n')
				|| (output.size() >= 2 && output[0] == '\r' && output[1] == '\n'));
}

bool
CGI::_body_expected(const CGIResponse &cgi_response) {
	return (cgi_response.get_type() == CGIResponse::DOCUMENT
			|| cgi_response.get_type() == CGIResponse::CLIENT_REDIRECT_DOC);
}

bool
CGI::_is_local_redirection(const std::string &location) {
	return (!location.empty()
			&& !location.compare(0, 1, "/"));
}

bool
CGI::_is_client_redirection(const std::string &location) {
	return (!location.empty()
			&& std::string::npos != location.find(":")
			&& std::isalpha(location[0]));
}

bool
CGI::_is_redirection_status(const std::string &status_line) {
	std::string status_code(status_line.substr(0, status_line.find(' ')));
	int			status_code_int(std::atol(status_code.c_str()));
	return (Syntax::is_redirection_code(status_code_int));
}

bool
CGI::_is_document_response(const CGIResponse &cgi_response) {
	return (cgi_response.get_headers().key_exists(CGI_CONTENT_TYPE)
			&& !_is_client_redirect_response_with_document(cgi_response));
}

bool
CGI::_is_local_redirect_response(const CGIResponse &cgi_response) {
	return (cgi_response.get_headers().key_exists(CGI_LOCATION)
			&& _is_local_redirection(cgi_response.get_headers().get_unparsed_value(CGI_LOCATION))
			&& cgi_response.get_headers().size() == 1);
}

bool
CGI::_is_client_redirect_response(const CGIResponse &cgi_response) {
	return (cgi_response.get_headers().key_exists(CGI_LOCATION)
			&& _is_client_redirection(cgi_response.get_headers().get_unparsed_value(CGI_LOCATION))
			&& cgi_response.get_headers().size() == 1);
}

bool
CGI::_is_client_redirect_response_with_document(const CGIResponse &cgi_response) {
	return (cgi_response.get_headers().key_exists(CGI_LOCATION)
			&& _is_client_redirection(cgi_response.get_headers().get_unparsed_value(CGI_LOCATION))
			&& cgi_response.get_headers().key_exists(CGI_STATUS)
			&& _is_redirection_status(cgi_response.get_headers().get_unparsed_value(CGI_STATUS))
			&& cgi_response.get_headers().key_exists(CGI_CONTENT_TYPE));
}

CGI::cgi_output_ret_t
CGI::_handle_cgi_response(Client &client) {
	if (client._cgi_response.get_type() == CGIResponse::LOCAL_REDIRECT)
		return (_handle_local_redirect_response(client));
	else if (client._cgi_response.get_type() == CGIResponse::CLIENT_REDIRECT)
		return (_handle_client_redirect_response(client));
	else if (client._cgi_response.get_type() == CGIResponse::CLIENT_REDIRECT_DOC)
		return (_handle_client_redirect_doc_response(client));
	else if (client._cgi_response.get_type() == CGIResponse::DOCUMENT)
		return (_handle_document_response(client));
	else
		return (SCRIPT_ERROR);
}

CGI::cgi_output_ret_t
CGI::_handle_local_redirect_response(Client &client) {
	Client::exchange_t	&exchange(client._exchanges.front());
	Request				&request(exchange.first);
	CGIResponse			&cgi_response(client._cgi_response);

	request.get_request_line().set_request_target(cgi_response.get_headers().get_unparsed_value(CGI_LOCATION));
	request.set_status(Request::REQUEST_RECEIVED);
	cgi_response.reset();
	return (REDIRECT);
}

CGI::cgi_output_ret_t
CGI::_handle_client_redirect_response(Client &client) {
	Client::exchange_t	&exchange(client._exchanges.front());
	Response			&response(exchange.second);
	CGIResponse			&cgi_response(client._cgi_response);

	for (Headers::const_iterator it(cgi_response.get_headers().begin()); it != cgi_response.get_headers().end() ; it++)
		response.get_headers().insert(*it);
	response.get_status_line().set_status_code(FOUND);
	cgi_response.reset();
	return (COMPLETE);
}

CGI::cgi_output_ret_t
CGI::_handle_client_redirect_doc_response(Client &client) {
	Client::exchange_t	&exchange(client._exchanges.front());
	Request				&request(exchange.first);
	Response			&response(exchange.second);
	CGIResponse			&cgi_response(client._cgi_response);
	std::string			status_line(cgi_response.get_headers().get_unparsed_value(CGI_STATUS));
	std::string 		status_code(status_line.substr(0, status_line.find(" ")));
	int					status_code_int(std::atol(status_code.c_str()));
	size_t				i(0);

	while (Syntax::status_codes_tab[i].code_index != TOTAL_STATUS_CODE
		&& Syntax::status_codes_tab[i].code_int != status_code_int)
		i++;
	response.get_status_line().set_status_code(Syntax::status_codes_tab[i].code_index);
	cgi_response.get_headers().erase(CGI_STATUS);

	for (Headers::const_iterator it(cgi_response.get_headers().begin()); it != cgi_response.get_headers().end() ; it++)
		response.get_headers().insert(*it);

	if (cgi_response.get_headers().key_exists(CGI_CONTENT_LENGTH))
		response.set_length(static_cast<size_t>(std::atol(cgi_response.get_headers().get_unparsed_value(CGI_CONTENT_LENGTH).c_str())));
	else
		response.set_chunked(true);

	ResponseHandling::process_cgi_response_headers(exchange);

	DEBUG_COUT("CGI client redirect response with document handling went well (" << request.get_ident() << ")");
	return (COMPLETE);
}

CGI::cgi_output_ret_t
CGI::_handle_document_response(Client &client) {
	Client::exchange_t	&exchange(client._exchanges.front());
	Request				&request(exchange.first);
	Response			&response(exchange.second);
	CGIResponse			&cgi_response(client._cgi_response);

	if (cgi_response.get_headers().key_exists(CGI_STATUS)) {
		std::string	status_line(cgi_response.get_headers().get_unparsed_value(CGI_STATUS));
		std::string status_code(status_line.substr(0, status_line.find(' ')));
		int			status_code_int(std::atol(status_code.c_str()));
		size_t		i(0);
		while (Syntax::status_codes_tab[i].code_index != TOTAL_STATUS_CODE
			&& Syntax::status_codes_tab[i].code_int != status_code_int)
			i++;
		response.get_status_line().set_status_code(Syntax::status_codes_tab[i].code_index);
		cgi_response.get_headers().erase(CGI_STATUS);
	}
	else
		response.get_status_line().set_status_code(OK);

	for (Headers::const_iterator it(cgi_response.get_headers().begin()); it != cgi_response.get_headers().end() ; it++)
		response.get_headers().insert(*it);

	if (cgi_response.get_headers().key_exists(CGI_CONTENT_LENGTH))
		response.set_length(static_cast<size_t>(std::atol(cgi_response.get_headers().get_unparsed_value(CGI_CONTENT_LENGTH).c_str())));
	else
		response.set_chunked(true);

	ResponseHandling::process_cgi_response_headers(exchange);

	DEBUG_COUT("CGI document response handling went well (" << request.get_ident() << ")");
	return (COMPLETE);
}
