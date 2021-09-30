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

#include <git2.h>

#include <algorithm>
#include <cassert>
#include <vector>

#include "commit_list.h"
#include "error.h"
#include "preferences.h"
#include "ref_map.h"
#include "cpp_git.h"

commit_list::node::node(graph_node *graph_node_ptr, unsigned int id) :
	graph_node_ptr(graph_node_ptr),
	id(id)
{}

commit_list::node::node(node &&other) noexcept :
	graph_node_ptr(other.graph_node_ptr),
	id(other.id)
{}

commit_list::node &commit_list::node::node::operator=(node &&other) noexcept
{
	graph_node_ptr = other.graph_node_ptr;
	id = other.id;
	return *this;
}

bool commit_list::node::operator==(const git_oid *node_oid) const
{
	const git_oid *this_oid = graph_node_ptr->commit.id();
	return git_oid_equal(this_oid, node_oid);
}

bool commit_list::node::operator <(const node &node) const
{
	if (graph_node_ptr->time < node.graph_node_ptr->time)
		return true;

	if (graph_node_ptr->time == node.graph_node_ptr->time) {
		const git_oid *this_oid = graph_node_ptr->commit.id();
		const git_oid *node_oid = node.graph_node_ptr->commit.id();
		if (git_oid_cmp(this_oid, node_oid) > 0)
			return true;
	}

	return false;
}

commit_list::graph_node::graph_node(git::commit &&commit, git_time_t time, size_t depth) :
	commit(std::move(commit)),
	time(time),
	depth(depth),
	parents(),
	children()
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

commit_list::commit_list(const ref_map &refs, const git::repository &repo, const preferences &prefs) :
	repo(repo),
	prefs(prefs)
{
	initialize_bfs_queue(refs);
	bfs(prefs.graph_approximation_factor);
	initialize(refs);
}

void commit_list::bfs(size_t requested_depth)
{
	while (!bfs_queue.empty() && bfs_queue.front()->depth <= requested_depth) {
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
				auto ret = commits_loaded.emplace(*parent_id, std::move(new_graph_node));
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
		if (parent_time + 1 >= child->time)
			fix_commit_times(child, parent_time + 1);
}

void commit_list::initialize_bfs_queue(const ref_map &refs)
{
	/* copy all of the refs into a set */
	std::unordered_set<git_oid, git_oid_ref_hash, git_oid_ref_cmp> refs_unique;
	for (auto &it : refs.refs)
		refs_unique.insert(it.first);

	/* load all of the unique refs into bfs_queue */
	for (auto &it : refs_unique) {
		git::commit commit = repo.commit_lookup(&it);
		git_time_t time = commit.time();

		commits_visited.insert(*commit.id());

		graph_node new_graph_node(std::move(commit), time, 0);
		auto ret = commits_loaded.emplace(*&it, std::move(new_graph_node));
		bfs_queue.push_back(&ret.first->second);
	}
}

void commit_list::initialize(const ref_map &refs)
{
	if (refs.refs.empty())
		return;

	/* reset state */
	next_id = 0;
	clist.clear();
	commits_returned.clear();

	/* copy all of the refs that are active into a set */
	std::unordered_set<git_oid, git_oid_ref_hash, git_oid_ref_cmp> refs_active;
	for (auto &it : refs.refs)
		if (it.second.second)
			refs_active.insert(it.first);

	/* load all of the unique refs into clist */
	for (auto &it : refs_active) {
		auto loaded_commit = commits_loaded.find(*&it);
		graph_node *node = &loaded_commit->second;
		clist.emplace_back(node, next_id++);
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
		/* load_parent */
		graph_node *node = latest_node.graph_node_ptr->parents[0];

		/* set the first parent to have the same id as the child */
		clist.emplace_back(node, latest_node.id);
		std::push_heap(clist.begin(), clist.end());
	}

	for (size_t i = 1; i < graph.num_parents; i++) {
		/* load_parent */
		graph_node *node = latest_node.graph_node_ptr->parents[i];

		/* give every additional parent a newly generated id */
		unsigned int node_id = next_id++;
		clist.emplace_back(node, node_id);
		std::push_heap(clist.begin(), clist.end());

		/* add additional parents to the the new_parent_ids list */
		graph.new_parent_ids.push_back(node_id);
	}
}

git::commit commit_list::get_next_commit(commit_graph_info &graph)
{
	/* get the latest commit from the heap */
	std::pop_heap(clist.begin(), clist.end());
	node latest_node = std::move(clist.back());
	clist.pop_back();

	if (commits_returned.count(*latest_node.graph_node_ptr->commit.id()) == 0)
		commits_returned.insert(*latest_node.graph_node_ptr->commit.id());
	else
		throw reef_error("commit returned twice");

	graph.id_of_commit = latest_node.id;

	remove_duplicates(latest_node.graph_node_ptr->commit.id(), graph);

	graph.num_parents = latest_node.graph_node_ptr->commit.parentcount();
	insert_parents(latest_node, graph);

	bfs(latest_node.graph_node_ptr->depth + prefs.graph_approximation_factor);

	return std::move(latest_node.graph_node_ptr->commit);
}

bool commit_list::empty()
{
	return clist.empty();
}
