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

#include "about_window.h"
#include "ui_about_window.h"

#include "compat/cpp_git.h"
#include "util/version.h"

about_window::about_window(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::about_window)
{
	ui->setupUi(this);
	ui->label_reef_version->setText(reef_version.toString());
	ui->label_qt_version->setText(qVersion());
	ui->label_libgit2_version->setText(git::version().toString());
}

about_window::~about_window()
{
	delete ui;
}
