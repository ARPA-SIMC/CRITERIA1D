#ifndef NEWCOLDIALOG_H
#define NEWCOLDIALOG_H

#include <QtWidgets>

class DbfNewColDialog : public QDialog
{
    Q_OBJECT

    private:
        QLineEdit* name;
        QRadioButton *stringButton;
        QRadioButton *intButton;
        QRadioButton *doubleButton;
        QLineEdit* nWidth;
        bool insertOK;


    public:

        enum eType {
            FTString = 0,
            FTInteger,
            FTDouble
        };

        DbfNewColDialog();
        ~DbfNewColDialog();
        bool insertCol();
        void showWidthEdit();
        void hideWidthEdit();
        bool checkValidData();
        bool getInsertOK() const;
        QString getName();
        int getType();
        int getWidth();
};

#endif // NEWCOLDIALOG_H
