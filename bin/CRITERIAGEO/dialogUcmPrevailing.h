#ifndef DIALOGUCMPREVAILING_H
#define DIALOGUCMPREVAILING_H

    #include <QtWidgets>
    #ifndef SHAPEHANDLER_H
        #include "shapeHandler.h"
    #endif

    class DialogUcmPrevailing : public QDialog
    {
        Q_OBJECT

        private:
            std::vector<Crit3DShapeHandler*> shapeObjList;
            QListWidget* cropShape;
            QListWidget* soilShape;
            QListWidget* meteoShape;
            QListWidget* cropField;
            QListWidget* soilField;
            QListWidget* meteoField;
            QLineEdit* cellSize;
            Crit3DShapeHandler *crop;
            Crit3DShapeHandler *soil;
            Crit3DShapeHandler *meteo;
            QString idCrop;
            QString idSoil;
            QString idMeteo;
        public:
            DialogUcmPrevailing(std::vector<Crit3DShapeHandler*> shapeObjList);

            void shapeCropClicked(QListWidgetItem* item);
            void shapeSoilClicked(QListWidgetItem* item);
            void shapeMeteoClicked(QListWidgetItem* item);
            void ucm();
            QString getIdCrop() const;
            QString getIdSoil() const;
            QString getIdMeteo() const;
            Crit3DShapeHandler *getCrop() const;
            Crit3DShapeHandler *getSoil() const;
            Crit3DShapeHandler *getMeteo() const;
            double getCellSize() const;
    };


#endif // DIALOGUCMPREVAILING_H
