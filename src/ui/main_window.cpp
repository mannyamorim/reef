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

#include "main_window.h"
#include "./ui_main_window.h"

#include "compat/cpp_git.h"

#include <QFileDialog>

main_window::main_window(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::main_window)
{
	ui->setupUi(this);
	connect(ui->action_open_repository, &QAction::triggered, this, &main_window::handle_open_repository);
	connect(ui->action_close_repository, &QAction::triggered, this, &main_window::handle_close_repository);
	connect(ui->action_exit, &QAction::triggered, qApp, QApplication::quit);
	connect(ui->action_about, &QAction::triggered, this, &main_window::handle_about);
	ui->commit_table->setItemDelegateForColumn(0, &gdelegate);
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

void main_window::handle_close_repository()
{
	ui->commit_table->setModel(nullptr);
	ui->ref_tree->setModel(nullptr);
	repo_ctrl.reset();
}

void main_window::load_repo(std::string dir)
{
	std::function<void(const QString &)> update_status_func = [this] (const QString &message) {
		ui->statusbar->showMessage(message);
	};

	try {
		repo_ctrl = std::make_unique<repository_controller>(dir, update_status_func);
	} catch (git::libgit_error e) {
		ui->statusbar->showMessage(e.what());
		return;
	}

	repo_ctrl->display_refs();

	ui->commit_table->setModel(repo_ctrl->get_commit_model());
	ui->ref_tree->setModel(repo_ctrl->get_ref_model());

	qApp->processEvents();

	repo_ctrl->display_commits();
}

void main_window::handle_about()
{
	about_dialog = std::make_unique<about_window>(this);
	about_dialog->show();
}
