/*
 * Reef - Cross Platform Git Client
 * Copyright (C) 2020-2021 Emmanuel Mathi-Amorim
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

#include <QString>

/*
 * UTF8 Decoder: See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
 *
 * Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

constexpr uint32_t UTF8_ACCEPT = 0;
constexpr uint32_t UTF8_REJECT = 1;

static const uint8_t utf8d[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
	8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
	0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
	0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
	0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
	1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
	1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
	1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

uint32_t inline utf8_decode(uint32_t* state, uint32_t* codepoint, uint32_t byte) {
	uint32_t type = utf8d[byte];
	*codepoint = (*state != UTF8_ACCEPT) ? (byte & 0x3Fu) | (*codepoint << 6) : (0xFF >> type) & (byte);
	*state = utf8d[256 + *state * 16 + type];
	return *state;
}

/* end UTF8 decoder */

/* appends the UTF8 string str to the buffer buf with size n at the position i updating i to the next available position */
template<size_t BUF_SIZE>
static inline void add_utf8_str_to_buf(QChar(&buf)[BUF_SIZE], const char str[], size_t &i)
{
	uint32_t state = UTF8_ACCEPT;
	uint32_t codepoint;
	for (size_t j = 0; i < BUF_SIZE && str[j]; ++j) {
		if (utf8_decode(&state, &codepoint, reinterpret_cast<const uint8_t&>(str[j])))
			continue;

		if (codepoint <= 0xFFFF) {
			buf[i++] = codepoint;
		} else {
			if (i > BUF_SIZE - 1)
				break;

			buf[i++] = 0xD7C0 + (codepoint >> 10);
			buf[i++] = 0xDC00 + (codepoint & 0x3FF);
		}
	}
}

/* appends the UTF8 string str with size n to the buffer buf with size n at the position i updating i to the next available position */
template<size_t BUF_SIZE>
static inline void add_utf8_str_to_buf(QChar(&buf)[BUF_SIZE], const char str[], size_t n, size_t &i)
{
	uint32_t state = UTF8_ACCEPT;
	uint32_t codepoint;
	for (size_t j = 0; i < BUF_SIZE && j < n; ++j) {
		if (utf8_decode(&state, &codepoint, reinterpret_cast<const uint8_t&>(str[j])))
			continue;

		if (codepoint <= 0xFFFF) {
			buf[i++] = codepoint;
		} else {
			if (i > BUF_SIZE - 1)
				break;

			buf[i++] = 0xD7C0 + (codepoint >> 10);
			buf[i++] = 0xDC00 + (codepoint & 0x3FF);
		}
	}
}

/* appends the string str to the buffer buf with size n at the position i updating i to the next available position */
template<size_t BUF_SIZE>
static inline void add_str_to_buf(QChar(&buf)[BUF_SIZE], const QChar str[], size_t &i)
{
	size_t j = 0;
	for (; i < BUF_SIZE && str[j] != u'\0'; ++i, ++j)
		buf[i] = str[j];
}

/* appends the string str with size n to the buffer buf with size n at the position i updating i to the next available position */
template<size_t BUF_SIZE>
static inline void add_str_to_buf(QChar(&buf)[BUF_SIZE], const QChar str[], size_t n, size_t &i)
{
	size_t j = 0;
	for (; i < BUF_SIZE && j < n; ++i, ++j)
		buf[i] = str[j];
}

#endif /* REEF_STRING_H */
