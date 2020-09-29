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
    QComboBox *elabList;
    QComboBox *climateCompList;
    QLabel *timeWindowLabel;
    QLineEdit *timeWindow;
    QLabel *thresholdLabel;
    QLineEdit *threshold;
    QLineEdit *fileNameEdit;
public:
    TabMap();
    void climateComputation(int state);
};

#endif // MAPTAB_H
