#include <iostream>
#include <sstream>
#include <thread>

#include <QtWidgets>

#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setSwapInterval(1);
    QSurfaceFormat::setDefaultFormat(fmt);

    MainWindow w;
    w.setGeometry(
        QStyle::alignedRect(
               Qt::LeftToRight,
               Qt::AlignCenter,
               w.size(),
               qApp->desktop()->availableGeometry()
           )
    );
    w.show();
    return app.exec();
}
