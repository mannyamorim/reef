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

#include <cstring>

#include "gtest/gtest.h"

#include "graph.h"

/* returns the number of elements in the array */
template<typename T, size_t size>
constexpr size_t count_of(T(&)[size])
{
	return size;
}

/* fixture for testing methods of the graph class */
class graph_test_fxt : public testing::Test
{
protected:
	graph_list glist;
	chtype buf[GRAPH_MAX_WIDTH];

	void run_graph_test(
		std::unordered_set<unsigned int> &&duplicate_ids,
		std::vector<unsigned int> &&new_parent_ids,
		const unsigned int id_of_commit,
		const unsigned int num_parents,
		const size_t expected_size,
		const chtype *expected)
	{
		commit_graph_info graph_info;
		graph_info.duplicate_ids = std::move(duplicate_ids);
		graph_info.new_parent_ids = std::move(new_parent_ids);
		graph_info.id_of_commit = id_of_commit;
		graph_info.num_parents = num_parents;

		const size_t res = glist.compute_graph(graph_info, buf);

		EXPECT_EQ(res, expected_size);
		EXPECT_EQ(memcmp(buf, expected, expected_size * sizeof(chtype)), 0);
	}
};

/* basic test of the compute_graph method */
TEST_F(graph_test_fxt, test_single_commit)
{
	const chtype expected[] = { ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected), expected
	);
}

/* test new branch
 * +
 * | +
 * + |
 */
TEST_F(graph_test_fxt, test_new_branch)
{
	const chtype expected1[] = { ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected1), expected1
	);

	const chtype expected2[] = { ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected2), expected2
	);

	const chtype expected3[] = { ACS_BULLET, ' ', ACS_VLINE, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected3), expected3
	);
}

/* test basic merge
 * +
 * +--
 * | +
 */
TEST_F(graph_test_fxt, test_basic_merge)
{
	const chtype expected1[] = { ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected1), expected1
	);

	const chtype expected2[] = { ACS_BULLET, ACS_HLINE, ACS_URCORNER, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{ 2 },	/* new_parent_ids */
		1,	/* id_of_commit */
		2,	/* num_parents */
		count_of(expected2), expected2
	);

	const chtype expected3[] = { ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected3), expected3
	);
}

/* test basic duplicate
 * +
 * | +
 * +--
 */
TEST_F(graph_test_fxt, test_basic_duplicate)
{
	const chtype expected1[] = { ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected1), expected1
	);

	const chtype expected2[] = { ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected2), expected2
	);

	const chtype expected3[] = { ACS_BULLET, ACS_HLINE, ACS_LRCORNER, ' ' };
	run_graph_test(
		{ 2 },	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected3), expected3
	);
}

/* test 3 head merge
 * +----
 * | + |
 * | | +
 */
TEST_F(graph_test_fxt, test_3_head_merge)
{
	const chtype expected1[] = { ACS_BULLET, ACS_HLINE, ACS_TTEE, ACS_HLINE, ACS_URCORNER, ' ' };
	run_graph_test(
		{},		/* duplicate_ids */
		{ 2, 3 },	/* new_parent_ids */
		1,		/* id_of_commit */
		3,		/* num_parents */
		count_of(expected1), expected1
	);

	const chtype expected2[] = { ACS_VLINE, ' ', ACS_BULLET, ' ', ACS_VLINE, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected2), expected2
	);

	const chtype expected3[] = { ACS_VLINE, ' ', ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected3), expected3
	);
}

/* test 3 duplicate
 * +
 * | + 
 * | | +
 * +----
 */
TEST_F(graph_test_fxt, test_3_duplicate)
{
	const chtype expected1[] = { ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected1), expected1
	);

	const chtype expected2[] = { ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected2), expected2
	);

	const chtype expected3[] = { ACS_VLINE, ' ', ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected3), expected3
	);

	const chtype expected4[] = { ACS_BULLET, ACS_HLINE, ACS_BTEE, ACS_HLINE, ACS_LRCORNER, ' ' };
	run_graph_test(
		{ 1, 3 },	/* duplicate_ids */
		{},		/* new_parent_ids */
		2,		/* id_of_commit */
		1,		/* num_parents */
		count_of(expected4), expected4
	);
}

