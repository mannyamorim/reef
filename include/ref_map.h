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

/* refs.h */
#ifndef REFS_H
#define REFS_H

#include <cstring>
#include <map>
#include <unordered_map>

#include <git2.h>

#include "cpp_git.h"

struct git_oid_ref_hash
{
	size_t operator()(const git_oid &x) const
	{
		size_t hash = 0;
		for (int i = 0; i < GIT_OID_RAWSZ; i++)
			hash ^= std::hash<unsigned char>{}(x.id[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		return hash;
	}
};

struct git_oid_ref_cmp
{
	bool operator()(const git_oid &lhs, const git_oid &rhs) const
	{
		for (size_t i = 0; i < GIT_OID_RAWSZ; i++)
			if (lhs.id[i] != rhs.id[i]) return false;
		return true;
	}
};

class ref_map {
public:
	ref_map(const git::repository &repo);
	~ref_map();

	struct char_ptr_cmp {
		bool operator()(const char *lhs, const char *rhs) const
		{
			return strcmp(lhs, rhs) < 0;
		}
	};

	using refs_unordered_multimap = std::unordered_multimap<git_oid, std::pair<git::reference, bool>, git_oid_ref_hash, git_oid_ref_cmp>;
	refs_unordered_multimap refs;
	using refs_ordered_map = std::map<const char *, std::pair<git_oid, std::pair<git::reference, bool> *>, char_ptr_cmp>;
	refs_ordered_map refs_ordered;
	
	void set_ref_active(refs_ordered_map::iterator &iter, bool is_active);
};

#endif /* REFS_H */
