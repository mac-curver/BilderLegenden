#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    int mainResult = -1;

    w.awake();
    mainResult = a.exec();
    return mainResult;
}
