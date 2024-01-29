#include <QApplication>

#include "soilWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Crit3DSoilWidget w;
    w.show();

    return a.exec();
}
