#ifndef DIALOGOUTPUTMAP_H
#define DIALOGOUTPUTMAP_H

#include <QtWidgets>

class DialogOutputMap : public QDialog
{
    Q_OBJECT
private:
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;

public:
    DialogOutputMap();
    ~DialogOutputMap();
};

#endif // DIALOGOUTPUTMAP_H
