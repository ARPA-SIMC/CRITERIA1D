#ifndef TABMAP_H
#define TABMAP_H

#include <QtWidgets>

class TabMap : public QWidget
{
    Q_OBJECT
private:
    QDateEdit *startDate;
    QDateEdit *endDate;
    QCheckBox *climateComp;
    QLineEdit *timeWindow;
    QLineEdit *threshold;
    QLineEdit *fileNameEdit;
public:
    TabMap();
};

#endif // MAPTAB_H
