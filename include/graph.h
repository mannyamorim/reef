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

/* graph.h */
#ifndef GRAPH_H
#define GRAPH_H

#define GRAPH_MAX_WIDTH 1024
#define GRAPH_MAX_COLORS 6

#include <curses.h>

#include <vector>

#include "commit_list.h"
#include "reef_string.h"

class graph_list {
public:
	void initialize();
	size_t compute_graph(commit_graph_info &graph, std::vector<char8_t> &buf);

private:
	enum class GRAPH_STATUS : char {
		OLD,
		NEW_HEAD,
		REMOVED,
		COMMIT,
		COMMIT_INITIAL,
		EMPTY,
		MERGE_HEAD,
		REM_MERGE,
		CLPSE_BEG,
		CLPSE_MID,
		CLPSE_END,
	};

	/* structure for storing the graph info */
	struct node {
		unsigned int commit_list_branch_id;
		GRAPH_STATUS status;
		char color;
	};

	unsigned int color_branches[GRAPH_MAX_COLORS] = { 0 };
	std::vector<node> glist;

	unsigned char get_next_color();
	void remove_color(unsigned char color);

	int search_for_commit_index(commit_graph_info &graph);
	size_t mark_graph_duplicates(commit_graph_info &graph);
	void add_parents(size_t list_head_commit, const commit_graph_info &graph);
	void cleanup_empty_graph_right();
	void collapse_graph(std::vector<node>::iterator graph_node, bool node_is_commit);
	void search_for_collapses();
};

#endif /* GRAPH_H */
