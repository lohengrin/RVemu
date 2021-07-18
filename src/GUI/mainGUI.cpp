#include "MainWindow.h"



#include <QApplication>

#include <iostream>
#include <iomanip>

int main(int argc, char** argv)
{


	QApplication app(argc, argv);
	MainWindow mwin;
	mwin.show();


	while (mwin.isVisible())
	{
		QApplication::processEvents();
	}

	return 0;
}
