#include "lauyoloposelabelerwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName(QString("Lau Consulting Inc"));
    a.setOrganizationDomain(QString("drhalftone.com"));
    a.setApplicationName(QString("LAUYoloPoseLabeler"));

    LAUYoloPoseLabelerWidget w;
    if (w.isValid()){
        w.show();
        return a.exec();
    }
    return -1;
}
