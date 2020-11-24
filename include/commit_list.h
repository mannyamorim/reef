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

#include <unordered_set>
#include <vector>

#include "cpp_git.h"
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
};

class commit_list {
public:
	/* constructor */
	commit_list(const ref_map &refs, const git::repository &repo);
	
	/* load a list of references into a git_commit_list to begin the display process */
	void initialize(const ref_map &refs, const git::repository &repo);

	/* retrieve the latest commit from the git_commit_list */
	git::commit get_next_commit(commit_graph_info &graph);

	bool empty();

private:
	struct node {
		git::commit commit;
		unsigned int id;

		node(git::commit &&commit, unsigned int id);
		node(const node &) = delete;
		node &operator=(const node &) = delete;
		node(node &&other) noexcept;
		node &operator=(node &&) noexcept;

		bool operator==(const git_oid *node_oid) const;
		bool operator <(const node &node) const;
	};

	unsigned int next_id = 0;
	std::vector<node> clist;

	void remove_duplicates(const git_oid *latest_commit_oid, commit_graph_info &graph);
	void insert_parents(const node &latest_node, commit_graph_info &graph);
};

#endif /* COMMIT_LIST_H */
