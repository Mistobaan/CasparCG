#include "Global.h"

#include "MainWindow.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QTextStream>

#include <QtGui/QApplication>

#include <QtSql/QSqlDatabase>

void loadStyleSheets(QApplication& application)
{
    QString stylesheet;

    QFile defaultStylesheet(":/Appearances/Stylesheets/Default.css");
    if(defaultStylesheet.open(QFile::ReadOnly))
    {
        QTextStream stream(&defaultStylesheet);
        stylesheet = stream.readAll();
        defaultStylesheet.close();

        application.setStyleSheet(stylesheet);
    }
    defaultStylesheet.close();

    QFile extendedStylesheet(":/Appearances/Stylesheets/Extended.css");
    if(extendedStylesheet.open(QFile::ReadOnly))
    {
        QTextStream stream(&extendedStylesheet);
        stylesheet += stream.readAll();
        extendedStylesheet.close();

        application.setStyleSheet(stylesheet);
    }
    extendedStylesheet.close();

#ifdef Q_OS_UNIX
    QFile platformStylesheet(":/Appearances/Stylesheets/Unix.css");
#else
    QFile platformStylesheet(":/Appearances/Stylesheets/Windows.css");
#endif

    if(platformStylesheet.open(QFile::ReadOnly))
    {
        QTextStream stream(&platformStylesheet);
        stylesheet += stream.readAll();
        platformStylesheet.close();

        application.setStyleSheet(stylesheet);
    }
    platformStylesheet.close();
}

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    application.setStyle("plastique");

    loadStyleSheets(application);

    MainWindow window;
    window.show();

    return application.exec();
}
