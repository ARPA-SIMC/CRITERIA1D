#include <QtWidgets>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.setFixedSize(w.width(), w.height());
    w.setWindowTitle("Heat 1D");

    w.show();

    return a.exec();
}
