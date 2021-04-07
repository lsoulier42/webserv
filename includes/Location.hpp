/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsoulier <lsoulier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/07 16:55:59 by lsoulier          #+#    #+#             */
/*   Updated: 2021/04/07 16:56:00 by lsoulier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP
# include "AConfig.hpp"

class Location : public AConfig {
	public:
		Location();
		Location(const Location& src);
		Location& operator=(const Location& rhs);
		~Location();

		std::string getConfigType() const;
		std::string getPath() const;
		void setPath(const std::string& path);

	private:
		std::string _path;
};
#endif
