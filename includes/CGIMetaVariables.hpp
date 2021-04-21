/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIMetaVariables.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/16 20:19:23 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/16 20:26:28 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIMETAVARIABLES_HPP
# define CGIMETAVARIABLES_HPP

#include <string>
#include <cstdlib>
#include <cstring>
#include "Syntax.hpp"
#include "Request.hpp"

class CGIMetaVariables {
	
	public:

		CGIMetaVariables(void);
		CGIMetaVariables(const Request &request) throw(std::bad_alloc);
		CGIMetaVariables(const CGIMetaVariables &x) throw(std::bad_alloc);
		~CGIMetaVariables(void);
		CGIMetaVariables &operator=(const CGIMetaVariables &x) throw(std::bad_alloc);

		static size_t get_size(void);
		char **get_tab(void) const;

	private:

		typedef char *(*_mv_builder_t)(const Request &request);

		static char *_build_auth_type(const Request &request) throw(std::bad_alloc);
		static char *_build_content_length(const Request &request) throw(std::bad_alloc);
		static char *_build_content_type(const Request &request) throw(std::bad_alloc);
		static char *_build_gateway_interface(const Request &request) throw(std::bad_alloc);
		static char *_build_path_info(const Request &request) throw(std::bad_alloc);
		static char *_build_path_translated(const Request &request) throw(std::bad_alloc);
		static char *_build_query_string(const Request &request) throw(std::bad_alloc);
		static char *_build_remote_addr(const Request &request) throw(std::bad_alloc);
		static char *_build_remote_ident(const Request &request) throw(std::bad_alloc);
		static char *_build_remote_user(const Request &request) throw(std::bad_alloc);
		static char *_build_request_method(const Request &request) throw(std::bad_alloc);
		static char *_build_request_uri(const Request &request) throw(std::bad_alloc);
		static char *_build_script_name(const Request &request) throw(std::bad_alloc);
		static char *_build_server_name(const Request &request) throw(std::bad_alloc);
		static char *_build_server_port(const Request &request) throw(std::bad_alloc);
		static char *_build_server_protocol(const Request &request) throw(std::bad_alloc);
		static char *_build_server_software(const Request &request) throw(std::bad_alloc);

		static const size_t _size;
		static const _mv_builder_t _builder_tab[];
		static const std::string _auth_type;
		static const std::string _content_length;
		static const std::string _content_type;
		static const std::string _gateway_interface;
		static const std::string _path_info;
		static const std::string _path_translated;
		static const std::string _query_string;
		static const std::string _remote_addr;
		static const std::string _remote_ident;
		static const std::string _remote_user;
		static const std::string _request_method;
		static const std::string _request_uri;
		static const std::string _script_name;
		static const std::string _server_name;
		static const std::string _server_port;
		static const std::string _server_protocol;
		static const std::string _server_software;

		char **_tab;

};

#endif
