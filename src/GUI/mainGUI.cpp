#include "MainWindow.h"

#include <QApplication>

#include <iostream>
#include <iomanip>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	app.setOrganizationName("Lohengrin");
	app.setApplicationName("RVemu");

	MainWindow mwin;
	mwin.show();


	while (mwin.isVisible())
	{
		QApplication::processEvents();
	}

	return 0;
}
