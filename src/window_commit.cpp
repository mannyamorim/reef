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

#include <algorithm>

#include <git2.h>

#include "chtype_buf_utilities.h"
#include "window_commit.h"

window_commit::window_commit(const git::repository &repo, const preferences &prefs, const git::commit &commit) :
	repo(repo),
	prefs(prefs),
	commit(commit),
	tree_a(nullptr),
	tree_b(nullptr),
	diff(nullptr),
	file_window(LINES, 40, 0, 0),
	line_window(LINES, COLS - 40, 0, 40)
{
	tree_a = commit.tree();

	if (commit.parentcount() != 0)
		tree_b = commit.parent(0).tree();

	diff = repo.diff_tree_to_tree(tree_b, tree_a, nullptr);

	const auto print_cb = [this](const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line) {
		return this->process_line_callback(delta, hunk, line);
	};
	diff.print(GIT_DIFF_FORMAT_PATCH, print_cb);
}

int window_commit::process_line_callback(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line)
{
	chtype buf[MAIN_LINE_LENGTH];
	size_t i = 0, header_size = 0;
	if (line->origin == GIT_DIFF_LINE_CONTEXT
		|| line->origin == GIT_DIFF_LINE_ADDITION
		|| line->origin == GIT_DIFF_LINE_DELETION) {
		buf[i++] = line->origin;
		header_size = 1;
	}
	
	chtype attr = 0;
	switch (line->origin) {
	case GIT_DIFF_LINE_ADDITION:
		attr = COLOR_PAIR(2) | A_BOLD;
		break;
	case GIT_DIFF_LINE_DELETION:
		attr = COLOR_PAIR(3) | A_BOLD;
		break;
	case GIT_DIFF_LINE_FILE_HDR:
		attr = COLOR_PAIR(6) | A_BOLD;
		break;
	case GIT_DIFF_LINE_HUNK_HDR:
		attr = COLOR_PAIR(4) | A_BOLD;
		break;
	}

	int pos = line_window.get_num_lines();

	size_t j = 0;
	while(j < line->content_len) {
		if (line->content[j] == '\n' || i == MAIN_LINE_LENGTH) {
			/* start a new line */
			terminate_buf(buf, i);
			line_window.add_line(buf, MAIN_LINE_LENGTH, line);
			i = 0;
			j++;
		} else if (line->content[j] == '\t') {
			/* handle a tab character */
			const size_t num_spaces_requested = prefs.tab_length - ((i - header_size) % prefs.tab_length);
			const size_t available_space = MAIN_LINE_LENGTH - i;
			for (int t = 0; t < std::min(num_spaces_requested, available_space); t++)
				buf[i++] = static_cast<chtype>(' ');
			j++;
		} else {
			buf[i++] = static_cast<chtype>(line->content[j++]) | attr;
		}
	}

	if (line->origin == GIT_DIFF_LINE_FILE_HDR) {
		i = 0;
		add_str_to_buf(buf, delta->new_file.path, i);
		terminate_buf(buf, i);
		file_window.add_line(buf, MAIN_LINE_LENGTH, std::make_pair(delta, pos));
	}

	return 0;
}

void window_commit::refresh()
{
	file_window.noutrefresh();
	line_window.noutrefresh();
	cpp_curses::curses_mode::_doupdate();
}

void window_commit::resize()
{
	file_window.resize(LINES, 40);
	line_window.resize(LINES, COLS - 40);
}

void window_commit::redraw()
{
	file_window.redraw();
	line_window.redraw();
}

window_base::input_response window_commit::process_key_input(int key)
{
	input_response res;
	res.type = input_response::type::NO_ACTION;

	switch (key) {
#ifdef REEF_MOUSE_SUPPORTED
	case KEY_MOUSE:
	{
		cpp_curses::mouse_event event;
		if (event.scroll_up) {
			if (event.x > 40 || event.x == -1) {
				line_window.adjust_selected_line(-prefs.lines_mouse_scroll);
				line_window.refresh();
			} else {
				file_window.adjust_selected_line(-prefs.lines_mouse_scroll);
				file_window.refresh();
			}
		} else if (event.scroll_down) {
			if (event.x > 40 || event.x == -1) {
				line_window.adjust_selected_line(+prefs.lines_mouse_scroll);
				line_window.refresh();
			} else {
				file_window.adjust_selected_line(+prefs.lines_mouse_scroll);
				file_window.refresh();
			}
		}

		if (event.button_1_clicked) {
			if (event.x > 40) {
				int clicked_line = line_window.get_current_line() + event.y;
				line_window.change_selected_line(clicked_line);
				line_window.refresh();
			} else {
				int clicked_line = file_window.get_current_line() + event.y;
				file_window.change_selected_line(clicked_line);
				file_window.refresh();
			}
		} else if (event.button_1_double_clicked) {
			if (event.x > 40) {
				int clicked_line = line_window.get_current_line() + event.y;
				line_window.change_selected_line(clicked_line);
				line_window.refresh();
			} else {
				int clicked_line = file_window.get_current_line() + event.y;
				file_window.change_selected_line(clicked_line);

				line_window.change_selected_line(file_window[file_window.get_selected_line()].second);

				file_window.noutrefresh();
				line_window.noutrefresh();
				cpp_curses::curses_mode::_doupdate();
			}
		}
		return res;
	}
#endif /* REEF_MOUSE_SUPPORTED */
	case 'w':
		file_window.adjust_selected_line(-1);
		file_window.refresh();
		return res;
	case 's':
		file_window.adjust_selected_line(+1);
		file_window.refresh();
		return res;
	case 'g':
		line_window.change_selected_line(file_window[file_window.get_selected_line()].second);
		line_window.refresh();
		return res;
	case KEY_UP:
	case 'j':
		line_window.adjust_selected_line(-1);
		line_window.refresh();
		return res;
	case KEY_DOWN:
	case 'k':
		line_window.adjust_selected_line(+1);
		line_window.refresh();
		return res;
	case KEY_PPAGE:
		line_window.adjust_selected_line(-prefs.lines_page_up_down);
		line_window.refresh();
		return res;
	case KEY_NPAGE:
		line_window.adjust_selected_line(+prefs.lines_page_up_down);
		line_window.refresh();
		return res;
	default:
		return res;
	};
}

int window_commit::_getch()
{
	return file_window._getch();
}
