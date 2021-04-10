/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AConfig.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/07 16:55:40 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/07 16:55:41 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ACONFIG_HPP
# define ACONFIG_HPP
# include <iostream>
# include <string>
# include <list>

# define DEFAULT_AUTOINDEX false
# define AUTOINDEX(x) x ? "on" : "off"
# define DEFAULT_ALLOWED_METHOD "GET"
# define DEFAULT_INDEX "index.html"

class AConfig {
	public:
		AConfig();
		virtual ~AConfig();

		std::string getRoot() const;
		void setRoot(const std::string &root);
		bool isAutoindex() const;
		void setAutoindex(bool autoindex);
		std::list<std::string> getIndex() const;
		void setIndex(const std::list<std::string> &index);
		std::list<std::string> getMethods() const;
		void setMethods(const std::list<std::string> &methods);
		std::string getCgiExtension() const;
		void setCgiExtension(const std::string &cgiExtension);
		std::string getCgiPath() const;
		void setCgiPath(const std::string &cgiPath);

		virtual void showConfig() const = 0;
		void showCommonConfig() const;

	protected:
		std::string _root;
		bool _autoindex;
		std::list<std::string> _index;
		std::list<std::string> _methods;
		std::string _cgi_extension;
		std::string _cgi_path;

	private:
		AConfig(const AConfig& src);
		AConfig& operator=(const AConfig& rhs);

};
#endif
