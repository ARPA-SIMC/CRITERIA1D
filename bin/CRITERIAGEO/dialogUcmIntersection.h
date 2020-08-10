#ifndef DIALOGUCMINTERSECTION_H
#define DIALOGUCMINTERSECTION_H

    #include <QtWidgets>
    #include "shapeHandler.h"

    class DialogUcmIntersection : public QDialog
    {
        Q_OBJECT

        private:
            std::vector<Crit3DShapeHandler*> shapeObjList;
            QGridLayout* cropFieldLayout;
            QGridLayout* soilFieldLayout;
            QGridLayout* meteoFieldLayout;
            QListWidget* cropShape;
            QListWidget* soilShape;
            QListWidget* meteoShape;
            QListWidget* cropField;
            QListWidget* soilField;
            QListWidget* meteoField;
            QLineEdit* idCropEdit;
            QLineEdit* idSoilEdit;
            QLineEdit* idMeteoEdit;
            Crit3DShapeHandler *crop;
            Crit3DShapeHandler *soil;
            Crit3DShapeHandler *meteo;
            QString idCrop;
            QString idSoil;
            QString idMeteo;
        public:
            DialogUcmIntersection(std::vector<Crit3DShapeHandler*> shapeObjList);

            void shapeCropClicked(QListWidgetItem* item);
            void shapeSoilClicked(QListWidgetItem* item);
            void shapeMeteoClicked(QListWidgetItem* item);
            void idCropEditing(QString newText);
            void idSoilEditing(QString newText);
            void idMeteoEditing(QString newText);
            void ucm();
            QString getIdCrop() const;
            QString getIdSoil() const;
            QString getIdMeteo() const;
            Crit3DShapeHandler *getCrop() const;
            Crit3DShapeHandler *getSoil() const;
            Crit3DShapeHandler *getMeteo() const;
            void showChildren(const QGridLayout* layout, bool show);
    };


#endif // DIALOGUCMINTERSECTION_H
