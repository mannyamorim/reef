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

/* repository_controller.h */
#ifndef REPOSITORY_CONTROLLER_H
#define REPOSITORY_CONTROLLER_H

#include <string>
#include <functional>

#include <QAbstractTableModel>
#include <QString>

#include "compat/cpp_git.h"
#include "core/commit_list.h"
#include "core/graph.h"
#include "core/ref_map.h"
#include "util/block_allocator.h"
#include "util/preferences.h"

class repository_controller;

class commit_model : public QAbstractTableModel
{
	friend class repository_controller;

	Q_OBJECT

public:
	commit_model(repository_controller &repo_ctrl, QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
	repository_controller &repo_ctrl;
};

class ref_model : public QAbstractItemModel
{
	friend class repository_controller;

	Q_OBJECT

public:
	ref_model(repository_controller &repo_ctrl, QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;

private:
	repository_controller &repo_ctrl;
};

class commit_file_model : public QAbstractListModel
{
	friend class repository_controller;

	Q_OBJECT

public:
	commit_file_model(repository_controller &repo_ctrl, QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	repository_controller &repo_ctrl;
};

class repository_controller : public QObject
{
	friend class commit_model;
	friend class ref_model;
	friend class commit_file_model;

	Q_OBJECT

public:
	repository_controller(std::string &dir, std::function<void(const QString &)> update_status_func);

	QAbstractItemModel *get_commit_model();
	QAbstractItemModel *get_ref_model();
	QAbstractItemModel *get_commit_file_model();

	void display_refs();
	void display_commits();
	void reload_commits();

public slots:
	void handle_commit_table_row_changed(const QModelIndex &current, const QModelIndex &previous);
	void handle_file_list_row_changed(const QModelIndex &current, const QModelIndex &previous);

signals:
	void commit_info_text_changed(QString text);
	void diff_view_text_changed(QString text);
	void diff_view_visible(bool visible);

private:
	struct commit_item
	{
		commit_item(const git_oid &commit_id, QByteArray &&graph, QString &&refs, QString &&summary);
		commit_item(const commit_item &) = delete;
		commit_item &operator=(const commit_item &) = delete;
		commit_item(commit_item &&other) noexcept;
		commit_item &operator=(commit_item &&) noexcept;

		git_oid commit_id;
		QByteArray graph;
		QString refs;
		QString summary;
	};

	struct ref_item
	{
		ref_item(QString &&name, ref_item *parent = nullptr);
		ref_item(QString &&name, ref_map::refs_ordered_map::iterator ref_iter, ref_item *parent = nullptr);
		ref_item(const commit_item &) = delete;
		ref_item &operator=(const commit_item &) = delete;
		ref_item(ref_item &&other) noexcept;
		ref_item &operator=(ref_item &&) noexcept;

		QString name;
		int index_in_parent;
		Qt::CheckState checked = Qt::Checked;
		ref_map::refs_ordered_map::iterator ref_iter;
		std::map<QString, ref_item> children_map;
		std::vector<std::pair<QString, ref_item>> children_vec;
		ref_item *parent = nullptr;

		void convert_to_vector();
	};

	git::repository repo;
	ref_map refs;
	preferences prefs;

	commit_list clist;
	graph_list glist;

	std::vector<commit_item> clist_items;
	commit_model clist_model;

	std::map<QString, ref_item> ref_items_map;
	std::vector<std::pair<QString, ref_item>> ref_items_vec;
	ref_model r_model;

	git::diff diff;
	git::patch patch;
	std::vector<const git_diff_delta *> cfile_items;
	commit_file_model cfile_model;

	block_allocator block_alloc;

	std::function<void(const QString &)> update_status_func;

	void insert_ref(const char *ref_name, ref_item *parent, std::map<QString, ref_item> &map, ref_map::refs_ordered_map::iterator ref_iter);
	void convert_ref_items_to_vectors();
};

#endif /* REPOSITORY_CONTROLLER_H */
