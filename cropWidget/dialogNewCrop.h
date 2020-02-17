#ifndef DIALOGNEWCROP_H
#define DIALOGNEWCROP_H

#include "crop.h"
#include <QtWidgets>

    class DialogNewCrop : public QDialog
    {
        Q_OBJECT
    public:
        DialogNewCrop(Crit3DCrop* newCrop);
        void on_actionChooseType(QString type);

    private:
        Crit3DCrop* newCrop;
        QLineEdit* idCropValue;
        QLineEdit* nameCropValue;
        QLineEdit* typeCropValue;
        QLabel *sowingDoY;
        QLineEdit* sowingDoYValue;
        QLabel *cycleMaxDuration;
        QLineEdit* cycleMaxDurationValue;
    };

#endif // DIALOGNEWCROP_H
