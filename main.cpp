#include "lauyoloposelabelerwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setOrganizationName(QString("Lau Consulting Inc"));
    app.setOrganizationDomain(QString("drhalftone.com"));
    app.setApplicationName(QString("Fly's Eye Interlacing Tool"));
    app.setQuitOnLastWindowClosed(true);

    LAUYoloPoseLabelerWidget w;
    if (w.isValid()){
        w.show();
        return app.exec();
    }
    return -1;
}
