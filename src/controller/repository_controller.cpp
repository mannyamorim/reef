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

#include <chrono>
#include <iterator>
#include <functional>

#include <QApplication>
#include <QFontDatabase>

#include "util/reef_string.h"

#include "repository_controller.h"

repository_controller::repository_controller(std::string &dir, std::function<void(const QString &)> update_status_func) :
	repo(dir.c_str()),
	refs(repo),
	prefs(),
	clist(refs, repo, prefs),
	glist(),
	clist_model(*this),
	r_model(*this),
	update_status_func(update_status_func)
{}

QAbstractItemModel *repository_controller::get_commit_model()
{
	return &clist_model;
}

QAbstractItemModel *repository_controller::get_ref_model()
{
	return &r_model;
}

QString repository_controller::get_commit_info_by_row(int row)
{
	git_oid *oid = &clist_items[row].commit_id;
	git::commit commit = clist.get_commit_by_id(oid);
	return QString(commit.message());
}

void repository_controller::insert_ref(const char *ref_name, ref_item *parent, std::map<QString, ref_item> &map, ref_map::refs_ordered_map::iterator ref_iter)
{
	const char *ptr = strchr(ref_name, '/');
	if (ptr != nullptr) {
		QString name = QString::fromUtf8(ref_name, ptr - ref_name);
		ref_item item((QString(name)), parent);
		auto it = map.insert(std::make_pair(name, std::move(item))).first;
		insert_ref(ptr + 1, &it->second, it->second.children_map, ref_iter);
	} else {
		QString name = QString::fromUtf8(ref_name);
		ref_item item((QString(name)), ref_iter, parent);
		map.insert(std::make_pair(name, std::move(item)));
	}
}

void repository_controller::convert_ref_items_to_vectors()
{
	ref_items_vec.insert(
			ref_items_vec.begin(),
			std::make_move_iterator(ref_items_map.begin()),
			std::make_move_iterator(ref_items_map.end()));

	ref_items_map.clear();

	for (auto &it : ref_items_vec)
		it.second.convert_to_vector();
}

void repository_controller::display_refs()
{
	for (ref_map::refs_ordered_map::iterator it = refs.refs_ordered.begin(); it != refs.refs_ordered.end(); it++)
		insert_ref(it->first + sizeof("refs/") - 1, nullptr, ref_items_map, it);

	convert_ref_items_to_vectors();
}

void repository_controller::display_commits()
{
	size_t i = 0;
	auto last_event_loop_time = std::chrono::steady_clock::now();

	while (!clist.empty()) {
		struct commit_graph_info graph;
		git::commit commit = clist.get_next_commit(graph);

		graph_char graph_buf[preferences::max_line_length];
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

		char *graph_str_memory = block_alloc.allocate<char>(graph_size * sizeof(graph_char));
		memcpy(graph_str_memory, graph_buf, graph_size * sizeof(graph_char));

		QChar *refs_str_memory = block_alloc.allocate<QChar>(refs_size);
		memcpy(refs_str_memory, refs_buf, refs_size * sizeof(QChar));

		QChar *summary_str_memory = block_alloc.allocate<QChar>(summary_size);
		memcpy(summary_str_memory, summary_buf, summary_size * sizeof(QChar));

		clist_model.beginInsertRows(QModelIndex(), clist_items.size(), clist_items.size());

		clist_items.emplace_back(*commit.id(),
				QByteArray::fromRawData(graph_str_memory, graph_size * sizeof(graph_char)),
				QString::fromRawData(refs_str_memory, refs_size),
				QString::fromRawData(summary_str_memory, summary_size));

		clist_model.endInsertRows();

		i++;
		const auto now = std::chrono::steady_clock::now();
		const long duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_event_loop_time).count();

		if (duration > preferences::window_update_interval) {
			update_status_func(QString::number(i));
			qApp->processEvents();
			last_event_loop_time = now;
		}
	}
}

void repository_controller::reload_commits()
{
	clist.initialize(refs);
	glist.initialize();

	clist_model.beginRemoveRows(QModelIndex(), 0, clist_items.size() - 1);

	clist_items.clear();
	block_alloc.clear();

	clist_model.endRemoveRows();

	display_commits();
}

