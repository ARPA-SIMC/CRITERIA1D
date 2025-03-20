#include "criteria1DWidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString title = "CRITERIA 1D PRO " + QString(CRITERIA_VERSION);

    Criteria1DWidget w;
    w.show();
    w.setWindowTitle(title);

    return a.exec();
}
