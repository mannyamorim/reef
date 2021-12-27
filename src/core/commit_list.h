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

/* commit_list.h */
#ifndef COMMIT_LIST_H
#define COMMIT_LIST_H

#include <git2.h>

#include <list>
#include <unordered_set>
#include <vector>

#include "compat/cpp_git.h"
#include "util/preferences.h"

#include "ref_map.h"

/*!
 * \struct commit_graph_info
 * \brief Structure for storing information needed to produce the commit graph
 */
struct commit_graph_info {
	/*! \brief A list showing the ids of commit branches that are considered duplicates */
	std::unordered_set<unsigned int> duplicate_ids;
	/*! \brief A list showing the ids of commit branches that were added from a merge */
	std::vector<unsigned int> new_parent_ids;

	/*! \brief The id of the commit branch returned */
	unsigned int id_of_commit;
	/*! \brief The number of parents of the commit returned */
	unsigned int num_parents;
	/*! \brief The number of duplicate ids */
	unsigned int num_duplicates;
};

/*!
 * \class commit_list
 * \brief Class for loading commits in a temporal topological order
 */
class commit_list {
public:
	/*!
	 * \brief Create a new instance of commit_list
	 * \param refs The ref_map containing all of references in the repo
	 * \param repo The git::repository instance
	 * \param prefs The prefs instance
	 */
	commit_list(const ref_map &refs, const git::repository &repo, const preferences &prefs);

	/*!
	 * \brief Reload the list of references into the commit_list to begin the display process
	 * \param refs The ref_map containing all of references in the repo
	 */
	void initialize(const ref_map &refs);

	/*!
	 * \brief Retrieve the latest commit from the git_commit_list
	 * \param graph The commit_graph_info struct to populate
	 * \return The next commit
	 */
	git::commit get_next_commit(commit_graph_info &graph);

	/*!
	 * \brief Retreive a commit by its id
	 * \param oid The git_oid to search for
	 * \return The commit with the matching id
	 */
	git::commit get_commit_by_id(git_oid *oid);

	/*!
	 * \brief Check if there are any remaining commits to load
	 * \return True if the commit list is empty
	 */
	bool empty();

private:
	/*!
	 * \struct commit_list::graph_node
	 * \brief Private structure for representing nodes in the repo graph
	 *
	 * This struct represents a node in the directed acyclic graph.
	 * It is populated by loading the commits using a depth first search.
	 * Each nodes has pointers to its parents and children.
	 * It also stores the time and depth. The time is the corrected time.
	 * For any node, the corrected time is the minimum of the MAX of the
	 * times of the parents plus one or the stored time of the commit.
	 */
	struct graph_node {
		git::commit commit;
		git_time_t time;
		size_t depth;

		std::vector<graph_node *> parents;
		std::vector<graph_node *> children;

		graph_node(git::commit &&commit, git_time_t time, size_t depth);
		graph_node(const graph_node &) = delete;
		graph_node &operator=(const graph_node &) = delete;
		graph_node(graph_node &&) noexcept;
		graph_node &operator=(graph_node &&) noexcept;
	};

	/*!
	 * \struct commit_list::node
	 * \brief Private structure for representing nodes in the commit list
	 *
	 * Each node points to a corresponding graph node. The id is used to keep
	 * track of branches. A new id is created for each new head or merge.
	 */
	struct node {
		graph_node *graph_node_ptr;
		unsigned int id;

		node(graph_node *graph_node_ptr, unsigned int id);
		node(const node &) = delete;
		node &operator=(const node &) = delete;
		node(node &&other) noexcept;
		node &operator=(node &&) noexcept;

		bool operator==(const git_oid *node_oid) const;
		bool operator <(const node &node) const;
	};

	const git::repository &repo;
	const preferences &prefs;
	unsigned int next_id = 0;
	std::vector<node> clist;
	std::unordered_set<git_oid, git_oid_ref_hash, git_oid_ref_cmp> commits_visited;
	std::unordered_set<git_oid, git_oid_ref_hash, git_oid_ref_cmp> commits_returned;
	std::unordered_map<git_oid, graph_node, git_oid_ref_hash, git_oid_ref_cmp> commits_loaded;
	std::list<graph_node *> bfs_queue;

	/*!
	 * \brief remove_duplicates
	 * \param latest_commit_oid
	 * \param graph The commit_graph_info structure
	 */
	void remove_duplicates(const git_oid *latest_commit_oid, commit_graph_info &graph);

	/*!
	 * \brief Insert the latest nodes parents into the commit list
	 * \param latest_node The latest node
	 * \param graph The commit_graph_info structure
	 */
	void insert_parents(const node &latest_node, commit_graph_info &graph);

	/*!
	 * \brief Prepare the bfs queue using the refs in refs
	 * \param refs The ref_map containing all of the refs
	 */
	void initialize_bfs_queue(const ref_map &refs);

	/*!
	 * \brief Execute the breadth first search up to the requested depth
	 * \param requested_depth The requested depth
	 */
	void bfs(size_t requested_depth);

	/*!
	 * \brief Recalculate the commit times
	 * We calculate the corrected time. For any node, the corrected time
	 * is the minimum of the MAX of the times of the parents plus one or
	 * the stored time of the commit.
	 *
	 * \param node The node to start at
	 * \param parent_time The maximum time of all of the nodes parents
	 */
	void fix_commit_times(graph_node *node, const git_time_t parent_time);
};

#endif /* COMMIT_LIST_H */
