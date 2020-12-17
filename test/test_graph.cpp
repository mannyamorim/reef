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

char32_t test_line_drawing_chars[] = {
	U' ', /* 00 = G_EMPTY                              */
	U' ', /* 01 = G_LEFT                               */
	U' ', /* 02 =          G_RIGHT                     */
	U'─', /* 03 = G_LEFT | G_RIGHT                     */

	U' ', /* 04 =                    G_UPPER           */
	U'┘', /* 05 = G_LEFT           | G_UPPER           */
	U'└', /* 06 =          G_RIGHT | G_UPPER           */
	U'┴', /* 07 = G_LEFT | G_RIGHT | G_UPPER           */

	U' ', /* 08 =                              G_LOWER */
	U'┐', /* 09 = G_LEFT                     | G_LOWER */
	U'┌', /* 10 =          G_RIGHT           | G_LOWER */
	U'┬', /* 11 = G_LEFT | G_RIGHT           | G_LOWER */

	U'│', /* 12 =                    G_UPPER | G_LOWER */
	U'┤', /* 13 = G_LEFT           | G_UPPER | G_LOWER */
	U'├', /* 14 =          G_RIGHT | G_UPPER | G_LOWER */
	U'┼', /* 15 = G_LEFT | G_RIGHT | G_UPPER | G_LOWER */

	U'•', /* 16 = G_MARK                               */
	U'I', /* 17 = G_INITIAL                            */
};

/* fixture for testing methods of the graph class */
class graph_test_fxt : public testing::Test
{
protected:
	graph_list glist;
	char8_t buf[MAX_LINE_LENGTH];
	utf8proc_int32_t codepoints[1024];

	void run_graph_test(
		std::unordered_set<unsigned int> &&duplicate_ids,
		std::vector<unsigned int> &&new_parent_ids,
		const unsigned int id_of_commit,
		const unsigned int num_parents,
		const char32_t *expected)
	{
		commit_graph_info graph_info;
		graph_info.duplicate_ids = std::move(duplicate_ids);
		graph_info.new_parent_ids = std::move(new_parent_ids);
		graph_info.id_of_commit = id_of_commit;
		graph_info.num_parents = num_parents;
		graph_info.num_duplicates = graph_info.duplicate_ids.size();

		size_t graph_size = glist.compute_graph(graph_info, buf);

		/* decode the UTF8 into UTF32 codepoints */
		size_t codepoints_decoded = utf8proc_decompose(buf, graph_size, codepoints, 1024, UTF8PROC_COMPOSE);

		for (size_t i = 0; i < codepoints_decoded; i++) {
			if (codepoints[i] >= 0x100000) {
				/* we found a marker to switch colors and potentially write out a box drawing character */
				unsigned char new_color;
				bool is_box_drawing_char;
				unsigned char box_drawing_flags;
				unpack_runchar(codepoints[i], new_color, is_box_drawing_char, box_drawing_flags);

				if (is_box_drawing_char) {
					codepoints[i] = test_line_drawing_chars[box_drawing_flags];
				}
			}
		}

		size_t expected_size = std::char_traits<char32_t>::length(expected);
		EXPECT_EQ(codepoints_decoded, expected_size);
		EXPECT_EQ(memcmp(codepoints, expected, expected_size * sizeof(char32_t)), 0);
	}
};

/* basic test of the compute_graph method */
TEST_F(graph_test_fxt, test_single_commit)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);
}

/* test initial commit
 * •
 * │ •
 * I │
 */
TEST_F(graph_test_fxt, test_initial_commit)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		0,	/* num_parents */
		U"I │ "
	);
}

/* test new branch
 * •
 * │ •
 * • │
 */
TEST_F(graph_test_fxt, test_new_branch)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• │ "
	);
}

/* test basic merge
 * •
 * •─┐
 * │ •
 */
TEST_F(graph_test_fxt, test_basic_merge)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{ 2 },	/* new_parent_ids */
		1,	/* id_of_commit */
		2,	/* num_parents */
		U"•─┐ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • "
	);
}

/* test basic duplicate
 * •
 * │ •
 * •─┘
 */
TEST_F(graph_test_fxt, test_basic_duplicate)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • "
	);

	run_graph_test(
		{ 2 },	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"•─┘ "
	);
}

/* test 3 head merge
 * •─┬─┐
 * │ • │
 * │ │ •
 */
TEST_F(graph_test_fxt, test_3_head_merge)
{
	run_graph_test(
		{},		/* duplicate_ids */
		{ 2, 3 },	/* new_parent_ids */
		1,		/* id_of_commit */
		3,		/* num_parents */
		U"•─┬─┐ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • │ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ • "
	);
}

