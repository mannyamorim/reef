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

/* cpp_git.h */
#ifndef CPP_GIT_H
#define CPP_GIT_H

#include <cstdarg>
#include <exception>
#include <memory>
#include <cstring>
#include <functional>
#include <string>

#include <git2.h>

namespace git
{
	class libgit_error : public std::exception
	{
	public:
		libgit_error(const int err_code) : err_code(err_code)
		{
			const git_error *err = giterr_last();
			if (err != NULL) {
				err_klass = err->klass;
				err_msg = std::string(err->message);
			}
		}

		virtual const char *what() const noexcept override
		{
			return err_msg.c_str();
		}

		int get_err_code() const noexcept
		{
			return err_code;
		}

		int get_err_klass() const noexcept
		{
			return err_klass;
		}

	private:
		const int err_code = 0;
		int err_klass = -1;
		std::string err_msg = "unknown error occured";
	};

	class git_library_lock
	{
	public:
		git_library_lock()
		{
			git_libgit2_init();
		}

		git_library_lock(const git_library_lock &) = delete;
		git_library_lock &operator=(const git_library_lock &) = delete;
		git_library_lock(git_library_lock &&) = delete;
		git_library_lock &operator=(git_library_lock &&) = delete;

		~git_library_lock()
		{
			git_libgit2_shutdown();
		}
	};

	class diff
	{
	public:
		diff(git_diff *ptr) : ptr(ptr) {}

		diff(const diff &) = delete;
		diff &operator=(const diff &) = delete;

		diff(diff &&other) noexcept
		{
			ptr = other.ptr;
			other.ptr = nullptr;
		}

		diff &operator=(diff &&other) noexcept
		{
			if (this != &other) {
				if (ptr != nullptr)
					git_diff_free(ptr);

				ptr = other.ptr;
				other.ptr = nullptr;
			}

			return *this;
		}

		~diff()
		{
			if (ptr != nullptr)
				git_diff_free(ptr);
		}

		git_diff *_ptr() const
		{
			return ptr;
		}

		using line_callback = std::function<int(const git_diff_delta *, const git_diff_hunk *, const git_diff_line *)>;
		void print(git_diff_format_t format, line_callback callback) const
		{
			const git_diff_line_cb print_cb = [](const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line, void *payload) {
				const line_callback *_callback = static_cast<line_callback *>(payload);
				return _callback->operator()(delta, hunk, line);
			};

			int err = git_diff_print(ptr, format, print_cb, &callback);
			if (err != 0)
				throw libgit_error(err);
		}

	private:
		git_diff *ptr;
	};

	class tree
	{
	public:
		tree(git_tree *ptr) : ptr(ptr) {}

		tree(const tree &) = delete;
		tree &operator=(const tree &) = delete;

		tree(tree &&other) noexcept
		{
			ptr = other.ptr;
			other.ptr = nullptr;
		}

		tree &operator=(tree &&other) noexcept
		{
			if (this != &other) {
				if (ptr != nullptr)
					git_tree_free(ptr);

				ptr = other.ptr;
				other.ptr = nullptr;
			}

			return *this;
		}

		~tree()
		{
			if (ptr != nullptr)
				git_tree_free(ptr);
		}

		git_tree *_ptr() const
		{
			return ptr;
		}

	private:
		git_tree *ptr;
	};

	class commit
	{
	public:
		commit(git_commit *ptr) : ptr(ptr) {}

		commit(const commit &other)
		{
			git_commit *commit = nullptr;
			int err = git_commit_dup(&commit, other.ptr);
			if (err != 0)
				throw libgit_error(err);
			ptr = commit;
		}

		commit &operator=(const commit &other)
		{
			if (this != &other) {
				git_commit *commit = nullptr;
				int err = git_commit_dup(&commit, other.ptr);
				if (err != 0)
					throw libgit_error(err);

				if (ptr != nullptr)
					git_commit_free(ptr);

				ptr = commit;
			}

			return *this;
		}

		commit(commit &&other) noexcept
		{
			ptr = other.ptr;
			other.ptr = nullptr;
		}

		commit &operator=(commit &&other) noexcept
		{
			if (this != &other) {
				if (ptr != nullptr)
					git_commit_free(ptr);

				ptr = other.ptr;
				other.ptr = nullptr;
			}

			return *this;
		}

		~commit()
		{
			if (ptr != nullptr)
				git_commit_free(ptr);
		}

		git_commit *_ptr() const
		{
			return ptr;
		}

		const git_oid *id() const
		{
			return git_commit_id(ptr);
		}

		git_time_t time() const
		{
			return git_commit_time(ptr);
		}

		unsigned int parentcount() const
		{
			return git_commit_parentcount(ptr);
		}

		const git_oid *parent_id(unsigned int n) const
		{
			return git_commit_parent_id(ptr, n);
		}

		const git_signature *author() const
		{
			return git_commit_author(ptr);
		}

		const char *summary() const
		{
			return git_commit_summary(ptr);
		}

