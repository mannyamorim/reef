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

/* commit_list.h */
#ifndef COMMIT_LIST_H
#define COMMIT_LIST_H

#include <git2.h>

#include <list>
#include <unordered_set>
#include <vector>

#include "cpp_git.h"
#include "preferences.h"
#include "ref_map.h"

/* structure for storing information needed to produce the commit graph */
struct commit_graph_info {
	/* a list showing the ids of commit branches that are considered duplicates */
	std::unordered_set<unsigned int> duplicate_ids;
	/* a list showing the ids of commit branches that were added from a merge */
	std::vector<unsigned int> new_parent_ids;

	/* the id of the commit branch returned */
	unsigned int id_of_commit;
	/* the number of parents of the commit returned */
	unsigned int num_parents;
	/* the number of duplicate ids */
	unsigned int num_duplicates;
};

class commit_list {
public:
	/* constructor */
	commit_list(const ref_map &refs, const git::repository &repo, const preferences &prefs);

	/* load a list of references into a git_commit_list to begin the display process */
	void initialize(const ref_map &refs);

	/* retrieve the latest commit from the git_commit_list */
	git::commit get_next_commit(commit_graph_info &graph);

	bool empty();

private:
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

	void remove_duplicates(const git_oid *latest_commit_oid, commit_graph_info &graph);
	void insert_parents(const node &latest_node, commit_graph_info &graph);

	void initialize_bfs_queue(const ref_map &refs);
	void bfs(int requested_depth);
	void fix_commit_times(graph_node *node, const git_time_t parent_time);
};

#endif /* COMMIT_LIST_H */
