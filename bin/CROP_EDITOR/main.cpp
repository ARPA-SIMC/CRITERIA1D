#include <QApplication>

#include "cropWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Crit3DCropWidget w;
    w.show();

    return a.exec();
}
