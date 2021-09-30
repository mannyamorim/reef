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
