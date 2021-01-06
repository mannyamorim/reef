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

/* window_main.h */
#ifndef WINDOW_MAIN_H
#define WINDOW_MAIN_H

#include <git2.h>

#include "cpp_git.h"
#include "ref_map.h"
#include "preferences.h"
#include "scroll_window.h"
#include "window_base.h"

class window_main : public window_base
{
public:
	window_main(const git::repository &repo, const preferences &prefs);
	~window_main();

	void refresh() override;
	void resize() override;
	void redraw() override;

	input_response process_key_input(int key) override;
	bool has_background_work() override;
	bool do_background_work() override;
	int _getch(bool block) override;

private:
	const git::repository &repo;
	const preferences &prefs;
	ref_map refs;
	scroll_window<git::commit> primary_window;
	scroll_window<ref_map::refs_ordered_map::iterator> refs_window;
	std::unordered_map<git_oid, int, git_oid_ref_hash, git_oid_ref_cmp> commit_id_line_map;

	commit_list clist;
	graph_list glist;
	int num_of_commits_loaded = 0;

	void display_refs();
	void display_commit();
	void display_commits(int max);
};

#endif /* WINDOW_MAIN_H */
