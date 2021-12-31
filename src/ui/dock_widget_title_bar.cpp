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

#include "dock_widget_title_bar.h"

#include <QStyle>

dock_widget_title_bar::dock_widget_title_bar(QWidget *parent) : QWidget(parent)
{
	dockWidget = qobject_cast<QDockWidget*>(parentWidget());

	label = new QLabel();
	label->setText(dockWidget->windowTitle());

	float_button = new QPushButton();
	float_button->setFlat(true);
	float_button->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
	float_button->setFixedSize(16, 16);
	connect(float_button, &QPushButton::clicked, this, &dock_widget_title_bar::handle_float_button_click);

	layout = new QHBoxLayout(this);
	layout->setContentsMargins(9, 9, 9, 0);
	layout->addWidget(label);
	layout->addStretch();
	layout->addWidget(float_button);

	setLayout(layout);
}

void dock_widget_title_bar::handle_float_button_click()
{
	dockWidget->setFloating(!dockWidget->isFloating());
}