repository_controller::commit_item::commit_item(const git_oid &commit_id, QByteArray &&graph, QString &&refs, QString &&summary) :
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
	if (!index.isValid())
		return QVariant();

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

	if (role == Qt::FontRole) {
		switch (index.column()) {
		case 0:
			return QFontDatabase::systemFont(QFontDatabase::FixedFont);
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

repository_controller::ref_item::ref_item(QString &&name, ref_map::refs_ordered_map::iterator ref_iter, ref_item *parent) :
	name(std::move(name)),
	ref_iter(ref_iter),
	parent(parent)
{}

repository_controller::ref_item::ref_item(ref_item &&other) noexcept :
	name(std::move(other.name)),
	ref_iter(other.ref_iter),
	children_map(std::move(other.children_map)),
	children_vec(std::move(other.children_vec)),
	parent(other.parent)
{}

repository_controller::ref_item &repository_controller::ref_item::operator=(ref_item &&other) noexcept
{
	name = std::move(other.name);
	ref_iter = other.ref_iter;
	children_map = std::move(other.children_map);
	children_vec = std::move(other.children_vec);
	parent = other.parent;

	return *this;
}

void repository_controller::ref_item::convert_to_vector()
{
	/* if there is only one child collapse the hierarchy */
	while (children_map.size() == 1) {
		std::pair<QString, ref_item> pair = std::move(*std::make_move_iterator(children_map.begin()));
		name = name + "/" + pair.second.name;
		ref_iter = std::move(pair.second.ref_iter);
		children_map = std::move(pair.second.children_map);
	}

	/* move and convert the children */
	children_vec.insert(
			children_vec.begin(),
			std::make_move_iterator(children_map.begin()),
			std::make_move_iterator(children_map.end()));

	children_map.clear();

	for (size_t i = 0; i < children_vec.size(); i++) {
		children_vec[i].second.index_in_parent = i;
		children_vec[i].second.parent = this;
		children_vec[i].second.convert_to_vector();
	}
}

ref_model::ref_model(repository_controller &repo_ctrl, QObject *parent) :
	QAbstractItemModel(parent),
	repo_ctrl(repo_ctrl)
{}

int ref_model::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return static_cast<repository_controller::ref_item *>(parent.internalPointer())->children_vec.size();
	else
		return repo_ctrl.ref_items_vec.size();
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

	auto item = static_cast<repository_controller::ref_item *>(index.internalPointer());

	if (role == Qt::CheckStateRole) {
		return item->checked;
	}

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case 0:
			return item->name;
		}
	}

	return QVariant();
}

bool ref_model::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid())
		return false;

	if (role == Qt::CheckStateRole) {
		Qt::CheckState state = value.value<Qt::CheckState>();

		/* set the check state for all of the child items recursively */
		std::function<void(const QModelIndex)> set_children = [this, state, &set_children] (const QModelIndex &index) {
			auto item = static_cast<repository_controller::ref_item *>(index.internalPointer());
			item->checked = state;
			if (item->children_vec.size() == 0)
				repo_ctrl.refs.set_ref_active(item->ref_iter, state == Qt::Checked);

			for (size_t i = 0; i < item->children_vec.size(); i++)
				set_children(ref_model::index(i, 0, index));

			int rows = rowCount(index);
			if (rows > 0)
				emit dataChanged(
						ref_model::index(0, 0, index),
						ref_model::index(rows - 1, 0, index),
						{ Qt::CheckStateRole });
		};

		set_children(index);

		/* iterate up through the parents and set the check state */
		QModelIndex index_it = index;
		while (true) {
			auto item = static_cast<repository_controller::ref_item *>(index_it.internalPointer());
			if (item->parent != nullptr) {
				Qt::CheckState new_parent_state = item->checked;
				for (auto &it : item->parent->children_vec) {
					if (new_parent_state != it.second.checked) {
						new_parent_state = Qt::PartiallyChecked;
						break;
					}
				}

				if (new_parent_state != item->parent->checked) {
					item->parent->checked = new_parent_state;
					index_it = parent(index_it);
					emit dataChanged(
							ref_model::index(0, 0, index),
							ref_model::index(0, 0, index),
							{ Qt::CheckStateRole });
				} else {
					/* parent didn't change, short circuit */
					break;
				}
			} else {
				/* no more parents */
				break;
			}
		}

		repo_ctrl.reload_commits();

		return true;
	}

	return false;
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
	if (index.isValid()) {
		if (rowCount(index) == 0)
			return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
		else
			return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate;

	} else {
		return Qt::NoItemFlags;
	}
}

QModelIndex ref_model::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	std::vector<std::pair<QString, repository_controller::ref_item>> &vec = parent.isValid() ?
			static_cast<repository_controller::ref_item *>(parent.internalPointer())->children_vec :
			repo_ctrl.ref_items_vec;

	return createIndex(row, column, &vec[row].second);
}

QModelIndex ref_model::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	repository_controller::ref_item *item = static_cast<repository_controller::ref_item *>(index.internalPointer());
	if (item->parent == nullptr)
		return QModelIndex();
	else
		return createIndex(item->index_in_parent, 0, item->parent);
}
