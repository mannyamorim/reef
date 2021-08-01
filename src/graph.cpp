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

#include <cassert>
#include <climits>
#include <vector>
#include <stdexcept>

#include "commit_list.h"
#include "graph.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

constexpr size_t num_line_drawing_chars = 18;

const QChar line_drawing_chars[] = {
	u' ', /* 00 = G_EMPTY                              */
	u' ', /* 01 = G_LEFT                               */
	u' ', /* 02 =          G_RIGHT                     */
	u'─', /* 03 = G_LEFT | G_RIGHT                     */

	u' ', /* 04 =                    G_UPPER           */
	u'┘', /* 05 = G_LEFT           | G_UPPER           */
	u'└', /* 06 =          G_RIGHT | G_UPPER           */
	u'┴', /* 07 = G_LEFT | G_RIGHT | G_UPPER           */

	u' ', /* 08 =                              G_LOWER */
	u'┐', /* 09 = G_LEFT                     | G_LOWER */
	u'┌', /* 10 =          G_RIGHT           | G_LOWER */
	u'┬', /* 11 = G_LEFT | G_RIGHT           | G_LOWER */

	u'│', /* 12 =                    G_UPPER | G_LOWER */
	u'┤', /* 13 = G_LEFT           | G_UPPER | G_LOWER */
	u'├', /* 14 =          G_RIGHT | G_UPPER | G_LOWER */
	u'┼', /* 15 = G_LEFT | G_RIGHT | G_UPPER | G_LOWER */

	u'•', /* 16 = G_MARK                               */
	u'I', /* 17 = G_INITIAL                            */
};

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

