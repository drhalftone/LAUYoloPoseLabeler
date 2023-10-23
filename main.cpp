#include "lauyoloposelabelerwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LAUYoloPoseLabelerWidget w;
    w.show();
    return a.exec();
}
