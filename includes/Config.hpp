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
# include "AConfig.hpp"
# include "Location.hpp"

class Config : public AConfig {
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
		int getClientMaxBodySize() const;
		void setClientMaxBodySize(int clientMaxBodySize);
		std::string getUploadDir() const;
		void setUploadDir(const std::string &uploadDir);
		std::list<int> getErrorPageCodes() const;
		void setErrorPageCodes(const std::list<int>& errorPageCodes);
		std::string getErrorPagePath() const;
		void setErrorPagePath(const std::string& errorPagePath);
		std::list<Location> getLocations() const;
		void addLocation(const Location& location);

		void showConfig() const;

	private:
		std::list<std::string> _server_name;
		std::string _ip_addr;
		int _port;
		int _client_max_body_size;
		std::string _upload_dir;
		std::list<int> _error_page_codes;
		std::string _error_page_path;
		std::list<Location> _locations;
};
#endif
