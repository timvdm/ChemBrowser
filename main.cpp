#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setApplicationName("ChemBrowser");
    QCoreApplication::setOrganizationName("SourceForge");

    MainWindow w;
    w.show();
    return a.exec();
}
