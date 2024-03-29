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

#ifndef ABOUT_WINDOW_H
#define ABOUT_WINDOW_H

#include <QDialog>

namespace Ui {
class about_window;
}

class about_window : public QDialog
{
	Q_OBJECT

public:
	explicit about_window(QWidget *parent = nullptr);
	~about_window();

private:
	Ui::about_window *ui;
};

#endif // ABOUT_WINDOW_H
