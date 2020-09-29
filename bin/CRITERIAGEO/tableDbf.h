#ifndef TABLEDBF_H
#define TABLEDBF_H

#include <QTableWidget>
#include <QKeyEvent>
#include <QWidget>

// custom QTableWidget to implement keyPressEvent and achieve the copy/paste functionality

class TableDbf: public QTableWidget
{
Q_OBJECT
public:
    TableDbf();
    void keyPressEvent(QKeyEvent *event);
    void copySelection();

};

#endif // TABLEDBF_H
