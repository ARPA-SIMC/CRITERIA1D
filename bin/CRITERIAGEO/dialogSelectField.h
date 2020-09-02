#ifndef DIALOGSELECTFIELD_H
#define DIALOGSELECTFIELD_H

#include <QtWidgets>
#ifndef SHAPEHANDLER_H
    #include "shapeHandler.h"
#endif

enum dialogType {SHAPESTYLE, RASTERIZE, GDALRASTER};

    class DialogSelectField : public QDialog
    {
        Q_OBJECT

        private:
            Crit3DShapeHandler *shapeHandler;
            QListWidget* listFields;
            QLineEdit* outputName;
            QLineEdit* cellSize;

        public:
            DialogSelectField(Crit3DShapeHandler *shapeHandler, QString fileName, bool onlyNumeric, dialogType dialogType);

            void acceptRasterize();
            void acceptSelection();
            QString getOutputName();
            QString getFieldSelected();
            double getCellSize() const;
    };

#endif // DIALOGSELECTFIELDG_H
