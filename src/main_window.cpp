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

#include "main_window.h"
#include "./ui_main_window.h"

#include "cpp_git.h"

#include <QFileDialog>

main_window::main_window(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::main_window)
{
	ui->setupUi(this);
	connect(ui->action_open_repository, &QAction::triggered, this, &main_window::handle_open_repository);
}

main_window::~main_window()
{
	delete ui;
}

void main_window::handle_open_repository()
{
	QString dir_qstr = QFileDialog::getExistingDirectory(this, tr("Open Repository"));

	if (dir_qstr.length() > 0)
		load_repo(dir_qstr.toStdString());
}

void main_window::load_repo(std::string dir)
{
	repo_ctrl = std::make_unique<repository_controller>(dir);
	repo_ctrl->display_refs([this] (const char *ref) {
		ui->ref_list->addItem(QString::fromUtf8(ref));
	});

	ui->commit_table->setModel(repo_ctrl->get_commit_list_model());
	repo_ctrl->display_commits();
}
