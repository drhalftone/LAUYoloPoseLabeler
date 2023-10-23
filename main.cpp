#include "lauyoloposelabelerwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LAUYoloPoseLabelerWidget w;
    if (w.isValid()){
        w.show();
        return a.exec();
    }
    return -1;
}
