#ifndef DIALOGOUTPUTMAP_H
#define DIALOGOUTPUTMAP_H

#include <QtWidgets>
#include "tabMap.h"
#include "tabSymbology.h"

class DialogOutputMap : public QDialog
{
    Q_OBJECT
private:
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
    QStringList varList;
    QDate firstDbDate;
    QDate lastDbDate;
    TabMap *tabMap;
    TabSymbology *tabSymbology;

public:
    DialogOutputMap(QStringList varList, QDate firstDate, QDate lastDate);
    ~DialogOutputMap();
    void done(bool res);
    QDate getTabMapStartDate() const;
    QDate getTabMapEndDate() const;
    QDate getTabMapDate() const;
    QString getTabMapVariable() const;
    QString getTabMapElab() const;
    bool isTabMapClimateComputation() const;
    QString getTabMapClimateComputation() const;
    int getTabMapTimeWindow() const;
    double getTabMapThreshold() const;
    QString getTabMapOutputName() const;
};

#endif // DIALOGOUTPUTMAP_H
