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

#include <algorithm>
#include <memory>
#include <vector>
#include <cstring>
#include <utility>

#include "cpp_curses.h"

template <typename T>
class scroll_window
{
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

		tmp_line_buf = std::make_unique<chtype[]>(win_columns);
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

		tmp_line_buf = std::make_unique<chtype[]>(win_columns);

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
	void add_line(const chtype *str, int size, _T &&data)
	{
		chtype *line_memory = get_memory_for_line(size);
		memcpy(line_memory, str, size * sizeof(chtype));
		lines.push_back(std::make_pair(line_memory, size));
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
	
	void change_horiz_scroll(int hscroll)
	{
		if (horiz_scroll == hscroll)
			return;
		
		horiz_scroll = hscroll;
		redraw_lines();
	}
	
	void adjust_horiz_scroll(int delta)
	{
		int hscroll = horiz_scroll + delta;

		if (hscroll < 0)
			hscroll = 0;

		change_horiz_scroll(hscroll);
	}

private:
	const int BLOCK_SIZE = 65536;

	cpp_curses::window window;
	int win_lines, win_columns;

	std::vector<std::unique_ptr<chtype[]>> blocks;
	std::vector<std::pair<chtype *, int>> lines;
	std::vector<T> line_data;

	std::unique_ptr<chtype[]> tmp_line_buf;

	int block_usage = 0;	/* number of bytes of the current block that has been allocated */
	int curr_block = 0;	/* number of the current block */

	int num_lines = 0;	/* number of lines in total in the scroll window */
	int current_line = 0;	/* the number of the line at the top of the window */
	int selected_line = 0;	/* the number of the selected/highlighted line */
	
	int horiz_scroll = 0;	/* the amount of horizontal scroll that has been applied */

	void add_block()
	{
		blocks.push_back(std::make_unique<chtype[]>(BLOCK_SIZE));
	}

	chtype *get_memory_for_line(int size)
	{
		if (BLOCK_SIZE - block_usage < size) {
			add_block();
			block_usage = 0;
			curr_block++;
		}

		chtype *line_memory = &blocks[curr_block][block_usage];
		block_usage += size;
		return line_memory;
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

			const chtype *line_buf = lines[line].first;
			
			int i = 0;
			for (; i < std::min(lines[line].second - horiz_scroll, win_columns); i++)
				tmp_line_buf[i] = line_buf[i + horiz_scroll] | attr;
			for (; i < win_columns; i++)
				tmp_line_buf[i] = static_cast<chtype>(' ') | attr;

			window._mvaddchnstr(line - current_line, 0, tmp_line_buf.get(), win_columns);
		} else {
			int chars_to_write = std::min(lines[line].second - horiz_scroll, win_columns);
			if (chars_to_write < 0)
				chars_to_write = 0;
			
			window._mvaddchnstr(line - current_line, 0, lines[line].first + horiz_scroll, chars_to_write);
			if (chars_to_write != win_columns) {
				window._move(line - current_line, chars_to_write);
				window._clrtoeol();
			}
		}
	}
};

#endif /* SCROLL_WINDOW_H */
