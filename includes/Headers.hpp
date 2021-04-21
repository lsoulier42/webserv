/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdereuse <mdereuse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/18 04:57:30 by mdereuse          #+#    #+#             */
/*   Updated: 2021/04/21 06:51:05 by mdereuse         ###   ########.fr       */
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
	friend
	bool operator==(const header_iterator &lhs, const header_iterator &rhs);
	friend
	bool operator!=(const header_iterator &lhs, const header_iterator &rhs);
	header_iterator(void);
	explicit header_iterator(std::list<header_t>::iterator x, std::vector<std::list<header_t> >::iterator y);
	header_t &operator*(void);
	header_t *operator->(void);
	header_iterator &operator++(void);
	header_iterator operator++(int);
	header_iterator &operator--(void);
	header_iterator operator--(int);
	std::list<header_t>::iterator _cur;
	std::list<header_t>::iterator _first;
	std::list<header_t>::iterator _last;
	std::vector<std::list<header_t> >::iterator _cell;
	void _set_cell(std::vector<std::list<header_t> >::iterator cell);
};

struct const_header_iterator {
	friend
	bool operator==(const const_header_iterator &lhs, const const_header_iterator &rhs);
	friend
	bool operator!=(const const_header_iterator &lhs, const const_header_iterator &rhs);
	const_header_iterator(void);
	explicit const_header_iterator(std::list<header_t>::const_iterator x, std::vector<std::list<header_t> >::const_iterator y);
	const_header_iterator(const header_iterator &it);
	const header_t &operator*(void);
	const header_t *operator->(void);
	const_header_iterator &operator++(void);
	const_header_iterator operator++(int);
	const_header_iterator &operator--(void);
	const_header_iterator operator--(int);
	std::list<header_t>::const_iterator _cur;
	std::list<header_t>::const_iterator _first;
	std::list<header_t>::const_iterator _last;
	std::vector<std::list<header_t> >::const_iterator _cell;
	void _set_cell(std::vector<std::list<header_t> >::const_iterator cell);
};

class Headers {

	public:

		typedef header_iterator iterator;
		typedef const_header_iterator const_iterator;

		Headers(void);
		Headers(const Headers &x);
		~Headers(void);
		Headers &operator=(const Headers &x);

		iterator begin(void);
		const_iterator begin(void) const;
		iterator end(void);
		const_iterator end(void) const;

		size_t size(void) const;
		bool empty(void) const;
		void clear(void);

		void insert(const header_t &header);
		void insert(const std::string &key, const std::string &unparsed_value);

		bool key_exists(const std::string &key) const;

		const std::string &get_unparsed_value(const std::string &key) const throw (std::invalid_argument);
		const std::list<std::string> &get_value(const std::string &key) const throw (std::invalid_argument);

		void set_value(const std::string &key, const std::list<std::string>& parsed_value) throw (std::invalid_argument);

		void render(void) const;

	private:

		static const size_t _tab_size;
		std::vector<std::list<header_t> > _tab;
		iterator _start;
		iterator _finish;

		void _reset_start_finish(void);
		void _update_start(size_t index);
		void _update_finish(size_t index);
		unsigned long _hash(const char *buf) const;

};

#endif
