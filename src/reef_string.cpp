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

#include <utf8proc.h>

#include "reef_string.h"

cchar_t line_drawing_empty = { 0, { L' ', L'\0' }, 0 };
cchar_t line_drawing_initial = { 0, { L'I', L'\0' }, 0 };

cchar_t* line_drawing_chars[num_line_drawing_chars];

void init_line_drawing_chars()
{
	cchar_t *tmp[num_line_drawing_chars] = {
		&line_drawing_empty,   /* 00 = G_EMPTY                              */
		&line_drawing_empty,   /* 01 = G_LEFT                               */
		&line_drawing_empty,   /* 02 =          G_RIGHT                     */
		WACS_HLINE,            /* 03 = G_LEFT | G_RIGHT                     */

		&line_drawing_empty,   /* 04 =                    G_UPPER           */
		WACS_LRCORNER,         /* 05 = G_LEFT           | G_UPPER           */
		WACS_LLCORNER,         /* 06 =          G_RIGHT | G_UPPER           */
		WACS_BTEE,             /* 07 = G_LEFT | G_RIGHT | G_UPPER           */

		&line_drawing_empty,   /* 08 =                              G_LOWER */
		WACS_URCORNER,         /* 09 = G_LEFT                     | G_LOWER */
		WACS_ULCORNER,         /* 10 =          G_RIGHT           | G_LOWER */
		WACS_TTEE,             /* 11 = G_LEFT | G_RIGHT           | G_LOWER */

		WACS_VLINE,            /* 12 =                    G_UPPER | G_LOWER */
		WACS_RTEE,             /* 13 = G_LEFT           | G_UPPER | G_LOWER */
		WACS_LTEE,             /* 14 =          G_RIGHT | G_UPPER | G_LOWER */
		WACS_PLUS,             /* 15 = G_LEFT | G_RIGHT | G_UPPER | G_LOWER */

		WACS_BULLET,           /* 16 = G_MARK                               */
		&line_drawing_initial, /* 17 = G_INITIAL                            */
	};

	std::copy(tmp, tmp + num_line_drawing_chars, line_drawing_chars);
}

int print_wchtype_buf(cchar_t *dest, utf8proc_int32_t *codepoints, size_t dest_len, const char8_t *buf, size_t buf_len, attr_t attr)
{
	/* decode the UTF8 into UTF32 codepoints */
	ssize_t codepoints_decoded = utf8proc_decompose(buf, buf_len, codepoints, dest_len, UTF8PROC_COMPOSE);

	/* state needed to find grapheme cluster breaks */
	utf8proc_int32_t grapheme_state = 0;

	/* state to keep track of the current color */
	unsigned char color = 0;

	size_t i = 0, j = 0;
	for (; i < codepoints_decoded && j < dest_len; i++) {
		if (codepoints[i] >= 0x100000) {
			/* we found a marker to switch colors and potentially write out a box drawing character */
			unsigned char new_color;
			bool is_box_drawing_char;
			unsigned char box_drawing_flags;
			unpack_runchar(codepoints[i], new_color, is_box_drawing_char, box_drawing_flags);

			/* set the color */
			color = new_color;

			if (is_box_drawing_char) {
				memcpy(&dest[j], line_drawing_chars[box_drawing_flags], sizeof(cchar_t));
				dest[j].attr |= attr;
				dest[j].ext_color = color;
				j++;
			}
		} else {
			dest[j].ext_color = color;
			dest[j].attr = attr;
			dest[j].chars[0] = codepoints[i];
			dest[j].chars[1] = L'\0';
			j++;
		}
	}

	return j;
}
