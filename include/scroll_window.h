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

/* scroll_window.h */
#ifndef SCROLL_WINDOW_H
#define SCROLL_WINDOW_H

#include <vector>
#include <cstring>
#include <utility>

#include "cpp_curses.h"

constexpr bool is_powerof2(int v)
{
	return v && ((v & (v - 1)) == 0);
}

template <int LINE_SIZE, typename T>
class scroll_window
{
	static_assert(is_powerof2(LINE_SIZE), "error LINE_SIZE should be a power of 2");

public:
	scroll_window(int nlines, int ncols, int begin_y, int begin_x) :
		window(nlines, ncols, begin_y, begin_x)
	{
		window.set_keypad(true);
		window.set_idlok(true);
		window.set_scrollok(true);

		add_block();
		win_lines = nlines;
		win_columns = ncols;
	}

	~scroll_window()
	{
		for (auto it : blocks)
			delete[] it;
		blocks.clear();
	}

	T &operator[](std::size_t idx)
	{
		return line_data[idx];
	}

	const T &operator[](std::size_t idx) const
	{
		return line_data[idx];
	}

	void resize(int lines, int columns)
	{
		window.resize(lines, columns);

		const bool selected_line_out_of_bounds = (selected_line >= current_line + lines);
		const bool redraw_needed = (lines > win_lines) || (columns > win_columns);

		win_lines = lines;
		win_columns = columns;

		if (selected_line_out_of_bounds)
			change_selected_line(current_line + lines - 1);
		else if (redraw_needed)
			redraw_lines();
	}

	void move(int y, int x)
	{
		window.mv(y, x);
	}

	void refresh()
	{
		window.refresh();
	}

	void noutrefresh()
	{
		window.noutrefresh();
	}

	void redraw()
	{
		window.redraw();
	}

	template<typename _T>
	void add_line(const chtype *str, _T &&data)
	{
		if (num_lines >= blocks.size() * LINES_PER_BLOCK)
			add_block();
		memcpy(get_line_ptr(num_lines), str, LINE_SIZE * sizeof(chtype));
		line_data.push_back(std::forward<_T>(data));

		num_lines++;
		redraw_line(num_lines - 1);
	}

	void clear()
	{
		num_lines = 0;
		current_line = 0;
		selected_line = 0;
		
		blocks.clear();
		line_data.clear();

		add_block();

		window.clear();

		redraw_lines();
	}

	int get_num_lines() const noexcept
	{
		return num_lines;
	}

	int get_current_line() const noexcept
	{
		return current_line;
	}

	int get_selected_line() const noexcept
	{
		return selected_line;
	}

	int _getch()
	{
		return window._getch();
	}

	void change_selected_line(int line)
	{
		if (line == selected_line)
			return;

		const int tmp = selected_line;
		selected_line = line;

		if (selected_line >= current_line + win_lines) {
			const int old = current_line;
			current_line = selected_line - win_lines + 1;

			window._scrl(current_line - old);

			redraw_line(tmp);
			for (int i = old + win_lines; i < current_line + win_lines; i++)
				redraw_line(i);
		} else if (selected_line < current_line) {
			const int old = current_line;
			current_line = selected_line;

			window._scrl(current_line - old);

			redraw_line(tmp);
			for (int i = current_line; i < old; i++)
				redraw_line(i);
		} else {
			redraw_line(tmp);
			redraw_line(selected_line);
		}
	}

	void adjust_selected_line(int delta)
	{
		int new_line = selected_line + delta;

		if (new_line >= num_lines)
			new_line = num_lines - 1;

		if (new_line < 0)
			new_line = 0;

		change_selected_line(new_line);
	}

private:
	const int BLOCK_SIZE = 65536;
	const int LINES_PER_BLOCK = BLOCK_SIZE / (LINE_SIZE * sizeof(chtype));

	cpp_curses::window window;
	int win_lines, win_columns;

	std::vector<chtype *> blocks;
	std::vector<T> line_data;

	int num_lines = 0;	/* number of lines in total in the scroll window */
	int current_line = 0;	/* the number of the line at the top of the window */
	int selected_line = 0;	/* the number of the selected/highlighted line */

	void add_block()
	{
		chtype *block = new chtype[BLOCK_SIZE];
		blocks.push_back(block);
	}

	chtype *get_line_ptr(int line)
	{
		chtype *block = blocks[line / LINES_PER_BLOCK];
		return block + ((line % LINES_PER_BLOCK) * LINE_SIZE);
	}

	void redraw_lines()
	{
		for (int i = 0; i < win_lines; i++)
			redraw_line(i + current_line);
	}

	void redraw_line(int line)
	{
		if (line < current_line || line >= current_line + win_lines)
			return;

		if (line < 0 || line >= num_lines)
			return;

		if (line == selected_line) {
			constexpr chtype attr = A_REVERSE;

			chtype tmp[LINE_SIZE];
			const chtype *line_buf = get_line_ptr(line);
			for (int i = 0; i < LINE_SIZE; i++)
				tmp[i] = line_buf[i] | attr;

			window._mvaddchnstr(line - current_line, 0, tmp, LINE_SIZE);
		} else {
			window._mvaddchnstr(line - current_line, 0, get_line_ptr(line), LINE_SIZE);
		}
	}
};

#endif /* SCROLL_WINDOW_H */