int graph_list::search_for_commit_index(commit_graph_info &graph)
{
	int i = 0;

	for (auto it = glist.begin(); it != glist.end(); it++) {
		if (it->status == GRAPH_STATUS::EMPTY)
			continue;

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

		if (graph.num_duplicates > 0 && graph.duplicate_ids.count(it->commit_list_branch_id) > 0) {
			it->status = GRAPH_STATUS::REMOVED;
			graph.num_duplicates--;
			continue;
		}

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

static void draw_merge_connection(
		graph_char (&buf)[preferences::max_line_length],
		int index, unsigned char color)
{
	for (; (buf[index].flags & G_MARK) == 0; index--) {
		assert(index > 0);
		if (buf[index].flags != (G_UPPER | G_LOWER)) {
			buf[index].flags |= (G_LEFT | G_RIGHT);
			if (buf[index].color == 0)
				buf[index].color = color;
		}
	}
}

void graph_list::collapse_graph(int node_index, bool node_is_commit, int empty_count)
{
	if (empty_count == 0)
		return;

	/* perform actual collapse */
	glist[node_index].status = GRAPH_STATUS::CLPSE_BEG;

	int i;
	for (i = node_index - 1; i > node_index - empty_count; i--) {
		glist[i].status = GRAPH_STATUS::CLPSE_MID;
		glist[i].color = glist[node_index].color;
	}

	glist[i].commit_list_branch_id = glist[node_index].commit_list_branch_id;
	glist[i].color = glist[node_index].color;

	if (node_is_commit)
		glist[i].status = GRAPH_STATUS::COMMIT;
	else
		glist[i].status = GRAPH_STATUS::CLPSE_END;
}

void graph_list::search_for_collapses(int index_of_commit)
{
	/* the region from index_of_commit to region_end is a line of merges where we cannot draw collapses */
	int region_end = -1;

	for (int i = 0; i < glist.size(); i++)
		if (glist[i].status == GRAPH_STATUS::MERGE_HEAD
				|| glist[i].status == GRAPH_STATUS::REM_MERGE
				|| glist[i].status == GRAPH_STATUS::REMOVED)
			/* we found another merge */
			region_end = i;

	int collapse_length = 0;
	for (int i = 0; i < glist.size(); i++) {
		if (i >= index_of_commit && i <= region_end) {
			collapse_length = 0;
			continue;
		}

		if (glist[i].status == GRAPH_STATUS::EMPTY) {
			collapse_length++;
		} else if (glist[i].status == GRAPH_STATUS::OLD) {
			collapse_graph(i, false, collapse_length);
			collapse_length = 0;
		} else if (glist[i].status == GRAPH_STATUS::COMMIT) {
			collapse_graph(i, true, collapse_length);
			collapse_length = 0;
		} else {
			collapse_length = 0;
		}
	}
}

void graph_list::initialize()
{
	for (unsigned char i = 0; i < GRAPH_MAX_COLORS; i++)
		color_branches[i] = 0;
	glist.clear();
}

size_t graph_list::compute_graph(commit_graph_info &graph, graph_char (&buf)[preferences::max_line_length])
{
	int graph_index = search_for_commit_index(graph);

	if (graph_index == -1) {
		node node;
		node.commit_list_branch_id = graph.id_of_commit;
		node.status = GRAPH_STATUS::NEW_HEAD;
		node.color = get_next_color();
		glist.push_back(node);
		graph_index = glist.size();
	}

	size_t list_head_commit = mark_graph_duplicates(graph);

	add_parents(list_head_commit, graph);

	search_for_collapses(graph_index);

	size_t i = 0;
	for (auto it = glist.begin(); it != glist.end(); it++) {
		if (i < preferences::max_line_length) {
			switch (it->status) {
			case GRAPH_STATUS::OLD:
				buf[i].color = it->color;
				buf[i++].flags = G_UPPER | G_LOWER;
				break;
			case GRAPH_STATUS::COMMIT:
				buf[i].color = 0;
				buf[i++].flags = G_MARK;
				break;
			case GRAPH_STATUS::COMMIT_INITIAL:
				buf[i].color = 0;
				buf[i++].flags = G_MARK | G_INITIAL;
				it->status = GRAPH_STATUS::EMPTY;
				break;
			case GRAPH_STATUS::MERGE_HEAD:
				buf[i].color = it->color;
				buf[i++].flags = G_LOWER | G_LEFT;
				it->status = GRAPH_STATUS::OLD;
				draw_merge_connection(buf, i - 2, it->color);
				break;
			case GRAPH_STATUS::REM_MERGE:
				buf[i].color = it->color;
				buf[i++].flags = G_LOWER | G_LEFT | G_UPPER;
				it->status = GRAPH_STATUS::OLD;
				draw_merge_connection(buf, i - 2, it->color);
				break;
			case GRAPH_STATUS::REMOVED:
				buf[i].color = it->color;
				buf[i++].flags = G_UPPER | G_LEFT;
				it->status = GRAPH_STATUS::EMPTY;
				draw_merge_connection(buf, i - 2, it->color);
				remove_color(it->color);
				break;
			case GRAPH_STATUS::CLPSE_BEG:
				buf[i-1].color = it->color;
				buf[i-1].flags = G_LEFT | G_RIGHT;
				buf[i].color = it->color;
				buf[i++].flags = G_UPPER | G_LEFT;
				it->status = GRAPH_STATUS::EMPTY;
				break;
			case GRAPH_STATUS::CLPSE_MID:
				buf[i-1].color = it->color;
				buf[i-1].flags = G_LEFT | G_RIGHT;
				buf[i].color = it->color;
				buf[i++].flags = G_LEFT | G_RIGHT;
				it->status = GRAPH_STATUS::EMPTY;
				break;
			case GRAPH_STATUS::CLPSE_END:
				buf[i].color = it->color;
				buf[i++].flags = G_LOWER | G_RIGHT;
				it->status = GRAPH_STATUS::OLD;
				break;
			case GRAPH_STATUS::EMPTY:
				buf[i].color = 0;
				buf[i++].flags = G_EMPTY;
				break;
			}

			buf[i].color = 0;
			buf[i++].flags = G_EMPTY;
		} else {
			switch (it->status) {
			case GRAPH_STATUS::MERGE_HEAD:
			case GRAPH_STATUS::REM_MERGE:
			case GRAPH_STATUS::CLPSE_END:
				it->status = GRAPH_STATUS::OLD;
				break;
			case GRAPH_STATUS::REMOVED:
				remove_color(it->color);
			case GRAPH_STATUS::COMMIT_INITIAL:
			case GRAPH_STATUS::CLPSE_BEG:
			case GRAPH_STATUS::CLPSE_MID:
				it->status = GRAPH_STATUS::EMPTY;
				break;
			case GRAPH_STATUS::OLD:
			case GRAPH_STATUS::COMMIT:
			case GRAPH_STATUS::EMPTY:
				break;
			}
		}
	}

	cleanup_empty_graph_right();

	return i;
}
