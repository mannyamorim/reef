/*
 * Reef - TUI Client for Git
 * Copyright (C) 2020 Emmanuel Mathi-Amorim
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/* chtype_buf_utilities.h */
#ifndef CHTYPE_BUF_UTILITIES_H
#define CHTYPE_BUF_UTILITIES_H

#include <curses.h>

/* appends the string str to the buffer buf with size n at the position i updating i to the next available position */
template<size_t BUF_SIZE> static inline
void add_str_to_buf(chtype(&buf)[BUF_SIZE], const char *str, size_t &i)
{
	size_t j = 0;
	for (; i < BUF_SIZE && str[j] != '\0'; ++i, ++j)
		buf[i] = static_cast<chtype>(str[j]);
}

/* appends the string str with size n to the buffer buf with size n at the position i updating i to the next available position */
template<size_t BUF_SIZE> static inline
void add_sized_str_to_buf_attr(chtype(&buf)[BUF_SIZE], const char *str, size_t n, const chtype attr, size_t &i)
{
	size_t j = 0;
	for (; i < BUF_SIZE && j < n; ++i, ++j)
		buf[i] = static_cast<chtype>(str[j]) | attr;
}

/* appends the chtype buffer str to the buffer buf with size n at the position i updating i to the next available position */
template<size_t BUF_SIZE> static inline
void add_str_to_buf_attr(chtype(&buf)[BUF_SIZE], const char *str, const chtype attr, size_t &i)
{
	size_t j = 0;
	for (; i < BUF_SIZE && str[j] != '\0'; ++i, ++j)
		buf[i] = static_cast<chtype>(str[j]) | attr;
}

/* writes spaces to the buffer buf with size n at the position i if there is space available */
template<size_t BUF_SIZE> static inline
void terminate_buf(chtype(&buf)[BUF_SIZE], size_t &i)
{
	for (; i < BUF_SIZE; ++i)
		buf[i] = static_cast<chtype>(' ');
}

#endif /* CHTYPE_BUF_UTILITIES_H */
