#ifndef TABMAP_H
#define TABMAP_H

#include <QtWidgets>

class TabMap : public QWidget
{
    Q_OBJECT
private:
    QStringList varList;
    QLabel *startDateLabel;
    QDateEdit *startDate;
    QLabel *endDateLabel;
    QDateEdit *endDate;
    QLabel *dateLabel;
    QDateEdit *date;
    QCheckBox *climateComp;
    QComboBox *variableList;
    QComboBox *elabList;
    QComboBox *climateCompList;
    QLabel *timeWindowLabel;
    QLineEdit *timeWindow;
    QLabel *thresholdLabel;
    QLineEdit *threshold;
    QLineEdit *fileNameEdit;
public:
    TabMap(QStringList varList);
    void climateComputation(int state);
    void listElaboration(const QString value);
};

#endif // MAPTAB_H
