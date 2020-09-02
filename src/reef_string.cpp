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

#include <stdexcept>

#include "reef_string.h"

cchar_t line_drawing_empty;
cchar_t line_drawing_initial;

cchar_t* line_drawing_chars[num_line_drawing_chars];

void init_line_drawing_chars()
{
	pack_cchar_t(&line_drawing_empty, U" ", 0, 0);
	pack_cchar_t(&line_drawing_initial, U"I", 0, 0);

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
	size_t utf8_bytes_read = 0;
	size_t codepoints_decoded = 0;
	while (utf8_bytes_read < buf_len) {
		size_t res = utf8proc_iterate(buf + utf8_bytes_read, buf_len, codepoints + codepoints_decoded);
		if (res > 0) {
			utf8_bytes_read += res;
			codepoints_decoded++;
		} else {
			throw std::invalid_argument("print_wchtype_buf: error while decoding UTF8");
		}
	}

	/* normalize characters */
	codepoints_decoded = utf8proc_normalize_utf32(codepoints, codepoints_decoded, (utf8proc_option_t)(UTF8PROC_NLF2LF | UTF8PROC_COMPOSE));

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
				/* check for illegal box drawing flags which will cause a SEGFAULT */
				if (box_drawing_flags >= num_line_drawing_chars)
					throw std::invalid_argument("print_wchtype_buf: illegal box_drawing_flags found");

				memcpy(&dest[j], line_drawing_chars[box_drawing_flags], sizeof(cchar_t));
				set_cchar_t_attr(&dest[j], get_cchar_t_attr(&dest[j]) | attr | (color != 0 ? A_BOLD : 0));
				set_cchar_t_color(&dest[j], color);
				j++;
			}
		} else {
#ifdef PDCURSES
			constexpr size_t max_cchar_characters = 1;
#else
			constexpr size_t max_cchar_characters = CCHARW_MAX;
#endif // PDCURSES

			char32_t chars[max_cchar_characters] = { 0 };
			chars[0] = codepoints[i];
			size_t k = 1;

			/* look for additional combining characters */
			while (i + 1 < codepoints_decoded && !utf8proc_grapheme_break_stateful(codepoints[i], codepoints[i + 1], &grapheme_state)) {
				if (k < max_cchar_characters)
					chars[k++] = codepoints[i + 1];

				i++;
			}

			pack_cchar_t(&dest[j], chars, attr | (color != 0 ? A_BOLD : 0), color);
			j++;
		}
	}

	return j;
}
