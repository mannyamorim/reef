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
	std::string dir = dir_qstr.toStdString();

	repo_ctrl = std::make_unique<repository_controller>(dir);
	repo_ctrl->display_commits([this] (const char *str, size_t size) {
		ui->text_area->add_line(str, size);
	});
}
