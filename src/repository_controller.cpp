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

#include "reef_string.h"
#include "repository_controller.h"

repository_controller::repository_controller(std::string &dir) :
	repo(dir.c_str()),
	refs(repo),
	prefs(),
	clist(refs, repo, prefs),
	glist(),
	clist_model(*this),
	r_model(*this)
{}

QAbstractItemModel *repository_controller::get_commit_model()
{
	return &clist_model;
}

QAbstractItemModel *repository_controller::get_ref_model()
{
	return &r_model;
}

void repository_controller::insert_ref(const char *ref_name, ref_item *parent, std::map<QString, ref_item> &map)
{
	const char *ptr = strchr(ref_name, '/');
	if (ptr != nullptr) {
		QString name = QString::fromUtf8(ref_name, ptr - ref_name);
		ref_item item((QString(name)), parent);
		auto it = map.insert(std::make_pair(name, std::move(item))).first;
		insert_ref(ptr + 1, &it->second, it->second.children);
	} else {
		QString name = QString::fromUtf8(ref_name);
		ref_item item((QString(name)), parent);
		map.insert(std::make_pair(name, std::move(item)));
	}
}

void repository_controller::display_refs()
{
	for (ref_map::refs_ordered_map::iterator it = refs.refs_ordered.begin(); it != refs.refs_ordered.end(); it++)
		insert_ref(it->first + sizeof("refs/") - 1, nullptr, ref_items);
}

void repository_controller::display_commits()
{
	while (!clist.empty()) {
		struct commit_graph_info graph;
		git::commit commit = clist.get_next_commit(graph);

		QChar graph_buf[preferences::max_line_length];
		size_t graph_size = glist.compute_graph(graph, graph_buf);

		QChar refs_buf[preferences::max_line_length];
		size_t refs_size = 0;

		if (refs.refs.count(*commit.id()) > 0) {
			auto ref_range = refs.refs.equal_range(*commit.id());
			for (auto &it = ref_range.first; it != ref_range.second; it++) {
				if (!it->second.second)
					/* ref is not active, don't show it */
					continue;

				add_utf8_str_to_buf(refs_buf, it->second.first.shorthand(), refs_size);
			}
		}

		QChar summary_buf[preferences::max_line_length];
		size_t summary_size = 0;
		const char *summary = commit.summary();
		add_utf8_str_to_buf(summary_buf, summary, summary_size);

		QChar *graph_str_memory = block_alloc.allocate<QChar>(graph_size);
		memcpy(graph_str_memory, graph_buf, graph_size * sizeof(QChar));

		QChar *refs_str_memory = block_alloc.allocate<QChar>(refs_size);
		memcpy(refs_str_memory, refs_buf, refs_size * sizeof(QChar));

		QChar *summary_str_memory = block_alloc.allocate<QChar>(summary_size);
		memcpy(summary_str_memory, summary_buf, summary_size * sizeof(QChar));

		clist_model.beginInsertRows(QModelIndex(), clist_items.size(), clist_items.size());

		clist_items.emplace_back(*commit.id(),
				QString::fromRawData(graph_str_memory, graph_size),
				QString::fromRawData(refs_str_memory, refs_size),
				QString::fromRawData(summary_str_memory, summary_size));

		clist_model.endInsertRows();
	}
}

repository_controller::commit_item::commit_item(const git_oid &commit_id, QString &&graph, QString &&refs, QString &&summary) :
	commit_id(commit_id),
	graph(std::move(graph)),
	refs(std::move(refs)),
	summary(std::move(summary))
{}

repository_controller::commit_item::commit_item(commit_item &&other) noexcept :
	commit_id(other.commit_id),
	graph(std::move(other.graph)),
	refs(std::move(other.refs)),
	summary(std::move(other.summary))
{}

repository_controller::commit_item &repository_controller::commit_item::operator=(commit_item &&other) noexcept
{
	commit_id = other.commit_id;
	graph = std::move(other.graph);
	refs = std::move(other.refs);
	summary = std::move(other.summary);

	return *this;
}

commit_model::commit_model(repository_controller &repo_ctrl, QObject *parent) :
	QAbstractTableModel(parent),
	repo_ctrl(repo_ctrl)
{}

int commit_model::rowCount(const QModelIndex &parent) const
{
	(void)parent;
	return repo_ctrl.clist_items.size();
}

int commit_model::columnCount(const QModelIndex &parent) const
{
	(void)parent;
	return 3;
}

QVariant commit_model::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case 0:
			return repo_ctrl.clist_items[index.row()].graph;
		case 1:
			return repo_ctrl.clist_items[index.row()].refs;
		case 2:
			return repo_ctrl.clist_items[index.row()].summary;
		}
	}

	return QVariant();
}

QVariant commit_model::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		switch (section) {
		case 0:
			return QString(tr("Graph"));
		case 1:
			return QString(tr("Refs"));
		case 2:
			return QString(tr("Summary"));
		}
	}

	return QVariant();
}

repository_controller::ref_item::ref_item(QString &&name, ref_item *parent) :
	name(std::move(name)),
	parent(parent)
{}

repository_controller::ref_item::ref_item(ref_item &&other) noexcept :
	name(std::move(other.name)),
	parent(other.parent)
{}

repository_controller::ref_item &repository_controller::ref_item::operator=(ref_item &&other) noexcept
{
	name = std::move(other.name);
	parent = other.parent;

	return *this;
}

ref_model::ref_model(repository_controller &repo_ctrl, QObject *parent) :
	QAbstractItemModel(parent),
	repo_ctrl(repo_ctrl)
{}

int ref_model::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return static_cast<repository_controller::ref_item *>(parent.internalPointer())->children.size();
	else
		return repo_ctrl.ref_items.size();
}

int ref_model::columnCount(const QModelIndex &parent) const
{
	(void)parent;
	return 1;
}

QVariant ref_model::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case 0:
			return static_cast<repository_controller::ref_item *>(index.internalPointer())->name;
		}
	}

	return QVariant();
}

QVariant ref_model::headerData(int section, Qt::Orientation orientation, int role) const
{
	(void)section;
	(void)orientation;
	(void)role;
	return QVariant();
}

Qt::ItemFlags ref_model::flags(const QModelIndex &index) const
{
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	else
		return Qt::NoItemFlags;
}

QModelIndex ref_model::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	std::map<QString, repository_controller::ref_item> &map = parent.isValid() ?
			static_cast<repository_controller::ref_item *>(parent.internalPointer())->children :
			repo_ctrl.ref_items;

	auto it = map.begin();
	for (int i = 0; i < row; i++)
		it++;

	return createIndex(row, column, &it->second);
}

QModelIndex ref_model::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	repository_controller::ref_item *item = static_cast<repository_controller::ref_item *>(index.internalPointer());
	if (item->parent == nullptr)
		return QModelIndex();
	else
		return createIndex(0, 0, item->parent); // TODO UPDATE INDEXES
}
