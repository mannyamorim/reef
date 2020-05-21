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

#include <cassert>
#include <climits>
#include <vector>
#include <stdexcept>

#include "commit_list.h"
#include "graph.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

constexpr unsigned char G_EMPTY = 0x00;
constexpr unsigned char G_LEFT  = 0x01;
constexpr unsigned char G_RIGHT = 0x02;
constexpr unsigned char G_UPPER = 0x04;
constexpr unsigned char G_LOWER = 0x08;
constexpr unsigned char G_MARK  = 0x10;
constexpr unsigned char G_INITIAL = 0x20;

unsigned char graph_list::get_next_color()
{
	unsigned int min = UINT_MAX;
	unsigned char color = 0;
	for (unsigned char i = 0; i < GRAPH_MAX_COLORS; i++) {
		if (color_branches[i] < min) {
			min = color_branches[i];
			color = i + 1;
		}
	}

	color_branches[color - 1]++;
	return color;
}

void graph_list::remove_color(unsigned char color)
{
	assert(color_branches[color - 1] > 0);
	color_branches[color - 1]--;
}

/* gets the box drawing character made up from combining the flags */
static chtype get_graph_char(unsigned char flags)
{
	switch (flags) {
		case (G_LEFT | G_RIGHT | G_UPPER | G_LOWER):
			return ACS_PLUS;
		case (G_LEFT | G_RIGHT | G_UPPER          ):
			return ACS_BTEE;
		case (G_LEFT | G_RIGHT           | G_LOWER):
			return ACS_TTEE;
		case (G_LEFT | G_RIGHT                    ):
			return ACS_HLINE;

		case (G_LEFT           | G_UPPER | G_LOWER):
			return ACS_RTEE;
		case (G_LEFT           | G_UPPER          ):
			return ACS_LRCORNER;
		case (G_LEFT                     | G_LOWER):
			return ACS_URCORNER;
		case (G_LEFT                              ):
			return -1;

		case (         G_RIGHT | G_UPPER | G_LOWER):
			return ACS_LTEE;
		case (         G_RIGHT | G_UPPER          ):
			return ACS_LLCORNER;
		case (         G_RIGHT           | G_LOWER):
			return ACS_ULCORNER;
		case (         G_RIGHT                    ):
			return -1;

		case (                   G_UPPER | G_LOWER):
			return ACS_VLINE;
		case (                   G_UPPER          ):
			return -1;
		case (                             G_LOWER):
			return -1;

		case (G_EMPTY):
			return static_cast<chtype>(' ');
		case (G_MARK | G_INITIAL):
			return static_cast<chtype>('I');
		case (G_MARK):
			return ACS_BULLET;
		
		default:
			return -1;
	}
}

int graph_list::search_for_commit_index(commit_graph_info &graph)
{
	int i = 0;

	for (auto it = glist.begin(); it != glist.end(); it++) {
		if (it->commit_list_branch_id == graph.id_of_commit) {
			return i;
		} else if (graph.duplicate_ids.count(it->commit_list_branch_id) > 0) {
			/* removed branch before the commit branch */
			graph.duplicate_ids.erase(it->commit_list_branch_id);
			graph.duplicate_ids.insert(graph.id_of_commit);
			it->commit_list_branch_id = graph.id_of_commit;
			return i;
		}

		i++;
	}

	return -1;
}

size_t graph_list::mark_graph_duplicates(commit_graph_info &graph)
{
	size_t list_head_commit = 0;
	bool found_commit = false;
	for (auto it = glist.begin(); it != glist.end(); it++) {
		if (it->status == GRAPH_STATUS::EMPTY)
			continue;

		if (!found_commit && graph.id_of_commit == it->commit_list_branch_id) {
			if (graph.num_parents == 0)
				it->status = GRAPH_STATUS::COMMIT_INITIAL;
			else
				it->status = GRAPH_STATUS::COMMIT;

			list_head_commit = it - glist.begin();
			found_commit = true;
			continue;
		}

		if (graph.duplicate_ids.count(it->commit_list_branch_id) > 0)
			it->status = GRAPH_STATUS::REMOVED;
		else
			it->status = GRAPH_STATUS::OLD;
	}

	assert(found_commit);

	return list_head_commit;
}

