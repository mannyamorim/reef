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

#include "include/commit_model.h"

#include "preferences.h"

constexpr size_t BLOCK_SIZE = 65536;

commit_model::commit_model(QObject *parent) :
	QAbstractTableModel(parent)
{
	add_block();
}

int commit_model::rowCount(const QModelIndex &parent) const
{
	(void)parent;
	return num_lines;
}

int commit_model::columnCount(const QModelIndex &parent) const
{
	(void)parent;
	return 1;
}

QVariant commit_model::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole)
		return QString::fromRawData(lines[index.row()].first, lines[index.column()].second);

	return QVariant();
}

void commit_model::add_line(const QChar *str, size_t size)
{
	beginInsertRows(QModelIndex(), num_lines, num_lines);

	QChar *line_memory = get_memory_for_line(size);
	memcpy(line_memory, str, size * sizeof(QChar));
	lines.push_back(std::make_pair(line_memory, size));

	num_lines++;

	endInsertRows();
}

void commit_model::add_block()
{
	blocks.push_back(std::make_unique<QChar[]>(BLOCK_SIZE));
}

QChar *commit_model::get_memory_for_line(size_t size)
{
	if (BLOCK_SIZE - block_usage < size) {
		add_block();
		block_usage = 0;
		curr_block++;
	}

	QChar *line_memory = &blocks[curr_block][block_usage];
	block_usage += size;
	return line_memory;
}
