/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/18 04:57:30 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/18 07:08:17 by mdereuse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERS_HPP
# define HEADERS_HPP

# include <iostream>
# include <string>
# include <vector>
# include <list>

struct header_t {
	std::string name;
	std::string unparsed_value;
	std::list<std::string> value;
};

struct header_iterator {
	header_iterator(void);
	explicit header_iterator(std::list<header_t>::iterator x, std::vector<std::list<header_t>*>::iterator y);
	header_t &operator*(void);
	header_t *operator->(void);
	header_iterator &operator++(void);
	header_iterator operator++(int);
	header_iterator &operator--(void);
	header_iterator operator--(int);
	std::list<header_t>::iterator _cur;
	std::list<header_t>::iterator _first;
	std::list<header_t>::iterator _last;
	std::vector<std::list<header_t>*>::iterator _cell;
	void _set_cell(std::vector<std::list<header_t>*>::iterator cell);
};

struct const_header_iterator {
	const_header_iterator(void);
	explicit const_header_iterator(std::list<header_t>::const_iterator x, std::vector<std::list<header_t>*>::const_iterator y);
	explicit const_header_iterator(const header_iterator &it);
	const header_t &operator*(void);
	const header_t *operator->(void);
	const_header_iterator &operator++(void);
	const_header_iterator operator++(int);
	const_header_iterator &operator--(void);
	const_header_iterator operator--(int);
	std::list<header_t>::const_iterator _cur;
	std::list<header_t>::const_iterator _first;
	std::list<header_t>::const_iterator _last;
	std::vector<std::list<header_t>*>::const_iterator _cell;
};

class Headers {

	public:

		Headers(void);
		Headers(const Headers &x);
		~Headers(void);
		Headers &operator=(const Headers &x);

		void insert(const header_t &header);
		void insert(const std::string &key, const std::string &unparsed_value);

		bool key_exists(const std::string &key) const;

		const std::list<std::string> &get_value(const std::string &key) const throw (std::invalid_argument);
		const std::string &get_unparsed_value(const std::string &key) const throw (std::invalid_argument);

		void set_value(const std::string &key, const std::list<std::string>& parsed_value) throw (std::invalid_argument);

		void reset(void);
		void render(void) const;

	private:

		static const size_t _tab_size;
		std::vector<std::list<header_t>*> _tab;

		unsigned long _hash(const char *buf) const;

};

#endif
