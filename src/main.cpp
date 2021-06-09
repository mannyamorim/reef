#include "main_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	/* init libgit */
	git::git_library_lock git_library_lock;

	main_window w;
	w.show();

	return a.exec();
}
