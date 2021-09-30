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

#include <QTest>

#include "core/graph.h"

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

/* structure for holding instructions for one step of the graph test case */
struct test_graph_step
{
	std::unordered_set<unsigned int> duplicate_ids;
	std::vector<unsigned int> new_parent_ids;
	const unsigned int id_of_commit;
	const unsigned int num_parents;
	const char32_t *expected;

	test_graph_step(std::unordered_set<unsigned int> &&duplicate_ids,
			std::vector<unsigned int> &&new_parent_ids,
			const unsigned int id_of_commit,
			const unsigned int num_parents,
			const char32_t *expected) :
		duplicate_ids(std::move(duplicate_ids)),
		new_parent_ids(std::move(new_parent_ids)),
		id_of_commit(id_of_commit),
		num_parents(num_parents),
		expected(expected)
	{}
};

Q_DECLARE_METATYPE(std::vector<test_graph_step>)

/* class for executing the graph tests */
class test_graph : public QObject
{
	Q_OBJECT

private:
	/* execute one step of the graph test case */
	void run_graph_test_step(graph_list &glist, test_graph_step &step)
	{
		commit_graph_info graph_info;
		graph_info.duplicate_ids = std::move(step.duplicate_ids);
		graph_info.new_parent_ids = std::move(step.new_parent_ids);
		graph_info.id_of_commit = step.id_of_commit;
		graph_info.num_parents = step.num_parents;
		graph_info.num_duplicates = graph_info.duplicate_ids.size();

		graph_char buf[preferences::max_line_length];
		size_t graph_size = glist.compute_graph(graph_info, buf);

		char32_t decoded_buf[preferences::max_line_length];
		for (size_t i = 0; i < graph_size; i++)
			decoded_buf[i] = test_line_drawing_chars[buf[i].flags];

		size_t expected_size = std::char_traits<char32_t>::length(step.expected);
		QCOMPARE(graph_size, expected_size);
		QCOMPARE(memcmp(decoded_buf, step.expected, expected_size * sizeof(char32_t)), 0);
	}

private slots:
	/* define all of the graph test cases */
	void run_graph_test_data()
	{
		QTest::addColumn<std::vector<test_graph_step>>("steps");

		/* basic test of the compute_graph method */
		QTest::newRow("test_single_commit") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({}, {}, 1, 1, U"• "),
		});

		/* test initial commit
		 * •
		 * │ •
		 * I │
		 */
		QTest::newRow("test_initial_commit") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({}, {}, 1, 1, U"• "),
			test_graph_step({}, {}, 2, 1, U"│ • "),
			test_graph_step({}, {}, 1, 0, U"I │ "),
		});

		/* test new branch
		 * •
		 * │ •
		 * • │
		 */
		QTest::newRow("test_new_branch") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({}, {}, 1, 1, U"• "),
			test_graph_step({}, {}, 2, 1, U"│ • "),
			test_graph_step({}, {}, 1, 1, U"• │ "),
		});

		/* test basic merge
		 * •
		 * •─┐
		 * │ •
		 */
		QTest::newRow("test_basic_merge") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({}, {},    1, 1, U"• "),
			test_graph_step({}, { 2 }, 1, 2, U"•─┐ "),
			test_graph_step({}, {},    2, 1, U"│ • "),
		});

		/* test basic duplicate
		 * •
		 * │ •
		 * •─┘
		 */
		QTest::newRow("test_basic_duplicate") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({},    {}, 1, 1, U"• "),
			test_graph_step({},    {}, 2, 1, U"│ • "),
			test_graph_step({ 2 }, {}, 1, 1, U"•─┘ "),
		});

		/* test 3 head merge
		 * •─┬─┐
		 * │ • │
		 * │ │ •
		 */
		QTest::newRow("test_3_head_merge") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({}, { 2, 3 }, 1, 3, U"•─┬─┐ "),
			test_graph_step({}, {},       2, 1, U"│ • │ "),
			test_graph_step({}, {},       3, 1, U"│ │ • "),
		});

		/* test 3 duplicate
		 * •
		 * │ •
		 * │ │ •
		 * •─┴─┘
		 */
		QTest::newRow("test_3_duplicate") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({},       {}, 1, 1, U"• "),
			test_graph_step({},       {}, 2, 1, U"│ • "),
			test_graph_step({},       {}, 3, 1, U"│ │ • "),
			test_graph_step({ 1, 3 }, {}, 2, 1, U"•─┴─┘ "),
		});

		/* test 3 way duplicate and merge
		 * •─┬─┐
		 * │ • │
		 * │ │ •
		 * •─┼─┤
		 * │ • │
		 * │ │ •
		 */
		QTest::newRow("test_3_head_merge_and_duplicate") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({},       { 2, 3 }, 1, 3, U"•─┬─┐ "),
			test_graph_step({},       {},       2, 1, U"│ • │ "),
			test_graph_step({},       {},       3, 1, U"│ │ • "),
			test_graph_step({ 1, 3 }, { 4, 5 }, 2, 3, U"•─┼─┤ "),
			test_graph_step({},       {},       4, 1, U"│ • │ "),
			test_graph_step({},       {},       5, 1, U"│ │ • "),
		});

		/* test merge through a branch
		 * •
		 * │ •
		 * •─│─┐
		 * │ │ •
		 * •─│─┘
		 */
		QTest::newRow("test_merge_through_a_branch") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({},    {},    1, 1, U"• "),
			test_graph_step({},    {},    2, 1, U"│ • "),
			test_graph_step({},    { 3 }, 1, 2, U"•─│─┐ "),
			test_graph_step({},    {},    3, 1, U"│ │ • "),
			test_graph_step({ 1 }, {},    3, 1, U"•─│─┘ "),
		});

		/* test merge with empty space
		 * •
		 * │ •
		 * │ │ •
		 * •─┘ │
		 * •─┐ │
		 */
		QTest::newRow("test_merge_with_empty_space") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({},    {},    1, 1, U"• "),
			test_graph_step({},    {},    2, 1, U"│ • "),
			test_graph_step({},    {},    3, 1, U"│ │ • "),
			test_graph_step({ 2 },    {}, 1, 1, U"•─┘ │ "),
			test_graph_step({},    { 4 }, 1, 2, U"•─┐ │ "),
		});

		/* test basic collapse
		 * •
		 * │ •
		 * │ │ •
		 * •─┘ │
		 * • ┌─┘
		 */
		QTest::newRow("test_basic_collapse") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({},    {}, 1, 1, U"• "),
			test_graph_step({},    {}, 2, 1, U"│ • "),
			test_graph_step({},    {}, 3, 1, U"│ │ • "),
			test_graph_step({ 2 }, {}, 1, 1, U"•─┘ │ "),
			test_graph_step({},    {}, 1, 1, U"• ┌─┘ "),
		});

		/* test double collapse
		 * •
		 * │ •
		 * │ │ •
		 * │ │ │ •
		 * •─┘ │ │
		 * • ┌─┘ │
		 * • │ ┌─┘
		 */
		QTest::newRow("test_double_collapse") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({},    {}, 1, 1, U"• "),
			test_graph_step({},    {}, 2, 1, U"│ • "),
			test_graph_step({},    {}, 3, 1, U"│ │ • "),
			test_graph_step({},    {}, 4, 1, U"│ │ │ • "),
			test_graph_step({ 2 }, {}, 1, 1, U"•─┘ │ │ "),
			test_graph_step({},    {}, 1, 1, U"• ┌─┘ │ "),
			test_graph_step({},    {}, 1, 1, U"• │ ┌─┘ "),
		});

		/* test long collapse
		 * •
		 * │ •
		 * │ │ •
		 * │ │ │ •
		 * •─┴─┘ │
		 * • ┌───┘
		 */
		QTest::newRow("test_long_collapse") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({},       {}, 1, 1, U"• "),
			test_graph_step({},       {}, 2, 1, U"│ • "),
			test_graph_step({},       {}, 3, 1, U"│ │ • "),
			test_graph_step({},       {}, 4, 1, U"│ │ │ • "),
			test_graph_step({ 2, 3 }, {}, 1, 1, U"•─┴─┘ │ "),
			test_graph_step({},       {}, 1, 1, U"• ┌───┘ "),
		});

		/* test commit collapse
		 * •
		 * │ •
		 * │ │ •
		 * │ │ │ •
		 * •─┴─┘ │
		 * │ •───┘
		 */
		QTest::newRow("test_commit_collapse") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({},       {}, 1, 1, U"• "),
			test_graph_step({},       {}, 2, 1, U"│ • "),
			test_graph_step({},       {}, 3, 1, U"│ │ • "),
			test_graph_step({},       {}, 4, 1, U"│ │ │ • "),
			test_graph_step({ 2, 3 }, {}, 1, 1, U"•─┴─┘ │ "),
			test_graph_step({},       {}, 4, 1, U"│ •───┘ "),
		});

		/* test blocked collapse
		 * •
		 * │ •
		 * │ │ •
		 * │ │ │ •
		 * •─┘ │ │
		 * •───│─┘
		 * • ┌─┘
		 */
		QTest::newRow("test_blocked_collapse") << std::vector<test_graph_step>({
			/* duplicate_ids, new_parent_ids, id_of_commit, num_parents, expected_string */
			test_graph_step({},    {}, 1, 1, U"• "),
			test_graph_step({},    {}, 2, 1, U"│ • "),
			test_graph_step({},    {}, 3, 1, U"│ │ • "),
			test_graph_step({},    {}, 4, 1, U"│ │ │ • "),
			test_graph_step({ 2 }, {}, 1, 1, U"•─┘ │ │ "),
			test_graph_step({ 4 }, {}, 1, 1, U"•───│─┘ "),
			test_graph_step({},    {}, 1, 1, U"• ┌─┘ "),
		});
	}

	/* execute the graph test case */
	void run_graph_test()
	{
		QFETCH(std::vector<test_graph_step>, steps);

		graph_list glist;

		for (test_graph_step &step : steps)
			run_graph_test_step(glist, step);
	}
};

QTEST_MAIN(test_graph)
#include "test_graph.moc"
