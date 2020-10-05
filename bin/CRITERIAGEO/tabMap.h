#ifndef TABMAP_H
#define TABMAP_H

#include <QtWidgets>

class TabMap : public QWidget
{
    Q_OBJECT
private:
    QStringList varList;
    QDate firstDbDate;
    QDate lastDbDate;
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
    TabMap(QStringList varList, QDate firstDbDate, QDate lastDbDate);
    void climateComputation(int state);
    void listElaboration(const QString value);
    QDate getStartDate() const;
    QDate getEndDate() const;
    QDate getDate() const;
    QString getVariable() const;
    QString getElab() const;
    bool isClimateComputation() const;
    QString getClimateComputation() const;
    int getTimeWindow() const;
    double getThreshold() const;
    QString getOutputName() const;
};

#endif // MAPTAB_H
