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

/* commit_model.h */
#ifndef TEXT_SCROLL_AREA_H
#define TEXT_SCROLL_AREA_H

#include <QAbstractTableModel>

#include <memory>
#include <vector>

class commit_model : public QAbstractTableModel
{
	Q_OBJECT

public:
	commit_model(QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	void add_line(const QChar *str, size_t size);

private:
	std::vector<std::unique_ptr<QChar[]>> blocks;
	std::vector<std::pair<QChar *, size_t>> lines;

	int block_usage = 0;	/* number of bytes of the current block that has been allocated */
	int curr_block = 0;	/* number of the current block */
	int num_lines = 0;	/* number of lines */

	void add_block();
	QChar *get_memory_for_line(size_t size);
};

#endif /* TEXT_SCROLL_AREA_H */
