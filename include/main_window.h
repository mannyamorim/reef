#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <memory>

#include "repository_controller.h"

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

private:
	Ui::main_window *ui;

	std::unique_ptr<repository_controller> repo_ctrl;
};
#endif // MAIN_WINDOW_H
