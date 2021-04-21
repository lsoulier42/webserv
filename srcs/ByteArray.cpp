/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ByteArray.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: louise <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 14:51:36 by louise            #+#    #+#             */
/*   Updated: 2021/04/21 14:51:37 by louise           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ByteArray.hpp"

const ByteArray::size_type
ByteArray::npos = -1;

ByteArray::ByteArray() {

}

ByteArray::ByteArray(const std::string& to_add) {
	_byte_array.reserve(to_add.size());
	_byte_array.insert(begin(), to_add.begin(), to_add.end());
}

ByteArray::ByteArray(const char* to_add, size_t len) {
	_byte_array.reserve(len);
	for(size_t i = 0; i < len; i++) {
		_byte_array.push_back(to_add[i]);
	}
}

ByteArray::ByteArray(const ByteArray& src) {
	*this = src;
}

ByteArray&
ByteArray::operator=(const ByteArray& rhs) {
	if (this != &rhs) {
		_byte_array.operator=(rhs._byte_array);
	}
	return *this;
}

ByteArray::~ByteArray() {

}

ByteArray::reference
ByteArray::front() {
	return _byte_array.front();
}

ByteArray::const_reference
ByteArray::front() const {
	return _byte_array.front();
}


ByteArray::reference
ByteArray::back() {
	return _byte_array.back();
}

ByteArray::const_reference
ByteArray::back() const {
	return _byte_array.back();
}

ByteArray::reference
ByteArray::operator[]( ByteArray::size_type pos ) {
	return _byte_array[pos];
}

ByteArray::const_reference
ByteArray::operator[]( ByteArray::size_type pos ) const {
	return _byte_array[pos];
}

ByteArray::iterator
ByteArray::begin() {
	return _byte_array.begin();
}

ByteArray::const_iterator
ByteArray::begin() const {
	return _byte_array.begin();
}

ByteArray::iterator
ByteArray::end() {
	return _byte_array.end();
}

ByteArray::const_iterator
ByteArray::end() const {
	return _byte_array.end();
}

ByteArray::reverse_iterator
ByteArray::rbegin() {
	return _byte_array.rbegin();
}

ByteArray::const_reverse_iterator
ByteArray::rbegin() const {
	return _byte_array.rbegin();
}

ByteArray::reverse_iterator
ByteArray::rend() {
	return _byte_array.rend();
}

ByteArray::const_reverse_iterator
ByteArray::rend() const {
	return _byte_array.rend();
}

bool
ByteArray::empty() const {
	return _byte_array.empty();
}

ByteArray::size_type
ByteArray::size() const {
	return _byte_array.size();
}

void
ByteArray::clear() {
	_byte_array.clear();
}

void
ByteArray::push_back(const char& c) {
	_byte_array.push_back(c);
}

void
ByteArray::append(const char* to_add, size_t len) {
	_byte_array.resize(size() + len);
	for(size_t i = 0; i < len; i++) {
		_byte_array.push_back(to_add[i]);
	}
}

void
ByteArray::append(const std::string& to_add) {
	_byte_array.resize(size() + to_add.size());
	_byte_array.insert(end(), to_add.begin(), to_add.end());
}

void
ByteArray::append(const ByteArray& to_add) {
	size_t size = to_add.size();
	_byte_array.resize(this->size() + to_add.size());
	for(size_t i = 0; i < size; i++)
		_byte_array.push_back(to_add[i]);
}

void
ByteArray::pop_back() {
	_byte_array.pop_back();
}

void
ByteArray::pop_back(size_t len) {
	if (len > size())
		return ;
	this->resize(size() - len);
}

void
ByteArray::pop_front() {
	_byte_array.erase(begin());
}

void
ByteArray::pop_front(size_t len) {
	if (len > size())
		return;
	_byte_array.erase(begin(), begin() + len);
}

void
ByteArray::resize( ByteArray::size_type count ) {
	_byte_array.resize(count);
}

const char*
ByteArray::c_str() const {
	return &(*_byte_array.begin());
}

std::string
ByteArray::to_str() {
	return (this->substr(0));
}

ByteArray::size_type
ByteArray::find(const char& c) const {
	const char* byte_array_pointer = c_str();
	const char* memchr_return;

	memchr_return = (const char*)memchr(byte_array_pointer, c, size());
	if (memchr_return != NULL)
		return (memchr_return - byte_array_pointer);
	return npos;
}

ByteArray::size_type
ByteArray::rfind(const char& c) const {
	const char* byte_array_pointer = c_str();
	const char* memrchr_return;

	memrchr_return = (const char*)memrchr(byte_array_pointer, c, size());
	if (memrchr_return != NULL)
		return (memrchr_return - byte_array_pointer);
	return npos;
}

ByteArray::size_type
ByteArray::find(const char* to_find, size_t len) const {
	const char* byte_array_pointer = c_str();
	const char* memmem_return;

	memmem_return = _memmem(byte_array_pointer, to_find,
		size(), len);
	if (memmem_return != NULL)
		return memmem_return - byte_array_pointer;
	return npos;
}

ByteArray::size_type
ByteArray::find(const std::string& to_find) const {
	return find(to_find.c_str(), to_find.size());
}

ByteArray::size_type
ByteArray::rfind(const char* to_find, size_t len) const {
	const char* byte_array_pointer = c_str();
	const char* memrmem_return;

	memrmem_return = _memrmem(byte_array_pointer, to_find,
		size(), len);
	if (memrmem_return != NULL)
		return memrmem_return - byte_array_pointer;
	return npos;
}

ByteArray::size_type
ByteArray::rfind(const std::string& to_find) const {
	return rfind(to_find.c_str(), to_find.size());
}

ByteArray::size_type
ByteArray::find_first_of(const char& c) const {
	return find(c);
}

ByteArray::size_type
ByteArray::find_first_of(const char* charset, size_t len) const {
	size_t size = this->size();

	for(size_t i = 0; i < size; i++) {
		for (size_t j = 0; j < len; j++) {
			if (charset[j] == _byte_array[i])
				return i;
		}
	}
	return npos;
}

ByteArray::size_type
ByteArray::find_first_of(const std::string& charset) const {
	return this->find_first_of(charset.c_str(), charset.size());
}

ByteArray::size_type
ByteArray::find_first_not_of(const char& c) const {
	size_t size = this->size();

	for(size_t i = 0; i < size; i++) {
		if (_byte_array[i] != c)
			return (i);
	}
	return npos;
}

ByteArray::size_type
ByteArray::find_first_not_of(const char* charset, size_t len) const {
	size_t size = this->size();

	for(size_t i = 0; i < size; i++) {
		for(size_t j = 0; j < len; j++) {
			if (_byte_array[i] == charset[j])
				break ;
			if (j == len - 1)
				return (i);
		}
	}
	return npos;
}

ByteArray::size_type
ByteArray::find_first_not_of(const std::string& charset) const {
	return (find_first_not_of(charset.c_str(), charset.size()));
}

ByteArray::size_type
ByteArray::find_last_of(const char& c) const {
	return rfind(c);
}

ByteArray::size_type
ByteArray::find_last_of(const char* charset, size_t len) const {
	size_t size = this->size();

	for(ssize_t i = static_cast<ssize_t>(size) - 1; i >= 0; i--) {
		for (size_t j = 0; j < len; j++) {
			if (charset[j] == _byte_array[i])
				return (static_cast<size_t>(i));
		}
	}
	return npos;
}

ByteArray::size_type
ByteArray::find_last_of(const std::string& charset) const {
	return this->find_last_of(charset.c_str(), charset.size());
}

ByteArray::size_type
ByteArray::find_last_not_of(const char& c) const {
	size_t size = this->size();

	for(ssize_t i = static_cast<ssize_t>(size) - 1; i >= 0; i--) {
		if (_byte_array[i] != c)
			return (static_cast<size_t>(i));
	}
	return npos;
}

ByteArray::size_type
ByteArray::find_last_not_of(const char* charset, size_t len) const {
	size_t size = this->size();

	for(ssize_t i = static_cast<ssize_t>(size) - 1; i >= 0; i--) {
		for(size_t j = 0; j < len; j++) {
			if (_byte_array[i] == charset[j])
				break ;
			if (j == len - 1)
				return (static_cast<size_t>(i));
		}
	}
	return npos;
}

ByteArray::size_type
ByteArray::find_last_not_of(const std::string& charset) const {
	return (find_last_not_of(charset.c_str(), charset.size()));
}

const char*
ByteArray::_memmem(const char *haystack, const char *needle,
	size_t haystack_size, size_t needle_size) {

	for(size_t i = 0; i < haystack_size; i++) {
		if (haystack[i] == *needle) {
			for(size_t j = 0; j < needle_size
				&& (i + j < haystack_size); j++) {
				if (haystack[i + j] != needle[j])
					break;
				if (j == needle_size - 1)
					return (haystack + i);
			}
		}
	}
	return (NULL);
}

const char*
ByteArray::_memrmem(const char *haystack, const char *needle,
	size_t haystack_size, size_t needle_size) {

	for(ssize_t i = static_cast<ssize_t>(haystack_size) - 1; i >= 0; i--) {
		if (haystack[i] == *needle) {
			for(size_t j = 0; j < needle_size
				&& (i + j < haystack_size); j++) {
				if (haystack[i + j] != needle[j])
					break;
				if (j == needle_size - 1)
					return (haystack + i);
			}
		}
	}
	return (NULL);
}

std::string
ByteArray::substr(size_type pos, size_type len) throw(std::out_of_range) {
	size_t size = this->size(), definite_len;
	std::string new_str;

	if (pos >= size)
		throw(std::out_of_range("pos >= size"));
	definite_len = len == npos ? size - pos : len;
	new_str.reserve(definite_len);
	for(size_t i = 0; i < definite_len && _byte_array[pos + i] != '\0'; i++)
		new_str.push_back(_byte_array[pos + i]);
	return new_str;
}

ByteArray
ByteArray::sub_byte_array(size_type pos, size_type len) throw(std::out_of_range) {
	size_t size = this->size(), definite_len;

	if (pos >= size)
		throw(std::out_of_range("pos >= size"));
	definite_len = len == npos ? size - pos : len;
	return (ByteArray(&(*(_byte_array.begin() + pos)), definite_len));
}

ByteArray&
ByteArray::operator+=(const ByteArray& rhs) {
	this->append(rhs);
	return *this;
}

ByteArray&
ByteArray::operator+=(const std::string& rhs) {
	this->append(rhs);
	return *this;
}

ByteArray
operator+(const ByteArray& lhs, const ByteArray& rhs) {
	ByteArray ret(lhs);

	ret += rhs;
	return ret;
}

ByteArray
operator+(const ByteArray& lhs, const std::string& rhs) {
	ByteArray ret(lhs);

	ret += rhs;
	return ret;
}

std::ostream&
operator<<(std::ostream& o, const ByteArray& src) {
	size_t size = src.size();

	for(size_t i = 0; i < size; i++)
		o << src[i];
	return o;
}
