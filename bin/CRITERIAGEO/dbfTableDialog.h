#ifndef tableDBFDialog_H
#define tableDBFDialog_H

#ifndef SHAPEHANDLER_H
    #include "shapeHandler.h"
#endif
#include "dbfNewColDialog.h"

#define DEFAULT_INTEGER_MAX_DIGITS 9
#define DEFAULT_DOUBLE_MAX_DIGITS 13
#define DEFAULT_DOUBLE_MAX_DECIMALS 3

class DbfTableDialog : public QDialog
{
    Q_OBJECT

    private:

    Crit3DShapeHandler *shapeHandler;
    QTableWidget* m_DBFTableWidget;
    DbfNewColDialog* newColDialog;
    QMenuBar *menuBar;
    QMenu *editMenu;
    QAction *addRow;
    QAction *deleteRow;
    QAction *addCol;
    QAction *deleteCol;
    QAction *copy;
    QAction *save;
    QStringList labels;
    QStringList m_DBFTableHeader;

    public:
        DbfTableDialog(Crit3DShapeHandler *shapeHandler, QString fileName);
        ~DbfTableDialog();
        void addRowClicked();
        void removeRowClicked();
        void addColClicked();
        void removeColClicked();
        void cellChanged(int row, int column);
        void closeEvent(QCloseEvent *);
        void copyAllClicked();
        void saveChangesClicked();
        void horizontalHeaderClick(int index);
        void verticalHeaderClick(int index);
};

#endif // TableDBFDialog_H
