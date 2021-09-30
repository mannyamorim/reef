#include "about_window.h"
#include "ui_about_window.h"

#include "cpp_git.h"
#include "version.h"

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
