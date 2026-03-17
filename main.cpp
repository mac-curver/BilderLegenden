#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QStandardPaths>
#include <QtCore/qdir.h>
#include <QLibraryInfo>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName(APP_COMPANY);
    QCoreApplication::setOrganizationDomain(APP_DOMAIN);
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);

    QTranslator translator;
    //QLocale().setDefault(QLocale("de"));
    if (translator.load(QLocale(), "BilderLegenden", "_", ":/i18n")) {
        QCoreApplication::installTranslator(&translator);
    }
    QTranslator qtTranslator;
    qDebug() << QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if (qtTranslator.load(QLocale(), "qtbase", "_", ":/i18n")) {
        qApp->installTranslator(&qtTranslator);
    }

    MainWindow w;
    int mainResult = -1;

    w.awake();
    mainResult = a.exec();
    return mainResult;
}
