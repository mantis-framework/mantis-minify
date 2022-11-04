/**
 *  replacement for std::string
 */

#ifndef MINIFY_STRING_H
#define MINIFY_STRING_H

#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <algorithm>

namespace minify {
	namespace type {
		class string {
			static constexpr std::size_t CHAR_SIZE = sizeof(char);
			static constexpr double RESIZE_MULTIPLIER = 1.25;

			bool allocated_  = 0;
			std::ptrdiff_t length_   = 0, 
			               capacity_ = 0;
			char* c_str_ = nullptr;

			void resize_capacity(
				std::ptrdiff_t const& capacity
			) {
				capacity_ = RESIZE_MULTIPLIER*capacity + 1;
			}

			void allocate(std::ptrdiff_t const& capacity) {
				assert(!allocated_);
				allocated_ = 1;
				capacity_  = std::max<std::ptrdiff_t>(1, capacity);
				c_str_ = (char*) malloc(
					CHAR_SIZE*capacity_
				);
				c_str_[capacity_-1] = '\0';
			}

			void reallocate(std::ptrdiff_t const& capacity) {
				assert(allocated_);
				if(capacity_ != capacity) {
					capacity_ = std::max<std::ptrdiff_t>(1, capacity);
					c_str_ = (char*) realloc(
						c_str_, 
						CHAR_SIZE*capacity_
					);
					c_str_[capacity_-1] = '\0';
				}
			}

			public:
			string() {
			}

			string(
				std::ptrdiff_t const& capacity
			) {
				strict_resize(capacity);
			}

			string(
				char const* c_str_in, 
				bool const& from_file = 0, 
				std::ptrdiff_t const& capacity = 0
			) {
				set(
					c_str_in, 
					from_file, 
					capacity
				);
			}


			string& substr(
				string const& str,
				std::ptrdiff_t const& pos,
				std::ptrdiff_t const& length
			) {
				assert(str.allocated());
				length_ = length;
				if(capacity_ <= length_)
					strict_resize(length_+1);

				memcpy(
					c_str_,
					&str[pos],
					length
				);
				c_str_[length_] = '\0';

				return *this;
			}

			void append_substr(
				string const& str,
				std::ptrdiff_t const& pos,
				std::ptrdiff_t const& length
			) {
				assert(str.allocated());
				if(capacity_ <= length_ + length)
					strict_resize(length_ + length + 1);

				memcpy(
					&c_str_[length_],
					&str[pos],
					length
				);
				length_ += length;
				c_str_[length_] = '\0';
			}

			~string() {
				if(allocated_)
				{
					allocated_ = 0;
					length_ = capacity_ = 0;
					free(c_str_);
				}
			}

			void drop() {
				if(allocated_)
				{
					free(c_str_);
					allocated_ = 0;
					length_ = capacity_ = 0;
				}
			}

			void strict_resize(
				int const& capacity
			) {
				if(!allocated_) {
					if(capacity)
						allocate(capacity);
				}
				else if(capacity_ != capacity)
					reallocate(capacity);
			}

			void smart_resize(
				std::ptrdiff_t const& capacity
			) {
				if(!allocated_) {
					if(capacity)
						allocate(capacity);
				}
				else if(capacity_ != capacity) {
					resize_capacity(capacity);
					reallocate(capacity);
				}
			}

			void save(char const* path) {
				FILE* f = fopen(path, "w");

				fputs(
					c_str_,
					f
				);

				fclose(f);
				f = nullptr;
			}

			void load_file(
				char const* path,
				std::ptrdiff_t const& capacity = 0,
				std::ptrdiff_t const& length   = 0
			) {
				FILE* f = fopen(path, "r");

				if(length) 
					length_ = length;
				else {
					fseek(f, 0, SEEK_END);
					length_ = ftell(f);
					fseek(f, 0, SEEK_SET);
					//rewind(f); //fseek alternative
				}

				
				if(capacity) {
					strict_resize(capacity);
					if(capacity_ <= length_)
						length_ = capacity_ - 1;
				}
				else if(capacity_ <= length_)
					strict_resize(length_ + 1);

				length_ = fread(
					c_str_, 
					CHAR_SIZE, 
					length_, 
					f
				);
				c_str_[length_] = '\0';
				fclose(f);
			}

			void set(string const& cstr) {
				strict_resize(cstr.capacity());
				length_ = cstr.length();

				if(capacity_)
				{
					memcpy(
						c_str_, 
						cstr.c_str(), 
						length_
					);
					c_str_[length_] = '\0';
				}
			}

