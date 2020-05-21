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

/* window_help.h */
#ifndef WINDOW_HELP_H
#define WINDOW_HELP_H

#include "scroll_window.h"
#include "window_base.h"
#include "cpp_curses.h"

class window_help : public window_base
{
public:
	window_help();
	~window_help();

	void refresh() override;
	void resize() override;
	void redraw() override;

	input_response process_key_input(int key) override;
	int _getch() override;

private:
	cpp_curses::window window;
};

#endif /* WINDOW_HELP_H */
