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

void repository_controller::display_refs(std::function<void (const char *)> display_ref)
{
	for (ref_map::refs_ordered_map::iterator it = refs.refs_ordered.begin(); it != refs.refs_ordered.end(); it++)
		display_ref(it->first);
}

void repository_controller::display_commits()
{
	while (!clist.empty()) {
		struct commit_graph_info graph;
		git::commit commit = clist.get_next_commit(graph);

		QChar buf[preferences::max_line_length];

		size_t i = glist.compute_graph(graph, buf);

		if (refs.refs.count(*commit.id()) > 0) {
			auto ref_range = refs.refs.equal_range(*commit.id());
			for (auto &it = ref_range.first; it != ref_range.second; it++) {
				if (!it->second.second)
					/* ref is not active, don't show it */
					continue;

				add_utf8_str_to_buf(buf, "[", i);
				add_utf8_str_to_buf(buf, it->second.first.shorthand(), i);
				add_utf8_str_to_buf(buf, "] ", i);
			}
		}

		const git_signature *author = commit.author();
		struct tm *author_time = localtime((time_t *)&(author->when.time));
		char time_buf[21];
		strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S ", author_time);
		add_utf8_str_to_buf(buf, time_buf, i);

		const char *summary = commit.summary();
		add_utf8_str_to_buf(buf, summary, i);

		clist_model.add_line(buf, i);
	}
}
