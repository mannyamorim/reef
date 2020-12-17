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

#include <git2.h>

#include <algorithm>
#include <cassert>
#include <vector>

#include "commit_list.h"
#include "ref_map.h"
#include "cpp_git.h"

commit_list::node::node(git::commit &&commit, unsigned int id) :
	commit(std::move(commit)),
	id(id)
{}

commit_list::node::node(node &&other) noexcept :
	commit(std::move(other.commit)),
	id(other.id)
{}

commit_list::node &commit_list::node::node::operator=(node &&other) noexcept
{
	commit = std::move(other.commit);
	id = other.id;
	return *this;
}

bool commit_list::node::operator==(const git_oid *node_oid) const
{
	const git_oid *this_oid = commit.id();
	return git_oid_equal(this_oid, node_oid);
}

bool commit_list::node::operator <(const node &node) const
{
	const git_time_t this_time = commit.time();
	const git_time_t node_time = node.commit.time();

	if (this_time < node_time)
		return true;

	if (this_time == node_time) {
		const git_oid *this_oid = commit.id();
		const git_oid *node_oid = node.commit.id();
		if (git_oid_cmp(this_oid, node_oid) > 0)
			return true;
	}

	return false;
}

commit_list::commit_list(const ref_map &refs, const git::repository &repo)
{
	initialize(refs, repo);
}


void commit_list::initialize(const ref_map &refs, const git::repository &repo)
{
	if (refs.refs.empty())
		return;

	/* reset state */
	next_id = 0;
	clist.clear();

	/* copy all of the refs that are active into a set */
	std::unordered_set<git_oid, git_oid_ref_hash, git_oid_ref_cmp> refs_active;
	for (auto &it : refs.refs)
		if (it.second.second)
			refs_active.insert(it.first);

	/* load all of the unique refs into clist */
	for (auto &it : refs_active) {
		node new_node(repo.commit_lookup(&it), next_id++);
		clist.push_back(std::move(new_node));
	}

	std::make_heap(clist.begin(), clist.end());
}

/* finds and removes the duplicate commits in the linked list and sets up the
 * duplicates array in the commit_graph_info structure */
void commit_list::remove_duplicates(const git_oid *latest_commit_oid, commit_graph_info &graph)
{
	graph.num_duplicates = 0;

	/* iterate and look for duplicates and remove them */
	while (!clist.empty()) {
		node &node = clist.front();
		if (node == latest_commit_oid) {
			/* mark the index of the duplicate in the graph list */
			graph.duplicate_ids.insert(node.id);
			graph.num_duplicates++;

			/* we found a duplicate so delete the commit */
			std::pop_heap(clist.begin(), clist.end());
			clist.pop_back();
		} else {
			break;
		}
	}
}

void commit_list::insert_parents(const node &latest_node, commit_graph_info &graph)
{
	if (graph.num_parents > 0) {
		/* set the first parent to have the same id as the child */
		node new_node(latest_node.commit.parent(0), latest_node.id);
		clist.push_back(std::move(new_node));
		std::push_heap(clist.begin(), clist.end());
	}

	for (int i = 1; i < graph.num_parents; i++) {
		/* give every additional parent a newly generated id */
		node new_node(latest_node.commit.parent(i), next_id++);
		clist.push_back(std::move(new_node));
		std::push_heap(clist.begin(), clist.end());

		/* add additional parents to the the new_parent_ids list*/
		graph.new_parent_ids.push_back(new_node.id);
	}
}

git::commit commit_list::get_next_commit(commit_graph_info &graph)
{
	/* get the latest commit from the heap */
	std::pop_heap(clist.begin(), clist.end());
	node latest_node = std::move(clist.back());
	clist.pop_back();

	graph.id_of_commit = latest_node.id;

	remove_duplicates(latest_node.commit.id(), graph);

	graph.num_parents = latest_node.commit.parentcount();
	insert_parents(latest_node, graph);

	return std::move(latest_node.commit);
}

bool commit_list::empty()
{
	return clist.empty();
}
