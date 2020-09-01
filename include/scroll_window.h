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

#include <utf8proc.h>

#include "cpp_curses.h"
#include "reef_string.h"

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

		tmp_line_buf = std::make_unique<cchar_t[]>(MAX_LINE_LENGTH);
		tmp_codepoint_buf = std::make_unique<utf8proc_int32_t[]>(MAX_LINE_LENGTH);
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
	void add_line(const char8_t *str, int size, _T &&data)
	{
		char8_t *line_memory = get_memory_for_line(size);
		memcpy(line_memory, str, size * sizeof(char8_t));
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

	void change_current_and_selected_lines(int cline, int sline)
	{
		if (cline == current_line && sline == selected_line)
			return;

		if (cline >= num_lines)
			cline = num_lines - 1;
		if (cline < 0)
			cline = 0;

		if (sline < cline)
			sline = cline;
		if (sline >= cline + win_lines)
			sline = cline + win_lines - 1;
		if (sline >= num_lines)
			sline = num_lines - 1;

		const int old_cline = current_line;
		const int old_sline = selected_line;
		current_line = cline;
		selected_line = sline;

		if (old_cline != cline) {
			window._scrl(cline - old_cline);
			if (cline > old_cline) {
				redraw_line(sline);
				for (int i = old_cline + win_lines; i < current_line + win_lines; i++)
					redraw_line(i);
			} else {
				redraw_line(sline);
				for (int i = current_line; i < old_cline; i++)
					redraw_line(i);
			}
		}

		redraw_line(old_sline);
		redraw_line(sline);
	}

	void change_selected_line(int line)
	{
		if (line == selected_line)
			return;

		if (line >= current_line + win_lines)
			change_current_and_selected_lines(line - win_lines + 1, line);
		else if (line < current_line)
			change_current_and_selected_lines(line, line);
		else
			change_current_and_selected_lines(current_line, line);
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

	void change_current_line(int line)
	{
		if (line == current_line)
			return;

		if (selected_line < line)
			change_current_and_selected_lines(line, line);
		else if (selected_line >= line + win_lines)
			change_current_and_selected_lines(line, line + win_lines - 1);
		else
			change_current_and_selected_lines(line, selected_line);
	}

	void adjust_current_line(int delta)
	{
		int new_line = current_line + delta;

		if (new_line >= num_lines)
			new_line = num_lines - 1;

		if (new_line < 0)
			new_line = 0;

		change_current_line(new_line);
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

	std::vector<std::unique_ptr<char8_t[]>> blocks;
	std::vector<std::pair<char8_t *, int>> lines;
	std::vector<T> line_data;

	const int MAX_LINE_LENGTH = 1024;

	std::unique_ptr<cchar_t[]> tmp_line_buf;
	std::unique_ptr<utf8proc_int32_t[]> tmp_codepoint_buf;

	int block_usage = 0;	/* number of bytes of the current block that has been allocated */
	int curr_block = 0;	/* number of the current block */

	int num_lines = 0;	/* number of lines in total in the scroll window */
	int current_line = 0;	/* the number of the line at the top of the window */
	int selected_line = 0;	/* the number of the selected/highlighted line */

	int horiz_scroll = 0;	/* the amount of horizontal scroll that has been applied */

	void add_block()
	{
		blocks.push_back(std::make_unique<char8_t[]>(BLOCK_SIZE));
	}

	char8_t *get_memory_for_line(int size)
	{
		if (BLOCK_SIZE - block_usage < size) {
			add_block();
			block_usage = 0;
			curr_block++;
		}

		char8_t *line_memory = &blocks[curr_block][block_usage];
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
			constexpr attr_t attr = WA_REVERSE;

			const int decoded_size = print_wchtype_buf(tmp_line_buf.get(), tmp_codepoint_buf.get(), MAX_LINE_LENGTH, lines[line].first, lines[line].second, attr);

			for (size_t i = decoded_size; i < win_columns + horiz_scroll; i++) {
				tmp_line_buf[i].chars[0] = L' ';
				tmp_line_buf[i].chars[1] = L'\0';
				tmp_line_buf[i].attr = attr;
				tmp_line_buf[i].ext_color = 0;
			}

			window._mvwadd_wchnstr(line - current_line, 0, tmp_line_buf.get() + horiz_scroll, win_columns);
		} else {
			const int decoded_size = print_wchtype_buf(tmp_line_buf.get(), tmp_codepoint_buf.get(), MAX_LINE_LENGTH, lines[line].first, lines[line].second, 0);

			int chars_to_write = std::min(decoded_size - horiz_scroll, win_columns);
			if (chars_to_write < 0)
				chars_to_write = 0;

			window._mvwadd_wchnstr(line - current_line, 0, tmp_line_buf.get() + horiz_scroll, chars_to_write);
			if (chars_to_write != win_columns) {
				window._move(line - current_line, chars_to_write);
				window._clrtoeol();
			}
		}
	}
};

#endif /* SCROLL_WINDOW_H */
