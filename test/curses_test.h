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

/* curses_test.h */
#ifndef CURSES_TEST_H
#define CURSES_TEST_H

typedef uint32_t chtype;

enum ACS_CHARS : chtype
{
	ACS_LRCORNER = 1,
	ACS_URCORNER,
	ACS_ULCORNER,
	ACS_LLCORNER,
	ACS_PLUS,
	ACS_LTEE,
	ACS_RTEE,
	ACS_BTEE,
	ACS_TTEE,
	ACS_HLINE,
	ACS_VLINE,
	ACS_BULLET,
};

#define COLOR_PAIR(x) 0
#define A_BOLD 0

#endif /* CURSES_TEST_H */
