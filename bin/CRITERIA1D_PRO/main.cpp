#include <QApplication>

#include "criteria1DWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Criteria1DWidget w;
    w.show();
    w.setWindowTitle(QStringLiteral("CRITERIA 1D_PRO"));

    return a.exec();
}