			string& assign(char const* c_str_in) {
				length_ = strlen(c_str_in);
				//strict_resize(length_+1);
				smart_resize(length_+1);

				memcpy(
					c_str_, 
					c_str_in, 
					length_
				);
				c_str_[length_] = '\0';

				return *this;
			}			

			void set(
				char const* c_str_in, 
				bool const& from_file = 0, 
				std::ptrdiff_t const& capacity  = 0
			) {
				if(from_file)
					load_file(c_str_in, capacity);
				else {
					length_ = std::ptrdiff_t(strlen(c_str_in));

					if(!capacity)
						strict_resize(length_ + 1);
					else
						strict_resize(capacity);

					if(capacity_ <= length_)
						length_ = capacity_-1;

					memcpy(
						c_str_, 
						c_str_in, 
						length_
					);
					c_str_[length_] = '\0';
				}
			}

			std::ptrdiff_t ext_pos() {
				std::ptrdiff_t pos = length_;
				while(length_)
					if(c_str_[--pos] == '.')
						return pos;

				return length_-1;
			}

			char const* c_str() const {
				return c_str_;
			}

			std::ptrdiff_t const& length() const {
				return length_;
			}

			void length(std::ptrdiff_t const& new_length) {
				length_ = new_length;

				if(capacity_ <= length_)
					strict_resize(length_+1);
				
				c_str_[length_] = '\0';
			}

			std::ptrdiff_t const& size() const {
				return length_;
			}

			bool empty() const {
				return !length_;
			}

			bool const& allocated() const {
				return allocated_;
			}

			std::ptrdiff_t const& capacity() const {
				return capacity_;
			}

			char& operator[](std::ptrdiff_t const& i) const {
				return c_str_[i];
			}

			void replace(
				std::ptrdiff_t const& pos, 
				std::ptrdiff_t const& replace_length,
				char const* replacement,
				std::ptrdiff_t const& replacement_length,
				bool const& quoted = 0
			) {
				std::ptrdiff_t actual_replacement_length = (quoted)
					? replacement_length + 2
					: replacement_length;

				length_ += (
					actual_replacement_length - 
					replace_length
				);

				if(!capacity_) 
					strict_resize(actual_replacement_length+1);
				else if(capacity_ <= length_)
					smart_resize(length_+1);

				if(replace_length > actual_replacement_length)
					memcpy(
						&c_str_[pos + actual_replacement_length],
						&c_str_[pos + replace_length],
						length_ - pos - actual_replacement_length
					);
				else if(replace_length < actual_replacement_length)
					memmove(
						&c_str_[pos + actual_replacement_length],
						&c_str_[pos + replace_length],
						length_ - pos - actual_replacement_length
					);

				c_str_[length_] = '\0';

				if(quoted) {
					c_str_[pos] = '"';
					memcpy(
						&c_str_[pos+1], 
						&replacement[0], 
						replacement_length
					);
					c_str_[pos+replacement_length+1] = '"';
				}
				else
					memcpy(
						&c_str_[pos], 
						&replacement[0], 
						replacement_length
					);

				//if(dynamic_capacity_ && 4*length_ < capacity_)
				//	smart_resize();
			}

			void replace(
				std::ptrdiff_t const& pos, 
				std::ptrdiff_t const& replace_length,
				std::string const& replacement,
				std::ptrdiff_t const& replacement_length = 0,
				bool const& quoted = 0
			) {
				if(replacement_length)
					replace(
						pos, 
						replace_length, 
						replacement.c_str(), 
						replacement_length, 
						quoted
					);
				else
					replace(
						pos, 
						replace_length, 
						replacement.c_str(), 
						replacement.size(),
						quoted);
			}

			friend bool operator==(
				string const& cstr1, 
				string const& cstr2
			) {
				if(cstr1.length_ != cstr2.length_)
					return 0;
				else if(!cstr1.length_)
					return 1;

				return !strcmp(
					cstr1.c_str_, 
					cstr2.c_str_
				);
			}

			friend bool operator==(
				string const& cstr, 
				char   const* c_str
			) {
				if(std::size_t(cstr.length_) != strlen(c_str))
					return 0;
				else if(!cstr.length_)
					return 1;

				return !strcmp(
					cstr.c_str_, 
					c_str
				);
			}

			friend bool operator==(
				char   const* c_str,
				string const& cstr
			) {
				return (cstr == c_str);
			}

			friend std::ostream& operator<<(
				std::ostream& os, 
				string const& cstr
			) {
				if(
					cstr.allocated_ &&
					cstr.capacity_  &&
					cstr.length_
				)
					os << cstr.c_str_;

				return os;
			}
		};
	}
}

#endif //MINIFY_STRING_H