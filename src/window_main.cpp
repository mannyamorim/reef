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

#include <climits>

#include <git2.h>

#include "chtype_buf_utilities.h"
#include "commit_list.h"
#include "ref_map.h"
#include "scroll_window.h"
#include "graph.h"
#include "window_main.h"
#include "window_commit.h"

constexpr int BASE_NUM_OF_COMMITS = 256;

static inline void add_str_to_vec_attr(std::vector<chtype> &chbuf, const char *str, chtype attr)
{
	for (size_t i = 0; str[i] != '\0'; i++)
		chbuf.push_back(static_cast<chtype>(str[i]) | attr);
}

static inline void add_str_to_vec(std::vector<chtype> &chbuf, const char *str)
{
	for (size_t i = 0; str[i] != '\0'; i++)
		chbuf.push_back(static_cast<chtype>(str[i]));
}

static void draw_commit(scroll_window<git::commit> &swin, git::commit &&commit,
	struct commit_graph_info &graph, graph_list &graph_list, ref_map &refs)
{
	const git_signature *author = commit.author();
	struct tm *author_time = localtime((time_t *)&(author->when.time));

	char time_buf[20];
	strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", author_time);

	std::vector<chtype> chbuf;

	chtype graphbuf[256];
	size_t graph_size = graph_list.compute_graph(graph, graphbuf);

	for (size_t i = 0; i < graph_size; i++)
		chbuf.push_back(graphbuf[i]);

	if (refs.refs.count(*commit.id()) > 0) {
		auto ref_range = refs.refs.equal_range(*commit.id());
		for (auto &it = ref_range.first; it != ref_range.second; it++) {
			constexpr chtype attr = COLOR_PAIR(2) | A_BOLD;
			chbuf.push_back(static_cast<chtype>('[') | attr);
			add_str_to_vec_attr(chbuf, it->second.shorthand(), attr);
			chbuf.push_back(static_cast<chtype>(']') | attr);
			chbuf.push_back(static_cast<chtype>(' '));
		}
	}

	add_str_to_vec(chbuf, time_buf);
	add_str_to_vec(chbuf, " ");
	add_str_to_vec(chbuf, commit.summary());

	swin.add_line(chbuf.data(), chbuf.size(), std::move(commit));
}

void window_main::display_commits(int max)
{
	while (!clist.empty() && (num_of_commits_loaded < max)) {
		struct commit_graph_info graph;
		git::commit commit = clist.get_next_commit(graph);

		commit_id_line_map.insert(std::make_pair(*commit.id(), num_of_commits_loaded++));
		draw_commit(primary_window, std::move(commit), graph, glist, refs);
	}
}

void window_main::display_refs()
{
	for (auto it : refs.refs_ordered) {
		chtype buf[REFS_LINE_LENGTH];
		size_t i = 0;
		add_str_to_buf(buf, it.first, i);
		terminate_buf(buf, i);
		refs_window.add_line(buf, REFS_LINE_LENGTH, it.second);
	}
}

window_main::window_main(const git::repository &repo, const preferences &prefs) :
	repo(repo),
	prefs(prefs),
	refs(repo),
	primary_window(LINES, COLS - 40, 0, 40),
	refs_window(LINES, 40, 0, 0),
	commit_id_line_map(),
	clist(refs, repo),
	glist()
{
	display_refs();
	display_commits(BASE_NUM_OF_COMMITS);
}

void window_main::finish_loading()
{
	display_commits(INT_MAX);
}

window_main::~window_main() {}

void window_main::refresh()
{
	primary_window.noutrefresh();
	refs_window.noutrefresh();
	cpp_curses::curses_mode::_doupdate();
}

void window_main::resize()
{
	primary_window.resize(LINES, COLS - 40);
	refs_window.resize(LINES, 40);
}

void window_main::redraw()
{
	primary_window.redraw();
	refs_window.redraw();
}

window_base::input_response window_main::process_key_input(int key)
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
				primary_window.adjust_selected_line(-prefs.lines_mouse_scroll);
				primary_window.refresh();
			} else {
				refs_window.adjust_selected_line(-prefs.lines_mouse_scroll);
				refs_window.refresh();
			}
		} else if (event.scroll_down) {
			if (event.x > 40 || event.x == -1) {
				primary_window.adjust_selected_line(+prefs.lines_mouse_scroll);
				primary_window.refresh();
			} else {
				refs_window.adjust_selected_line(+prefs.lines_mouse_scroll);
				refs_window.refresh();
			}
		}

		if (event.button_1_clicked) {
			if (event.x > 40) {
				int clicked_line = primary_window.get_current_line() + event.y;
				primary_window.change_selected_line(clicked_line);
				primary_window.refresh();
			} else {
				int clicked_line = refs_window.get_current_line() + event.y;
				refs_window.change_selected_line(clicked_line);
				refs_window.refresh();
			}
		} else if (event.button_1_double_clicked) {
			if (event.x > 40) {
				int clicked_line = primary_window.get_current_line() + event.y;
				primary_window.change_selected_line(clicked_line);
				res.type = input_response::type::OPEN_WINDOW;
				res.window = std::make_shared<window_commit>(repo, prefs, primary_window[clicked_line]);
			} else {
				int clicked_line = refs_window.get_current_line() + event.y;
				refs_window.change_selected_line(clicked_line);

				const git_oid &id = refs_window[(refs_window.get_selected_line())]->first;
				auto it = commit_id_line_map[id];
				primary_window.change_selected_line(it);
				
				primary_window.noutrefresh();
				refs_window.noutrefresh();
				cpp_curses::curses_mode::_doupdate();
			}
		}
		return res;
	}
#endif /* REEF_MOUSE_SUPPORTED */
	case KEY_PPAGE:
		primary_window.adjust_selected_line(-prefs.lines_page_up_down);
		primary_window.refresh();
		return res;
	case KEY_NPAGE:
		primary_window.adjust_selected_line(+prefs.lines_page_up_down);
		primary_window.refresh();
		return res;
	case KEY_UP:
	case 'j':
		primary_window.adjust_selected_line(-1);
		primary_window.refresh();
		return res;
	case KEY_DOWN:
	case 'k':
		primary_window.adjust_selected_line(+1);
		primary_window.refresh();
		return res;
	case 'w':
		refs_window.adjust_selected_line(-1);
		refs_window.refresh();
		return res;
	case 's':
		refs_window.adjust_selected_line(+1);
		refs_window.refresh();
		return res;
	case 'g':
	{
		const git_oid &id = refs_window[(refs_window.get_selected_line())]->first;
		auto it = commit_id_line_map[id];
		primary_window.change_selected_line(it);
		primary_window.refresh();
		return res;
	}
	case 'd':
		res.type = input_response::type::OPEN_WINDOW;
		res.window = std::make_shared<window_commit>(repo, prefs, primary_window[primary_window.get_selected_line()]);
		return res;
	default:
		return res;
	};
}

int window_main::_getch()
{
	return primary_window._getch();
}
