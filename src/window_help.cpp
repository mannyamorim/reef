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

#include <git2.h>

#include "window_help.h"
#include "version.h"

window_help::window_help() :
	window(LINES, COLS, 0, 0)
{
	window.set_keypad(true);
	window.print("Welcome to reef!\n");

	/* get libgit version */
	int major, minor, rev;
	git_libgit2_version(&major, &minor, &rev);

	/* get curses version */
	const char *curses_ver = curses_version();

	/* print version info to screen */
	window.print("reef v%s, libgit v%d.%d.%d, %s\n", REEF_VER_DOT, major, minor, rev, curses_ver);
}

window_help::~window_help()
{
}

void window_help::refresh()
{
	window.refresh();
}

void window_help::resize()
{
	window.resize(LINES, COLS);
}

void window_help::redraw()
{
	window.redraw();
}

window_base::input_response window_help::process_key_input(int key)
{
	input_response res;
	res.type = input_response::type::NO_ACTION;
	return res;
}

int window_help::_getch()
{
	return window._getch();
}
