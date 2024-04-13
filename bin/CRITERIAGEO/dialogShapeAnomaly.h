#ifndef DIALOGSHAPEANOMALY_H
#define DIALOGSHAPEANOMALY_H

    #include <QtWidgets>

    #ifndef SHAPEHANDLER_H
        #include "shapeHandler.h"
    #endif

    class DialogShapeAnomaly : public QDialog
    {
        Q_OBJECT

        private:
            std::vector<Crit3DShapeHandler*> shapeObjList;
            QListWidget* shape1ListWidget;
            QListWidget* shape2ListWidget;
            QListWidget* id1ListWidget;
            QListWidget* id2ListWidget;
            QListWidget* field1ListWidget;
            QListWidget* field2ListWidget;

            Crit3DShapeHandler *shapeClimate;
            Crit3DShapeHandler *shapeForecast;

            QString idClimate;
            QString idForecast;
            QString fieldClimate;
            QString fieldForecast;

            void shape1Clicked(QListWidgetItem* item);
            void shape2Clicked(QListWidgetItem* item);
            void computeAnomaly();

        public:
            DialogShapeAnomaly(std::vector<Crit3DShapeHandler*> shapeObjList);

            QString getIdClimate() const { return idClimate; }
            QString getIdForecast() const { return idForecast; }

            QString getFieldClimate() const { return fieldClimate; }
            QString getFieldForecast() const { return fieldForecast; }

            Crit3DShapeHandler *getClimateShape() const { return shapeClimate; }
            Crit3DShapeHandler *getForecastShape() const { return shapeForecast; }
    };


#endif // DIALOGSHAPEANOMALY_H
