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

/* window_base.h */
#ifndef WINDOW_BASE_H
#define WINDOW_BASE_H

#include <memory>

class window_base
{
public:
	struct input_response
	{
		enum type
		{
			NO_ACTION,
			OPEN_WINDOW,
		} type;

		std::shared_ptr<window_base> window;
	};

	virtual void refresh() = 0;
	virtual void resize() = 0;
	virtual void redraw() = 0;

	virtual input_response process_key_input(int key) = 0;
	virtual bool has_background_work() { return false; }
	virtual bool do_background_work() { return false; }
	virtual int _getch(bool block) = 0;
};

#endif /* WINDOW_BASE_H */
