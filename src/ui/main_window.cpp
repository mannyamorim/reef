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

	ref_list_title_bar = new dock_widget_title_bar(ui->dock_widget_ref_tree);
	file_list_title_bar = new dock_widget_title_bar(ui->dock_widget_commit_file);
	commit_info_title_bar = new dock_widget_title_bar(ui->dock_widget_commit_info);

	ui->dock_widget_ref_tree->setTitleBarWidget(ref_list_title_bar);
	ui->dock_widget_commit_file->setTitleBarWidget(file_list_title_bar);
	ui->dock_widget_commit_info->setTitleBarWidget(commit_info_title_bar);

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
	ui->commit_file_list->setModel(nullptr);
	ui->commit_info->setText(QString());
	repo_ctrl.reset();
}

void main_window::handle_about()
{
	about_dialog = std::make_unique<about_window>(this);
	about_dialog->show();
}

void main_window::handle_diff_view_visible(bool visible)
{
	if (visible)
		ui->stacked_widget->setCurrentIndex(1);
	else
		ui->stacked_widget->setCurrentIndex(0);
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
	ui->commit_file_list->setModel(repo_ctrl->get_commit_file_model());

	connect(ui->commit_table->selectionModel(), &QItemSelectionModel::currentRowChanged, &*repo_ctrl, &repository_controller::handle_commit_table_row_changed);
	connect(&*repo_ctrl, &repository_controller::commit_info_text_changed, ui->commit_info, &QTextBrowser::setText);
	connect(ui->commit_file_list->selectionModel(), &QItemSelectionModel::currentRowChanged, &*repo_ctrl, &repository_controller::handle_file_list_row_changed);
	connect(&*repo_ctrl, &repository_controller::diff_view_text_changed, ui->diff_view, &QTextEdit::setText);
	connect(&*repo_ctrl, &repository_controller::diff_view_visible, this, &main_window::handle_diff_view_visible);

	qApp->processEvents();

	repo_ctrl->display_commits();
}
