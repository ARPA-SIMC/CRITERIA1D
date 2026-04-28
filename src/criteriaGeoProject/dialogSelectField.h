#ifndef DIALOGSELECTFIELD_H
#define DIALOGSELECTFIELD_H

#include "commonConstants.h"
#include <QtWidgets>

#ifndef SHAPEHANDLER_H
    #include "shapeHandler.h"
#endif

    enum dialogType {SHAPESTYLE, RASTERIZE, RASTERIZE_WITHBASE, GDALRASTER, PREVAILING};

    class DialogSelectField : public QDialog
    {
        Q_OBJECT

        private:
            Crit3DShapeHandler* _shapeHandler;
            QListWidget* _listFields;
            QLineEdit* _stringValue;
            QLineEdit* _cellSize;

        public:
            DialogSelectField(Crit3DShapeHandler *shapeHandler, QString fileName, bool isOnlyNumeric, dialogType dialogType);

            void acceptRasterize();
            void acceptSelection();
            void acceptPrevailing();

            QString getStringValue() const
            {
                return _stringValue->text();
            }

            QString getFieldSelected() const
            {
                return _listFields->currentItem()->text();
            }

            double getCellSize() const
            {
                bool isOk;
                double cellSize = _cellSize->text().replace(",", ".").toDouble(&isOk);
                return (isOk? cellSize: NODATA);
            }
    };

#endif // DIALOGSELECTFIELDG_H
