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

/* repository_controller.h */
#ifndef REPOSITORY_CONTROLLER_H
#define REPOSITORY_CONTROLLER_H

#include <string>
#include <functional>

#include <QString>

#include "cpp_git.h"
#include "commit_list.h"
#include "graph.h"
#include "ref_map.h"
#include "preferences.h"

class repository_controller
{
public:
	repository_controller(std::string &dir);

	void display_commits(std::function<void(const QChar *, size_t)> display_line);

private:
	git::repository repo;
	ref_map refs;
	preferences prefs;
	commit_list clist;
	graph_list glist;
};

#endif /* REPOSITORY_CONTROLLER_H */
