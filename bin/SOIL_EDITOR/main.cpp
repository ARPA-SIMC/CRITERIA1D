#include <QApplication>

#include "soilWidget.h"
#include "utilities.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString docPath;
    if (! searchDocPath(docPath))
    {
        QMessageBox::critical(nullptr, "", "/DOC/img/ directory is missing.");
        return -1;
    }

    Crit3DSoilWidget w(docPath + "img/");
    w.show();

    return a.exec();
}
