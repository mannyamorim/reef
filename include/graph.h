/*
 * Reef - Cross Platform Git Client
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

/* graph.h */
#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

#include "commit_list.h"

constexpr unsigned char GRAPH_MAX_COLORS = 6;

/* flags for graph drawing characters */
constexpr unsigned char G_EMPTY = 0x00;
constexpr unsigned char G_LEFT  = 0x01;
constexpr unsigned char G_RIGHT = 0x02;
constexpr unsigned char G_UPPER = 0x04;
constexpr unsigned char G_LOWER = 0x08;
constexpr unsigned char G_MARK  = 0x10;
constexpr unsigned char G_INITIAL = 0x11;

/* structure to hold a finished graph character along with its color */
struct graph_char {
	/* the character defined using the flags */
	unsigned char flags;
	/* the character's color */
	unsigned char color;
};

class graph_list {
public:
	void initialize();
	size_t compute_graph(commit_graph_info &graph, graph_char (&buf)[preferences::max_line_length]);

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
	void collapse_graph(int node_index, bool node_is_commit, int empty_count);
	void search_for_collapses(int index_of_commit);
};

#endif /* GRAPH_H */
