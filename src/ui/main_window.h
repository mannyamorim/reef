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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <memory>

#include "controller/repository_controller.h"

#include "about_window.h"
#include "dock_widget_title_bar.h"
#include "graph_delegate.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class main_window; }
QT_END_NAMESPACE

class main_window : public QMainWindow
{
	Q_OBJECT

public:
	main_window(QWidget *parent = nullptr);
	~main_window();

public slots:
	void handle_open_repository();
	void handle_close_repository();
	void handle_about();
	void handle_diff_view_visible(bool visible);

private:
	Ui::main_window *ui;
	std::unique_ptr<dock_widget_title_bar> ref_list_title_bar, file_list_title_bar, commit_info_title_bar;

	graph_delegate gdelegate;
	std::unique_ptr<repository_controller> repo_ctrl;
	std::unique_ptr<about_window> about_dialog;

	void load_repo(std::string dir);
};
#endif // MAIN_WINDOW_H
