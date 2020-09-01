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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unordered_map>
#include <memory>

#include <git2.h>
#include <curses.h>

#include "cpp_git.h"
#include "cpp_curses.h"
#include "commit_list.h"
#include "reef_string.h"
#include "ref_map.h"
#include "scroll_window.h"
#include "graph.h"
#include "preferences.h"
#include "window_base.h"
#include "window_help.h"
#include "window_commit.h"
#include "window_main.h"

#define CTRL(k) ((k) & 0x1f)

void window_loop(const git::repository &repo, const preferences &prefs)
{
	/* initialize curses mode */
	cpp_curses::curses_mode curses_mode_lock;

	/* set the cursor to be invisible */
	curs_set(0);

	/* initialize array for the box drawing chars */
	init_line_drawing_chars();

	if (has_colors()) {
		start_color();
		use_default_colors();
		init_pair(1, COLOR_BLUE, -1);
		init_pair(2, COLOR_GREEN, -1);
		init_pair(3, COLOR_RED, -1);
		init_pair(4, COLOR_CYAN, -1);
		init_pair(5, COLOR_MAGENTA, -1);
		init_pair(6, COLOR_YELLOW, -1);
	}

	std::shared_ptr<window_main> main_window = std::make_shared<window_main>(repo, prefs);
	std::shared_ptr<window_help> help_window = std::make_shared<window_help>();
	std::shared_ptr<window_base> curr_window = main_window;

	curr_window->refresh();

	main_window->finish_loading();

	while (1) {
		/* wait for user input */
		int key = curr_window->_getch();

		switch (key) {
		case CTRL('c'):
			return;
		case KEY_RESIZE:
#ifdef PDCURSES
			resize_term(0, 0);
#endif // PDCURSES
			curr_window->resize();
			curr_window->refresh();

			if (curr_window != main_window)
				main_window->resize();
			if (curr_window != help_window)
				help_window->resize();
			break;
		case 'm':
			curr_window = main_window;
			curr_window->redraw();
			curr_window->refresh();
			break;
		case 'h':
			curr_window = help_window;
			curr_window->redraw();
			curr_window->refresh();
			break;
		default:
		{
			window_base::input_response res = curr_window->process_key_input(key);
			switch (res.type) {
			case window_base::input_response::type::OPEN_WINDOW:
				curr_window = std::move(res.window);
				curr_window->redraw();
				curr_window->refresh();
				break;
			}
		}
		}
	}
}

int main(int argc, char * argv[])
{
	/* set the locale for curses, must be done before any I/O */
	setlocale(LC_ALL, "");

	/* init libgit */
	git::git_library_lock git_library_lock;

	/* get the repo path, default to current directory */
	const char *start_path = ".";

	/* check if a path is specified on the command line */
	if (argc > 1)
		start_path = argv[1];

	/* search for a git repo */
	std::string repo_path;
	try {
		repo_path = git::repository::discover(start_path);
	} catch (const git::libgit_error &e) {
		if (e.get_err_code() == GIT_ENOTFOUND && e.get_err_klass() == GITERR_REPOSITORY) {
			printf("could not find a git repository from the starting path '%s'\n", start_path);
		} else {
			printf("libgit2 returned an error:\nerr_code = %d\nerr_klass = %d\nerr_msg = %s\n",
				e.get_err_code(), e.get_err_klass(), e.what());
		}
		return -1;
	}

	try {
		/* open the git repo */
		git::repository repo(repo_path.c_str());

		/* load user preferences */
		preferences prefs;

		/* start the main application */
		window_loop(repo, prefs);
	} catch (const cpp_curses::curses_error &e) {
		printf("curses library error: %s\n", e.what());
		return -1;
	} catch (const git::libgit_error &e) {
		printf("libgit2 returned an error:\nerr_code = %d\nerr_klass = %d\nerr_msg = %s\n",
			e.get_err_code(), e.get_err_klass(), e.what());
		return -1;
	} catch (const std::exception &e) {
		printf("stl error: %s\n", e.what());
		return -1;
	} catch (...) {
		printf("unknown error\n");
		return -1;
	}

	return 0;
}
