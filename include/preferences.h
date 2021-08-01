/*
 * Reef - TUI Client for Git
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

/* preferences.h */
#ifndef PREFERENCES_H
#define PREFERENCES_H

class preferences
{
public:
	/* user controllable properties */
	/* the number of spaces that a tab is displayed as */
	const size_t tab_length = 8;

	/* the maximum length of a timestamp anomaly in the graph */
	const int graph_approximation_factor = 32;

	/* non user controllable properties */
	/* the maximum line length */
	static constexpr size_t max_line_length = 1024;

	/* the interval in milliseconds for often the UI events should be processed */
	static constexpr long window_update_interval = 50;
};

#endif /* PREFERENCES_H */
