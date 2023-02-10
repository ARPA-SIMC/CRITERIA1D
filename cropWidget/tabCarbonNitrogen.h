
#ifndef TABCARBONNITROGEN_H
#define TABCARBONNITROGEN_H

    #include <QtWidgets>
    #include "criteria1DCase.h"
    #include "carbonNitrogenModel.h"
    #include "qcustomplot.h"

    class TabCarbonNitrogen : public QWidget
    {
        Q_OBJECT
    public:
        TabCarbonNitrogen();

        void computeCarbonNitrogen(Crit1DCase myCase, Crit1DCarbonNitrogenProfile myCarbonNitrogen, int firstYear, int lastYear, QDate lastDBMeteoDate);

    private:
        QString title;
        QCustomPlot *graphic;
        QCPColorMap *colorMap;
        QCPColorScale *colorScale;
        QCPColorGradient gradient;
        int nx;
        int ny;
    };

#endif // TABCARBONNITROGEN_H
