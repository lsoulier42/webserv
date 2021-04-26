/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIScriptArgs.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/24 09:16:24 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/24 10:59:33 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGISCRIPTARGS_HPP
# define CGISCRIPTARGS_HPP

# include <string>
# include <cstring>
# include "Request.hpp"

struct CGIScriptArgs {

	CGIScriptArgs(void);
	explicit CGIScriptArgs(const Request &request) throw(std::bad_alloc);
	CGIScriptArgs(const CGIScriptArgs &x) throw(std::bad_alloc);
	~CGIScriptArgs(void);
	CGIScriptArgs &operator=(const CGIScriptArgs &x) throw(std::bad_alloc);

	static char *_build_script_path(const Request &request) throw(std::bad_alloc);
	static char *_build_requested_file_path(const Request &request) throw(std::bad_alloc);

	static const size_t	size_tab;
	char **tab;

};

#endif
