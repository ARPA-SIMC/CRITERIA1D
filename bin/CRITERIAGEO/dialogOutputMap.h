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
    TabMap *tabMap;
    TabSymbology *tabSymbology;

public:
    DialogOutputMap(QStringList varList);
    ~DialogOutputMap();
    void done(bool res);
};

#endif // DIALOGOUTPUTMAP_H