		git::commit parent(unsigned int n) const
		{
			git_commit *commit;
			int err = git_commit_parent(&commit, ptr, n);
			if (err != 0)
				throw libgit_error(err);

			return git::commit(commit);
		}

		git::tree tree() const
		{
			git_tree *tree;
			int err = git_commit_tree(&tree, ptr);
			if (err != 0)
				throw libgit_error(err);

			return git::tree(tree);
		}

	private:
		git_commit *ptr;
	};

	class reference
	{
	public:
		reference(git_reference *ptr) : ptr(ptr) {}

		reference(const reference &old_commit) = delete;
		reference &operator=(const reference &other) = delete;

		reference(reference &&other) noexcept
		{
			ptr = other.ptr;
			other.ptr = nullptr;
		}

		reference &operator=(reference &&other) noexcept
		{
			if (this != &other) {
				if (ptr != nullptr)
					git_reference_free(ptr);

				ptr = other.ptr;
				other.ptr = nullptr;
			}

			return *this;
		}

		~reference()
		{
			if (ptr != nullptr)
				git_reference_free(ptr);
		}

		git_reference *_ptr() const
		{
			return ptr;
		}

		bool is_note() const
		{
			return git_reference_is_note(ptr);
		}

		bool is_tag() const
		{
			return git_reference_is_tag(ptr);
		}

		git::reference resolve() const
		{
			git_reference *resolved_ref;
			int err = git_reference_resolve(&resolved_ref, ptr);
			if (err != 0)
				throw libgit_error(err);

			return git::reference(resolved_ref);
		}

		const git_oid *target() const
		{
			return git_reference_target(ptr);
		}

		const char *shorthand() const
		{
			return git_reference_shorthand(ptr);
		}

	private:
		git_reference *ptr;
	};

	class repository
	{
	public:
		repository(const char *repo_path)
		{
			const int err = git_repository_open(&ptr, repo_path);
			if (err != 0)
				throw libgit_error(err);
		}

		repository(const repository &) = delete;
		repository &operator=(const repository &) = delete;
		repository(repository &&) = delete;
		repository &operator=(repository &&) = delete;

		~repository()
		{
			git_repository_free(ptr);
		}

		git_repository *_ptr() const
		{
			return ptr;
		}

		git::commit commit_lookup(const git_oid *oid) const
		{
			git_commit *commit;
			int err = git_commit_lookup(&commit, ptr, oid);
			if (err != 0)
				throw libgit_error(err);

			return git::commit(commit);
		}

		git::diff diff_tree_to_tree(const git::tree &old_tree, const git::tree &new_tree, const git_diff_options *opts) const
		{
			git_diff *diff;
			int err = git_diff_tree_to_tree(&diff, ptr, old_tree._ptr(), new_tree._ptr(), opts);
			if (err != 0)
				throw libgit_error(err);

			return git::diff(diff);
		}

		class reference_iterator
		{
		public:
			reference_iterator(git_reference_iterator *ptr) : ptr(ptr), ref(nullptr)
			{
				git_reference *new_ref;
				int err = git_reference_next(&new_ref, ptr);
				if (err != 0 && err != GIT_ITEROVER)
					throw libgit_error(err);

				ref = git::reference(new_ref);
			}

			reference_iterator(const reference &old_commit) = delete;
			reference_iterator &operator=(const reference_iterator &other) = delete;

			reference_iterator(reference_iterator &&other) noexcept : ref(std::move(other.ref))
			{
				ptr = other.ptr;
				other.ptr = nullptr;
			}

			reference_iterator &operator=(reference_iterator &&other) = delete;

			~reference_iterator()
			{
				if (ptr != nullptr)
					git_reference_iterator_free(ptr);
			}

			git_reference_iterator *_ptr() const
			{
				return ptr;
			}

			git::reference&& get_reference()
			{
				return std::move(ref);
			}

			const reference_iterator &operator++()
			{
				git_reference *new_ref;
				int err = git_reference_next(&new_ref, ptr);
				if (err == GIT_ITEROVER) {
					end = true;
					ref = git::reference(nullptr);
					return *this;
				} else if (err != 0) {
					throw libgit_error(err);
				}

				ref = git::reference(new_ref);

				return *this;
			}

			bool finished()
			{
				return end;
			}

		private:
			bool end = false;
			git::reference ref;
			git_reference_iterator *ptr;

		};

		reference_iterator get_reference_iterator() const
		{
			git_reference_iterator *iter;
			int err = git_reference_iterator_new(&iter, ptr);
			if (err != 0)
				throw libgit_error(err);

			return reference_iterator(iter);
		}

		static std::string discover(const char *start_path)
		{
			git_buf repo_path = GIT_BUF_INIT_CONST(nullptr, 0);
			int err = git_repository_discover(&repo_path, start_path, false, nullptr);
			if (err != 0) {
				git_buf_free(&repo_path);
				throw libgit_error(err);
			}

			std::string path(repo_path.ptr);

			git_buf_free(&repo_path);
			return std::move(path);
		}

	private:
		git_repository *ptr;
	};
}

#endif /* CPP_GIT_H */
