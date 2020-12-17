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

/* reef_string.h */
#ifndef REEF_STRING_H
#define REEF_STRING_H

#include <stdexcept>
#include <vector>

#include <curses.h>
#include <utf8proc.h>

#include <cstring>

/*
 * In reef we are using a run based encoding method to keep track of colors.
 * We are using UTF-8 to store all of the internal strings and we are using
 * the Unicode private use area characters to represent a change in color.
 *
 * Specifically we are using the Supplementary Private Use Area-B (plane 16).
 * With Unicode we have 5 bits for the plane number and then 16 bits for the characters within the plane.
 * We will pack the 16 bits as follows:
 *
 * |15|14|..|9|8|  7 | 6 |           5           |  4  |  3  |  2  |  1  |  0  |
 * | color pair | unused | is a box drawing char | box drawing character flags |
 *
 */

#if __cplusplus < 202002L
typedef unsigned char char8_t;
#endif

constexpr void pack_runchar(char8_t (&utf)[4], unsigned char color, bool is_box_drawing_char, unsigned char box_drawing_flags)
{
	/*
	 * From UTF=8 standard
	 * characters in range U+10000 to U+10FFFF are represented as:
	 * |11110xxx|10xxxxxx|10xxxxxx|10xxxxxx|
	 *
	 * Our run characters are in the PUA-B (plane 16) so the top 5 bits are 16 (10000).
	 */
	utf[0] = 0b11110100;
	utf[1] = 0b10000000 | ((color & 0b11110000) >> 4);
	utf[2] = 0b10000000 | ((color & 0b00001111) << 2);
	utf[3] = 0b10000000 | (is_box_drawing_char << 5) | box_drawing_flags;
}

constexpr void unpack_runchar(char32_t utf32, unsigned char &color, bool &is_box_drawing_char, unsigned char &box_drawing_flags)
{
	color = (utf32 & 0xFF00) >> 8;
	is_box_drawing_char = (utf32 & 0b100000) >> 5;
	box_drawing_flags = utf32 & 0b11111;
}

static inline void push_string_to_vec(std::vector<char8_t> &buf, const char8_t str[], size_t size)
{
	for (size_t i = 0; i < size; i++)
		buf.push_back(str[i]);
}

static inline void push_string_to_vec(std::vector<char8_t> &buf, const char8_t str[])
{
	for (size_t i = 0; str[i] != '\0'; i++)
		buf.push_back(str[i]);
}


/* appends the string str to the buffer buf with size n at the position i updating i to the next available position */
template<size_t BUF_SIZE>
static inline void add_str_to_buf(char8_t(&buf)[BUF_SIZE], const char str[], size_t &i)
{
	size_t j = 0;
	for (; i < BUF_SIZE && str[j] != '\0'; ++i, ++j)
		buf[i] = str[j];
}

/* appends the string str with size n to the buffer buf with size n at the position i updating i to the next available position */
template<size_t BUF_SIZE>
static inline void add_str_to_buf(char8_t(&buf)[BUF_SIZE], const char str[], size_t n, size_t &i)
{
	size_t j = 0;
	for (; i < BUF_SIZE && j < n; ++i, ++j)
		buf[i] = str[j];
}

static inline void pack_cchar_t(cchar_t *cchar, const char32_t *str, const attr_t attrs, const short color_pair)
{
#ifdef PDCURSES
        *cchar = str[0] | attrs | COLOR_PAIR(color_pair);
#else
        setcchar(cchar, (const wchar_t *)str, attrs, color_pair, nullptr);
#endif /* PDCURSES */
}

static inline attr_t get_cchar_t_attr(cchar_t *cchar)
{
#ifdef PDCURSES
        return *cchar & (A_ATTRIBUTES & ~A_COLOR);
#else
        return cchar->attr;
#endif /* PDCURSES */
}

constexpr void set_cchar_t_attr(cchar_t *cchar, const attr_t attrs)
{
#ifdef PDCURSES
        *cchar = (*cchar & ~(A_ATTRIBUTES & ~A_COLOR)) | attrs;
#else
        cchar->attr = attrs;
#endif /* PDCURSES */
}

constexpr void set_cchar_t_color(cchar_t *cchar, const short color_pair)
{
#ifdef PDCURSES
        *cchar = (*cchar & ~A_COLOR) | COLOR_PAIR(color_pair);
#else
        cchar->ext_color = color_pair;
#endif /* PDCURSES */
}

constexpr unsigned char G_EMPTY = 0x00;
constexpr unsigned char G_LEFT  = 0x01;
constexpr unsigned char G_RIGHT = 0x02;
constexpr unsigned char G_UPPER = 0x04;
constexpr unsigned char G_LOWER = 0x08;
constexpr unsigned char G_MARK  = 0x10;
constexpr unsigned char G_INITIAL = 0x11;

constexpr size_t num_line_drawing_chars = 18;
extern cchar_t* line_drawing_chars[num_line_drawing_chars];

void init_line_drawing_chars();

int print_wchtype_buf(cchar_t *dest, utf8proc_int32_t *codepoints, size_t dest_len, const char8_t *buf, size_t buf_len, attr_t attr);

#endif /* REEF_STRING_H */
