#include <iostream>
#include <sstream>
#include <thread>

#include <QtWidgets>

#include "ui/MainWindow.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	QSurfaceFormat fmt;
	fmt.setSwapInterval(1);
	QSurfaceFormat::setDefaultFormat(fmt);

	MainWindow w;
	w.setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, w.size(),
#if USE_QT_6
		qApp->primaryScreen()->availableGeometry()
#else
		qApp->desktop()->availableGeometry()
#endif
			));
	w.show();
	return app.exec();
}
