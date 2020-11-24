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

#include <assert.h>

#include <git2.h>

#include "cpp_git.h"
#include "ref_map.h"

ref_map::ref_map(const git::repository &repo)
{
	/* create an iterator to go through the refs */
	auto it = repo.get_reference_iterator();

	while (!it.finished()) {
		git::reference ref = it.get_reference();

		if (ref.is_note())
			continue;

		const git_oid *id = ref.target();

		git::reference resolved_ref(nullptr);
		if (id == NULL) {
			resolved_ref = ref.resolve();
			id = resolved_ref.target();
		}

		git_object *peeled_obj = NULL;
		if (ref.is_tag()) {
			git_reference_peel(&peeled_obj, ref._ptr(), GIT_OBJ_COMMIT);
			id = git_object_id(peeled_obj);
		}

		assert(id != NULL);

		const char *shorthand = ref.shorthand();
		std::pair<git::reference, bool> *pair_ref =
			&refs.insert(std::make_pair(*id, std::make_pair(std::move(ref), true)))->second;
		refs_ordered.insert(std::make_pair(shorthand, std::make_pair(*id, pair_ref)));

		git_object_free(peeled_obj);

		++it;
	}
}

ref_map::~ref_map()
{
	refs_ordered.clear();
	refs.clear();
}

void ref_map::set_ref_active(refs_ordered_map::iterator &iter, bool is_active)
{
	iter->second.second->second = is_active;
}
