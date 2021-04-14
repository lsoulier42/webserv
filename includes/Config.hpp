/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/06 17:04:14 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/14 19:36:18 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP
# include <vector>
# include "AConfig.hpp"
# include "Location.hpp"

# define DEFAULT_MAX_BODY_SIZE 8192

class Config : public AConfig {
	public:
		Config();
		~Config();
		Config(const Config& src);
		Config& operator=(const Config& rhs);

		std::list<std::string> getServerNames() const;
		void setServerNames(const std::list<std::string> &serverNames);
		std::string getIpAddr() const;
		void setIpAddr(const std::string& ipAddr);
		int getPort() const;
		void setPort(int port);
		unsigned long getClientMaxBodySize() const;
		void setClientMaxBodySize(unsigned long clientMaxBodySize);
		std::string getUploadDir() const;
		void setUploadDir(const std::string &uploadDir);
		std::list<int> getErrorPageCodes() const;
		void setErrorPageCodes(const std::list<int>& errorPageCodes);
		const std::string &getErrorPagePath() const;
		void setErrorPagePath(const std::string& errorPagePath);
		std::list<Location> getLocations() const;
		void addLocation(const Location& location);

		void showConfig() const;

		static std::list<const Config*> buildConfigsList(const std::vector<Config>& configs, const Config* default_config);

	private:
		std::list<std::string> _server_names;
		std::string _ip_addr;
		int _port;
		unsigned long _client_max_body_size;
		std::string _upload_dir;
		std::list<int> _error_page_codes;
		std::string _error_page_path;
		std::list<Location> _locations;
};
#endif
