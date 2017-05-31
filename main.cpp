#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>

int main(int argc, char *argv[])
{
//    QApplication a(argc, argv);

    QApplication app(argc, argv);

    QTranslator qtTranslator;
    qDebug() << "LOADING TRANSLATION" << QLocale::system().name();
    qDebug() << "TRANSLATION SUCCESFULLY LOADED:" << qtTranslator.load(QLocale::system().name(),
            ":/translations/");
    app.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    myappTranslator.load("myapp_" + QLocale::system().name());
    app.installTranslator(&myappTranslator);

    MainWindow w;
    w.show();

    return app.exec();
}
