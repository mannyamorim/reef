/*
 * Reef - TUI Client for Git
 * Copyright (C) 2020-2021 Emmanuel Mathi-Amorim
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

#include "reef_string.h"
#include "repository_controller.h"

repository_controller::repository_controller(std::string &dir) :
	repo(dir.c_str()),
	refs(repo),
	prefs(),
	clist(refs, repo, prefs),
	glist()
{}

void repository_controller::display_commits(std::function<void(const QChar *, size_t)> display_line)
{
	while (!clist.empty()) {
		struct commit_graph_info graph;
		git::commit commit = clist.get_next_commit(graph);
		const char *summary = commit.summary();

		QChar buf[preferences::max_line_length];

		size_t i = glist.compute_graph(graph, buf);
		add_utf8_str_to_buf(buf, summary, i);

		display_line(buf, i);
	}
}
