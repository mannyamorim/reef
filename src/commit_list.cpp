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

commit_list::node::node(git::commit &&commit, unsigned int id, git_time_t time) :
	commit(std::move(commit)),
	id(id),
	time(time)
{}

commit_list::node::node(node &&other) noexcept :
	commit(std::move(other.commit)),
	id(other.id),
	time(other.time)
{}

commit_list::node &commit_list::node::node::operator=(node &&other) noexcept
{
	commit = std::move(other.commit);
	id = other.id;
	time = other.time;
	return *this;
}

bool commit_list::node::operator==(const git_oid *node_oid) const
{
	const git_oid *this_oid = commit.id();
	return git_oid_equal(this_oid, node_oid);
}

bool commit_list::node::operator <(const node &node) const
{
	if (time < node.time)
		return true;

	if (time == node.time) {
		const git_oid *this_oid = commit.id();
		const git_oid *node_oid = node.commit.id();
		if (git_oid_cmp(this_oid, node_oid) > 0)
			return true;
	}

	return false;
}

commit_list::graph_node::graph_node(git::commit && commit, git_time_t time, size_t depth) :
	commit(std::move(commit)),
	time(time),
	depth(depth)
{}

commit_list::graph_node::graph_node(graph_node &&other) noexcept :
	commit(std::move(other.commit)),
	time(other.time),
	depth(other.depth),
	parents(std::move(other.parents)),
	children(std::move(other.children))
{}

commit_list::graph_node &commit_list::graph_node::operator=(graph_node &&other) noexcept
{
	commit = std::move(other.commit);
	time = other.time;
	depth = other.depth;
	parents = std::move(other.parents);
	children = std::move(other.children);
	return *this;
}

commit_list::commit_list(const ref_map &refs, const git::repository &repo) :
	repo(repo)
{
	initialize(refs);
	bfs();
}

void commit_list::bfs()
{
	while (!bfs_queue.empty()) {
		graph_node *node = bfs_queue.front();
		bfs_queue.pop_front();

		git_time_t max_parent_time = 0;

		for (unsigned int i = 0; i < node->commit.parentcount(); i++) {
			const git_oid *parent_id = node->commit.parent_id(i);
			git_time_t parent_time;
			if (commits_visited.count(*parent_id) == 0) {
				/* load parent */
				git::commit parent = node->commit.parent(i);
				parent_time = parent.time();

				/* mark parent as visited */
				commits_visited.insert(*parent_id);

				/* add parent to the queue */
				graph_node new_graph_node(std::move(parent), parent_time, node->depth + 1);
				auto ret = commits_loaded.insert(std::make_pair(*parent_id, std::move(new_graph_node)));
				bfs_queue.push_back(&ret.first->second);

				/* add pointer from child to parent */
				node->parents.push_back(&ret.first->second);

				/* add pointer from parent to child */
				ret.first->second.children.push_back(node);
			} else {
				/* find parent */
				graph_node *parent = &commits_loaded.find(*parent_id)->second;
				parent_time = parent->time;

				/* add pointer from child to parent */
				node->parents.push_back(parent);

				/* add pointer from parent to child */
				parent->children.push_back(node);
			}

			if (parent_time > max_parent_time)
				max_parent_time = parent_time;
		}

		if (max_parent_time >= node->time)
			fix_commit_times(node, max_parent_time);
	}
}

void commit_list::fix_commit_times(graph_node *node, const git_time_t parent_time)
{
	node->time = parent_time + 1;
	for (graph_node *child : node->children)
		if (child->time <= parent_time + 1)
			fix_commit_times(child, parent_time + 1);
}

void commit_list::initialize(const ref_map &refs)
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

	/* load all of the unique refs into clist and bfs_queue */
	for (auto &it : refs_active) {
		git::commit commit = repo.commit_lookup(&it);
		git::commit commit_copy = commit;
		git_time_t time = commit.time();

		node new_node(std::move(commit), next_id++, time);
		clist.push_back(std::move(new_node));

		graph_node new_graph_node(std::move(commit_copy), time, 0);
		auto ret = commits_loaded.insert(std::make_pair(*&it, std::move(new_graph_node)));
		bfs_queue.push_back(&ret.first->second);
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
		git::commit parent = latest_node.commit.parent(0);
		git_time_t parent_time = parent.time();
		if (parent_time >= latest_node.time)
			parent_time = latest_node.time - 1;

		/* set the first parent to have the same id as the child */
		node new_node(std::move(parent), latest_node.id, parent_time);
		clist.push_back(std::move(new_node));
		std::push_heap(clist.begin(), clist.end());
	}

	for (int i = 1; i < graph.num_parents; i++) {
		git::commit parent = latest_node.commit.parent(i);
		git_time_t parent_time = parent.time();
		if (parent_time >= latest_node.time)
			parent_time = latest_node.time - 1;

		/* give every additional parent a newly generated id */
		node new_node(std::move(parent), next_id++, parent_time);
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