/* test 3 duplicate
 * •
 * │ •
 * │ │ •
 * •─┴─┘
 */
TEST_F(graph_test_fxt, test_3_duplicate)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ • "
	);

	run_graph_test(
		{ 1, 3 },	/* duplicate_ids */
		{},		/* new_parent_ids */
		2,		/* id_of_commit */
		1,		/* num_parents */
		U"•─┴─┘ "
	);
}

/* test 3 way duplicate and merge
 * •─┬─┐
 * │ • │
 * │ │ •
 * •─┼─┤
 * │ • │
 * │ │ •
 */
TEST_F(graph_test_fxt, test_3_head_merge_and_duplicate)
{
	run_graph_test(
		{},		/* duplicate_ids */
		{ 2, 3 },	/* new_parent_ids */
		1,		/* id_of_commit */
		3,		/* num_parents */
		U"•─┬─┐ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • │ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ • "
	);

	run_graph_test(
		{ 1, 3 },	/* duplicate_ids */
		{ 4, 5 },	/* new_parent_ids */
		2,		/* id_of_commit */
		3,		/* num_parents */
		U"•─┼─┤ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		4,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • │ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		5,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ • "
	);
}

/* test merge through a branch
 * •
 * │ •
 * •─│─┐
 * │ │ •
 * •─│─┘
 */
TEST_F(graph_test_fxt, test_merge_through_a_branch)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{ 3 },	/* new_parent_ids */
		1,	/* id_of_commit */
		2,	/* num_parents */
		U"•─│─┐ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ • "
	);

	run_graph_test(
		{ 1 },	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		U"•─│─┘ "
	);
}

/* test merge with empty space
 * •
 * │ •
 * │ │ •
 * •─┘ │
 * •─┐ │
 */
TEST_F(graph_test_fxt, test_merge_with_empty_space)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ • "
	);

	run_graph_test(
		{ 2 },	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"•─┘ │ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{ 4 },	/* new_parent_ids */
		1,	/* id_of_commit */
		2,	/* num_parents */
		U"•─┐ │ "
	);
}

/* test basic collapse
 * •
 * │ •
 * │ │ •
 * •─┘ │
 * • ┌─┘
 */
TEST_F(graph_test_fxt, test_basic_collapse)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ • "
	);

	run_graph_test(
		{ 2 },	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"•─┘ │ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• ┌─┘ "
	);
}

/* test double collapse
 * •
 * │ •
 * │ │ •
 * │ │ │ •
 * •─┘ │ │
 * • ┌─┘ │
 * • │ ┌─┘
 */
TEST_F(graph_test_fxt, test_double_collapse)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		4,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ │ • "
	);

	run_graph_test(
		{ 2 },	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"•─┘ │ │ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• ┌─┘ │ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• │ ┌─┘ "
	);
}

/* test long collapse
 * •
 * │ •
 * │ │ •
 * │ │ │ •
 * •─┴─┘ │
 * • ┌───┘
 */
TEST_F(graph_test_fxt, test_long_collapse)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		4,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ │ • "
	);

	run_graph_test(
		{ 2, 3 },	/* duplicate_ids */
		{},		/* new_parent_ids */
		1,		/* id_of_commit */
		1,		/* num_parents */
		U"•─┴─┘ │ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• ┌───┘ "
	);
}

/* test commit collapse
 * •
 * │ •
 * │ │ •
 * │ │ │ •
 * •─┴─┘ │
 * │ •───┘
 */
TEST_F(graph_test_fxt, test_commit_collapse)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		4,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ │ • "
	);

	run_graph_test(
		{ 2, 3 },	/* duplicate_ids */
		{},		/* new_parent_ids */
		1,		/* id_of_commit */
		1,		/* num_parents */
		U"•─┴─┘ │ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		4,	/* id_of_commit */
		1,	/* num_parents */
		U"│ •───┘ "
	);
}

/* test blocked collapse
 * •
 * │ •
 * │ │ •
 * │ │ │ •
 * •─┘ │ │
 * •───│─┘
 * • ┌─┘
 */
TEST_F(graph_test_fxt, test_blocked_collapse)
{
	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		2,	/* id_of_commit */
		1,	/* num_parents */
		U"│ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		3,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ • "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		4,	/* id_of_commit */
		1,	/* num_parents */
		U"│ │ │ • "
	);

	run_graph_test(
		{ 2 },	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"•─┘ │ │ "
	);

	run_graph_test(
		{ 4 },	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"•───│─┘ "
	);

	run_graph_test(
		{},	/* duplicate_ids */
		{},	/* new_parent_ids */
		1,	/* id_of_commit */
		1,	/* num_parents */
		U"• ┌─┘ "
	);
}
