#ifndef DIALOGOUTPUTMAP_H
#define DIALOGOUTPUTMAP_H

#include <QtWidgets>

class DialogOutputMap : public QDialog
{
    Q_OBJECT
private:
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
    QStringList varList;

public:
    DialogOutputMap(QStringList varList);
    ~DialogOutputMap();
    void done(bool res);
};

#endif // DIALOGOUTPUTMAP_H
