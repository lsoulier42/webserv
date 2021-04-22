/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ByteArray.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: louise <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 14:51:46 by louise            #+#    #+#             */
/*   Updated: 2021/04/21 14:51:46 by louise           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BYTE_ARRAY_HPP
# define BYTE_ARRAY_HPP
# include <cstring>
# include <vector>
# include <string>
# include <algorithm>
# include <iostream>
# include <stdexcept>

class ByteArray {
	public:
		typedef size_t size_type;
		typedef std::vector<char>::value_type value_type;
		typedef std::vector<char>::reference reference;
		typedef std::vector<char>::const_reference const_reference;
		typedef std::vector<char>::pointer pointer;
		typedef std::vector<char>::const_pointer const_pointer;
		typedef std::vector<char>::iterator iterator;
		typedef std::vector<char>::const_iterator const_iterator;
		typedef std::vector<char>::reverse_iterator reverse_iterator;
		typedef std::vector<char>::const_reverse_iterator const_reverse_iterator;

		ByteArray();
		explicit ByteArray(const std::string& to_add);
		ByteArray(const char* to_add, size_t len);
		ByteArray(const ByteArray& src);
		ByteArray& operator=(const ByteArray& rhs);
		virtual ~ByteArray();

		reference front();
		const_reference front() const;
		reference back();
		const_reference back() const;

		reference operator[]( size_type pos );
		const_reference operator[]( size_type pos ) const;
		ByteArray& operator+=(const ByteArray& rhs);
		ByteArray& operator+=(const std::string& rhs);

		iterator begin();
		const_iterator begin() const;
		iterator end();
		const_iterator end() const;
		reverse_iterator rbegin();
		const_reverse_iterator rbegin() const;
		reverse_iterator rend();
		const_reverse_iterator rend() const;

		bool empty() const;
		size_type size() const;

		void clear();
		void push_back(const char& c);
		void append(const char* to_add, size_t len);
		void append(const std::string& to_add);
		void append(const ByteArray& to_add);
		void pop_back();
		void pop_back(size_t len);
		void pop_front();
		void pop_front(size_t len);
		void resize( size_type count );


		const char* c_str() const;

		size_type find(const char& c) const;
		size_type rfind(const char& c) const;
		size_type find(const char* to_find, size_t len) const;
		size_type rfind(const char* to_find, size_t len) const;
		size_type find(const std::string& to_find) const;
		size_type rfind(const std::string& to_find) const;
		size_type find_first_of(const char& c) const;
		size_type find_first_of(const char* charset, size_t len) const;
		size_type find_first_of(const std::string& charset) const;
		size_type find_first_not_of(const char& c) const;
		size_type find_first_not_of(const char* charset, size_t len) const;
		size_type find_first_not_of(const std::string& charset) const;
		size_type find_last_of(const char& c) const;
		size_type find_last_of(const char* charset, size_t len) const;
		size_type find_last_of(const std::string& charset) const;
		size_type find_last_not_of(const char& c) const;
		size_type find_last_not_of(const char* charset, size_t len) const;
		size_type find_last_not_of(const std::string& charset) const;

		std::string substr(size_type pos, size_type len = npos) throw(std::out_of_range);
		ByteArray sub_byte_array(size_type pos, size_type len = npos) throw(std::out_of_range);

		int compare( size_type pos1, size_type count1, const std::string& str ) const throw(std::out_of_range);
		bool starts_with( const std::string& str ) const;
		bool ends_with( const std::string& str ) const;

		static const size_type npos;

	private:
		static const char* _memmem(const char *haystack, const char *needle,
			size_t haystack_size, size_t needle_size);
		static const char* _memrmem(const char *haystack, const char *needle,
			size_t haystack_size, size_t needle_size);

		std::vector<char> _byte_array;
};

ByteArray operator+(const ByteArray& lhs, const ByteArray& rhs);
ByteArray operator+(const ByteArray& lhs, const std::string& rhs);

std::ostream & operator<<(std::ostream & o, const ByteArray& src);

#endif
