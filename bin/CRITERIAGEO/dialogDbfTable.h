#ifndef DIALOGDBFTABLE_H
#define DIALOGDBFTABLE_H

#ifndef SHAPEHANDLER_H
    #include "shapeHandler.h"
#endif
#include "dialogDbfNewCol.h"
#include "tableDbf.h"

#define DEFAULT_INTEGER_MAX_DIGITS 9
#define DEFAULT_DOUBLE_MAX_DIGITS 13
#define DEFAULT_DOUBLE_MAX_DECIMALS 3

class DialogDbfTable : public QDialog
{
    Q_OBJECT

    private:

    Crit3DShapeHandler *shapeHandler;
    TableDbf* m_DBFTableWidget;
    DialogDbfNewCol* newColDialog;
    QMenuBar *menuBar;
    QMenu *editMenu;
    QAction *addRow;
    QAction *deleteRow;
    QAction *addCol;
    QAction *deleteCol;
    QAction *copyAll;
    QAction *save;
    QList<QString> labels;
    QList<QString> m_DBFTableHeader;

    public:
        DialogDbfTable(Crit3DShapeHandler *shapeHandler, QString fileName);
        ~DialogDbfTable();
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
        void menuRequested(const QPoint point);
};

#endif // DIALOGDBFTABLE_H