/* test 3 way duplicate and merge
 * +----
 * | + |
 * | | +
 * +----
 * | + |
 * | | +
 */
TEST_F(graph_test_fxt, test_3_head_merge_and_duplicate)
{
	const chtype expected1[] = { ACS_BULLET, ACS_HLINE, ACS_TTEE, ACS_HLINE, ACS_URCORNER, ' ' };
	run_graph_test(
		{},		/* duplicate_ids */
		{ 2, 3 },	/* new_parent_ids */
		1,		/* id_of_commit */
		3,		/* num_parents */
		count_of(expected1), expected1
	);

	const chtype expected2[] = { ACS_VLINE, ' ', ACS_BULLET, ' ', ACS_VLINE, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected2), expected2
	);

	const chtype expected3[] = { ACS_VLINE, ' ', ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected3), expected3
	);

	const chtype expected4[] = { ACS_BULLET, ACS_HLINE, ACS_PLUS, ACS_HLINE, ACS_RTEE, ' ' };
	run_graph_test(
		{ 1, 3 },	/* duplicate_ids */
		{ 4, 5 },	/* new_parent_ids */
		2,		/* id_of_commit */
		3,		/* num_parents */
		count_of(expected4), expected4
	);

	const chtype expected5[] = { ACS_VLINE, ' ', ACS_BULLET, ' ', ACS_VLINE, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		4,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected5), expected5
	);

	const chtype expected6[] = { ACS_VLINE, ' ', ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		5,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected6), expected6
	);
}

/* test merge through a branch
 * +
 * | + 
 * +-|--
 * | | +
 * +-|--
 */
TEST_F(graph_test_fxt, test_merge_through_a_branch)
{
	const chtype expected1[] = { ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected1), expected1
	);

	const chtype expected2[] = { ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected2), expected2
	);

	const chtype expected3[] = { ACS_BULLET, ACS_HLINE, ACS_VLINE, ACS_HLINE, ACS_URCORNER, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{ 3 },	/* new_parent_ids */
		1,	/* id_of_commit */
		2,	/* num_parents */
		count_of(expected3), expected3
	);

	const chtype expected4[] = { ACS_VLINE, ' ', ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected4), expected4
	);

	const chtype expected5[] = { ACS_BULLET, ACS_HLINE, ACS_VLINE, ACS_HLINE, ACS_LRCORNER, ' ' };
	run_graph_test(
		{ 1 },	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected5), expected5
	);
}

/* test merge with empty space
 * +
 * | +
 * | | +
 * +-- |
 * +-- |
 */
TEST_F(graph_test_fxt, test_merge_with_empty_space)
{
	const chtype expected1[] = { ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected1), expected1
	);

	const chtype expected2[] = { ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected2), expected2
	);

	const chtype expected3[] = { ACS_VLINE, ' ', ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected3), expected3
	);

	const chtype expected4[] = { ACS_BULLET, ACS_HLINE, ACS_LRCORNER, ' ', ACS_VLINE, ' ' };
	run_graph_test(
		{ 2 },	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected4), expected4
	);

	const chtype expected5[] = { ACS_BULLET, ACS_HLINE, ACS_URCORNER, ' ', ACS_VLINE, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{ 4 },	/* new_parent_ids */
		1,	/* id_of_commit */
		2,	/* num_parents */
		count_of(expected5), expected5
	);
}

/* test basic collapse
 * +
 * | +
 * | | +
 * +-- |
 * + ---
 */
TEST_F(graph_test_fxt, test_basic_collapse)
{
	const chtype expected1[] = { ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected1), expected1
	);

	const chtype expected2[] = { ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected2), expected2
	);

	const chtype expected3[] = { ACS_VLINE, ' ', ACS_VLINE, ' ', ACS_BULLET, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected3), expected3
	);

	const chtype expected4[] = { ACS_BULLET, ACS_HLINE, ACS_LRCORNER, ' ', ACS_VLINE, ' ' };
	run_graph_test(
		{ 2 },	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected4), expected4
	);

	const chtype expected5[] = { ACS_BULLET, ' ', ACS_ULCORNER, ACS_HLINE, ACS_LRCORNER, ' ' };
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		count_of(expected5), expected5
	);
}
