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

/* window_commit.h */
#ifndef WINDOW_COMMIT_H
#define WINDOW_COMMIT_H

#include <git2.h>

#include "cpp_git.h"
#include "preferences.h"
#include "scroll_window.h"
#include "window_base.h"

class window_commit : public window_base
{
public:
	window_commit(const git::repository &repo, const preferences &prefs, const git::commit &commit);

	void refresh() override;
	void resize() override;
	void redraw() override;

	input_response process_key_input(int key) override;
	int _getch() override;

	static constexpr size_t MAIN_LINE_LENGTH = 256;

private:
	const git::repository &repo;
	const preferences &prefs;
	const git::commit &commit;
	git::tree tree_a, tree_b;
	git::diff diff;

	scroll_window<std::pair<const git_diff_delta *, int>> file_window;
	scroll_window<const git_diff_line *> line_window;

	int process_line_callback(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line);
};

#endif /* WINDOW_COMMIT_H */
