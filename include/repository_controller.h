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

#include <QAbstractTableModel>
#include <QString>

#include "cpp_git.h"
#include "commit_list.h"
#include "graph.h"
#include "ref_map.h"
#include "preferences.h"

class commit_model;

class repository_controller
{
	friend class commit_model;

public:
	repository_controller(std::string &dir);

	QAbstractTableModel *get_commit_list_model();

	void display_refs(std::function<void(const char *)> display_ref);
	void display_commits();

private:
	struct commit_item
	{
		commit_item(const git_oid &commit_id, QString &&graph, QString &&refs, QString &&summary);
		commit_item(const commit_item &) = delete;
		commit_item &operator=(const commit_item &) = delete;
		commit_item(commit_item &&other) noexcept;
		commit_item &operator=(commit_item &&) noexcept;

		git_oid commit_id;
		QString graph;
		QString refs;
		QString summary;
	};

	git::repository repo;
	ref_map refs;
	preferences prefs;

	commit_list clist;
	graph_list glist;

	std::unique_ptr<commit_model> clist_model;
};

class commit_model : public QAbstractTableModel
{
	friend class repository_controller;

	Q_OBJECT

public:
	commit_model(QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
	std::vector<std::unique_ptr<QChar[]>> blocks;
	std::vector<repository_controller::commit_item> items;

	int block_usage = 0;	/* number of bytes of the current block that has been allocated */
	int curr_block = 0;	/* number of the current block */
	int num_items = 0;	/* number of items */

	void add_commit(const git_oid &commit_id,
			const QChar *graph_str, size_t graph_size,
			const QChar *refs_str, size_t refs_size,
			const QChar *summary_str, size_t summary_size);
	void add_block();
	QChar *get_memory_for_str(size_t size);
};

#endif /* REPOSITORY_CONTROLLER_H */
