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

#ifndef DESELECTABLE_LIST_VIEW_H
#define DESELECTABLE_LIST_VIEW_H

#include <QListView>
#include <QMouseEvent>

/*!
 * \class deselectable_list_view
 * \brief Custom version of QListView which deselects rows when an already selected row is clicked again
 */
class deselectable_list_view : public QListView
{
	Q_OBJECT

public:
	deselectable_list_view(QWidget* parent);
	void mousePressEvent(QMouseEvent *e) override;
};

#endif /* DESELECTABLE_LIST_VIEW_H */
