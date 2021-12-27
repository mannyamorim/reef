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

/* refs.h */
#ifndef REFS_H
#define REFS_H

#include <cstring>
#include <map>
#include <unordered_map>

#include <git2.h>

#include "compat/cpp_git.h"

/*!
 * \struct git_oid_ref_hash
 * \brief Structure for computing hashes of git_oid
 */
struct git_oid_ref_hash
{
	/*!
	 * \brief Compute the hash of a git_oid
	 * \param x The git_oid to hash
	 * \return The hash of x
	 */
	size_t operator()(const git_oid &x) const
	{
		size_t hash = 0;
		for (int i = 0; i < GIT_OID_RAWSZ; i++)
			hash ^= std::hash<unsigned char>{}(x.id[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		return hash;
	}
};

/*!
 * \struct git_oid_ref_cmp
 * \brief Structure for comparing git_oid
 */
struct git_oid_ref_cmp
{
	/*!
	 * \brief Compare two git_oids
	 * \param lhs The first git_oid to compare
	 * \param rhs The second git_oid to compare
	 * \return Boolean comparison result
	 */
	bool operator()(const git_oid &lhs, const git_oid &rhs) const
	{
		for (size_t i = 0; i < GIT_OID_RAWSZ; i++)
			if (lhs.id[i] != rhs.id[i]) return false;
		return true;
	}
};

/*!
 * \class ref_map
 * \brief Class for keeping track of all of the refs in a repo
 *
 * Refs can be accessed through an ordered map by the ref names.
 * Also through an unordered multimap by the git_oid.
 * Refs can be activated and de-activated.
 */
class ref_map {
public:
	ref_map(const git::repository &repo);
	~ref_map();

	/*!
	 * \struct char_ptr_cmp
	 * \brief Structure for comparing c strings by pointer
	 */
	struct char_ptr_cmp {
		/*!
		 * \brief Compare two c strings by their pointers
		 * \param lhs The first c string to compare
		 * \param rhs The second c string to compare
		 * \return Boolean comparison result
		 */
		bool operator()(const char *lhs, const char *rhs) const
		{
			return strcmp(lhs, rhs) < 0;
		}
	};

	using refs_unordered_multimap = std::unordered_multimap<git_oid, std::pair<git::reference, bool>, git_oid_ref_hash, git_oid_ref_cmp>;
	refs_unordered_multimap refs;
	using refs_ordered_map = std::map<const char *, std::pair<git_oid, std::pair<git::reference, bool> *>, char_ptr_cmp>;
	refs_ordered_map refs_ordered;
	
	/*!
	 * \brief Change the active status of a ref
	 * \param iter An iterator in the ordered ref map
	 * \param is_active Whether or not the ref should be active
	 */
	void set_ref_active(refs_ordered_map::iterator &iter, bool is_active);
};

#endif /* REFS_H */
