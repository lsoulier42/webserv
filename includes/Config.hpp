/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 17:04:14 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/06 17:04:15 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP
# include <iostream>
# include <string>
# include <list>

class Config {
	public:
		Config();
		~Config();
		Config(const Config& src);
		Config& operator=(const Config& rhs);

		std::list<std::string> getServerName() const;
		void setServerName(const std::list<std::string> &serverName);
		std::string getIpAddr() const;
		void setIpAddr(const std::string& ipAddr);
		int getPort() const;
		void setPort(int port);
		std::string getRoot() const;
		void setRoot(const std::string &root);
		bool isAutoindex() const;
		void setAutoindex(bool autoindex);
		std::list<std::string> getIndex() const;
		void setIndex(const std::list<std::string> &index);
		std::list<std::string> getMethods() const;
		void setMethods(const std::list<std::string> &methods);
		int getClientMaxBodySize() const;
		void setClientMaxBodySize(int clientMaxBodySize);
		std::string getUploadDir() const;
		void setUploadDir(const std::string &uploadDir);

	private:
		std::list<std::string> _server_name;
		std::string _ip_addr;
		int _port;
		std::string _root;
		bool _autoindex;
		std::list<std::string> _index;
		std::list<std::string> _methods;
		int _client_max_body_size;
		std::string _upload_dir;

		//TODO : location (need vector of future class instance Location ?
		//TODO : CGI params
};
#endif