void graph_list::add_parents(size_t list_head_commit, const commit_graph_info &graph)
{
	size_t pos = list_head_commit;

	for (unsigned int i = 1; i < graph.num_parents; i++) {
		bool node_added = false;
		pos++;
		while (pos != glist.size()) {
			node &node = glist[pos];
			if (node.status == GRAPH_STATUS::REMOVED) {
				node.commit_list_branch_id = graph.new_parent_ids[i - 1];
				node.status = GRAPH_STATUS::REM_MERGE;
				node_added = true;
				break;
			} else if (node.status == GRAPH_STATUS::EMPTY) {
				node.commit_list_branch_id = graph.new_parent_ids[i - 1];
				node.status = GRAPH_STATUS::MERGE_HEAD;
				node.color = get_next_color();
				node_added = true;
				break;
			}

			pos++;
		}

		if (!node_added) {
			node node;
			node.commit_list_branch_id = graph.new_parent_ids[i - 1];
			node.status = GRAPH_STATUS::MERGE_HEAD;
			node.color = get_next_color();
			glist.push_back(node);
		}
	}
}

void graph_list::cleanup_empty_graph_right()
{
	auto it = glist.end();
	while (it != glist.begin()) {
		it--;

		if (it->status == GRAPH_STATUS::EMPTY) {
			it = glist.erase(it);
			continue;
		} else {
			break;
		}
	}
}

static void draw_merge_connection(unsigned char (&flags_buf)[GRAPH_MAX_WIDTH], unsigned char (&color_buf)[GRAPH_MAX_WIDTH], int index, unsigned char color)
{
	for (; (flags_buf[index] & G_MARK) == 0; index--) {
		assert(index > 0);
		if (flags_buf[index] != (G_UPPER | G_LOWER)) {
			flags_buf[index] |= (G_LEFT | G_RIGHT);
			if (color_buf[index] == 0)
				color_buf[index] = color;
		}
	}
}

void graph_list::collapse_graph(std::vector<node>::iterator graph_node, bool node_is_commit)
{
	/* search for emtpy columns to the right */
	int empty_count = 0;
	bool commit_found_left = false;
	
	{
		auto it = graph_node;
		while (it != glist.begin()) {
			it--;

			if (it->status == GRAPH_STATUS::EMPTY) {
				empty_count++;
			} else if (empty_count == 0) {
				break;
			} else if (it->status == GRAPH_STATUS::COMMIT) {
				commit_found_left = true;
				break;
			}
		}
	}

	if (empty_count == 0)
		return;

	if (commit_found_left || node_is_commit) {
		/* if the commit is found to the left of the potential collapse
		we need to search for any merges or duplicates to the right */
		for (auto it = graph_node; it != glist.end(); it++) {
			if (it->status == GRAPH_STATUS::MERGE_HEAD
					|| it->status == GRAPH_STATUS::REM_MERGE
					|| it->status == GRAPH_STATUS::REMOVED) {
				/* ineligible for collapse */
				return;
			}
		}
	}
	
	{
		/* perform actual collapse */
		graph_node->status = GRAPH_STATUS::CLPSE_BEG;

		auto it = graph_node;
		it--;

		for (int i = 0; i < empty_count - 1; i++) {
			it->status = GRAPH_STATUS::CLPSE_MID;
			it->color = graph_node->color;
			it--;
		}

		it->commit_list_branch_id = graph_node->commit_list_branch_id;
		it->color = graph_node->color;

		if (node_is_commit)
			it->status = GRAPH_STATUS::COMMIT;
		else
			it->status = GRAPH_STATUS::CLPSE_END;
	}
}

