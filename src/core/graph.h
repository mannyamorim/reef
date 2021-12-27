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

/*!
 * \struct graph_char
 * \brief Structure to hold a finished graph character along with its color
 */
struct graph_char {
	/*! \brief The character defined using the flags */
	unsigned char flags;
	/*! \brief The character's color */
	unsigned char color;
};

/*!
 * \class graph_list
 * \brief Class to manage the graph of the commits
 */
class graph_list {
public:
	/*!
	 * \brief Reset the state of the graph
	 */
	void initialize();

	/*!
	 * \brief Compute the next step in the graph
	 * \param graph The commit_graph_info structure describing the changes to the graph
	 * \param buf The buffer to write the graph_chars to
	 * \return The number of characters writtn to the buffer
	 */
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

	/*!
	 * \struct graph_list::node
	 * \brief Structure for storing the graph info
	 */
	struct node {
		/*! \brief The branch id of the node */
		unsigned int commit_list_branch_id;
		/*! \brief The status of the node */
		GRAPH_STATUS status;
		/*! \brief The color of the node */
		char color;
	};

	unsigned int color_branches[GRAPH_MAX_COLORS] = { 0 };
	std::vector<node> glist;

	/*!
	 * \brief Gets the next color to use for a new branch
	 * \return The next color to use
	 */
	unsigned char get_next_color();

	/*!
	 * \brief Decrement the number of branches using a color, used when branches end
	 * \param color The color to remove
	 */
	void remove_color(unsigned char color);

	/*!
	 * \brief Search for the selected commit in the graph list
	 * \param graph The commit_graph_info structure
	 * \return The index of the selected commit
	 */
	int search_for_commit_index(commit_graph_info &graph);

	/*!
	 * \brief Update the status for all of the nodes in the graph list
	 * \param graph The commit_graph_info structure
	 * \return The index of the selected node
	 */
	size_t mark_graph_duplicates(commit_graph_info &graph);

	/*!
	 * \brief Add the parents for the current node
	 * \param list_head_commit The index of the node to add the parents
	 * \param graph The commit_graph_info structure
	 */
	void add_parents(size_t list_head_commit, const commit_graph_info &graph);

	/*!
	 * \brief Erase any empty graph nodes on the right
	 */
	void cleanup_empty_graph_right();

	/*!
	 * \brief Collapse the graph at node_index empty_count spaces to the left
	 * \param node_index The index in the graph list of the node to start the collapse
	 * \param node_is_commit Whether or not the node to collapse represents a commit
	 * \param empty_count The number of empty spaces to the left of the node_index
	 */
	void collapse_graph(int node_index, bool node_is_commit, int empty_count);

	/*!
	 * \brief Search for possible points to collapse the graph
	 * \param index_of_commit The index in the graph list of the node representing the current commit
	 */
	void search_for_collapses(int index_of_commit);
};

#endif /* GRAPH_H */
