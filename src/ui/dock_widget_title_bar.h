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

#ifndef DOCK_WIDGET_TITLE_BAR_H
#define DOCK_WIDGET_TITLE_BAR_H

#include <QDockWidget>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>

class dock_widget_title_bar : public QWidget
{
	Q_OBJECT
public:
	explicit dock_widget_title_bar(QWidget *parent = nullptr);

public slots:
	void handle_float_button_click();

private:
	QDockWidget *dockWidget;
	QHBoxLayout *layout;
	QLabel *label;
	QPushButton *float_button;

};

#endif /* DOCK_WIDGET_TITLE_BAR_H */