void graph_list::search_for_collapses()
{
	for (auto it = glist.begin(); it != glist.end(); it++) {
		if (it->status == GRAPH_STATUS::OLD)
			collapse_graph(it, false);
		else if (it->status == GRAPH_STATUS::COMMIT)
			collapse_graph(it, true);
	}
}

size_t graph_list::compute_graph(commit_graph_info &graph, chtype *buf)
{
	int graph_index = search_for_commit_index(graph);

	if (graph_index == -1) {
		node node;
		node.commit_list_branch_id = graph.id_of_commit;
		node.status = GRAPH_STATUS::NEW_HEAD;
		node.color = get_next_color();
		glist.push_back(node);
	}

	size_t list_head_commit = mark_graph_duplicates(graph);

	add_parents(list_head_commit, graph);

	search_for_collapses();

	unsigned char tmp[GRAPH_MAX_WIDTH];
	unsigned char tmp_clr[GRAPH_MAX_WIDTH];
	size_t i = 0;

	for (auto it = glist.begin(); it != glist.end(); it++) {
		if (i >= sizeof(tmp))
			throw std::out_of_range("max graph width reached (" STR(GRAPH_MAX_WIDTH) ")");

		switch (it->status) {
		case GRAPH_STATUS::OLD:
			tmp_clr[i] = it->color;
			tmp[i++] = G_UPPER | G_LOWER;
			break;
		case GRAPH_STATUS::COMMIT:
			tmp_clr[i] = 0;
			tmp[i++] = G_MARK;
			break;
		case GRAPH_STATUS::COMMIT_INITIAL:
			tmp_clr[i] = 0;
			tmp[i++] = G_MARK | G_INITIAL;
			it->status = GRAPH_STATUS::EMPTY;
			break;
		case GRAPH_STATUS::MERGE_HEAD:
			tmp_clr[i] = it->color;
			tmp[i++] = G_LOWER | G_LEFT;
			it->status = GRAPH_STATUS::OLD;
			draw_merge_connection(tmp, tmp_clr, i - 2, it->color);
			break;
		case GRAPH_STATUS::REM_MERGE:
			tmp_clr[i] = it->color;
			tmp[i++] = G_LOWER | G_LEFT | G_UPPER;
			it->status = GRAPH_STATUS::OLD;
			draw_merge_connection(tmp, tmp_clr, i - 2, it->color);
			break;
		case GRAPH_STATUS::REMOVED:
			tmp_clr[i] = it->color;
			tmp[i++] = G_UPPER | G_LEFT;
			it->status = GRAPH_STATUS::EMPTY;
			draw_merge_connection(tmp, tmp_clr, i - 2, it->color);
			remove_color(it->color);
			break;
		case GRAPH_STATUS::CLPSE_BEG:
			tmp_clr[i-1] = it->color;
			tmp[i-1] = G_LEFT | G_RIGHT;
			tmp_clr[i] = it->color;
			tmp[i++] = G_UPPER | G_LEFT;
			it->status = GRAPH_STATUS::EMPTY;
			break;
		case GRAPH_STATUS::CLPSE_MID:
			tmp_clr[i-1] = it->color;
			tmp[i-1] = G_LEFT | G_RIGHT;
			tmp_clr[i] = it->color;
			tmp[i++] = G_LEFT | G_RIGHT;
			it->status = GRAPH_STATUS::EMPTY;
			break;
		case GRAPH_STATUS::CLPSE_END:
			tmp_clr[i] = it->color;
			tmp[i++] = G_LOWER | G_RIGHT;
			it->status = GRAPH_STATUS::OLD;
			break;
		case GRAPH_STATUS::EMPTY:
			tmp_clr[i] = 0;
			tmp[i++] = G_EMPTY;
			break;
		}

		tmp_clr[i] = 0;
		tmp[i++] = G_EMPTY;
	}

	cleanup_empty_graph_right();

	for (int j = 0; j < i; j++)
		buf[j] = get_graph_char(tmp[j]) | COLOR_PAIR(tmp_clr[j]) | (tmp_clr[j] != 0 ? A_BOLD : 0);

	return i;
}
