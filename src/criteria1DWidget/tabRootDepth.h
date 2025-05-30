#ifndef TABROOTDEPTH_H
#define TABROOTDEPTH_H

    #include <QtWidgets>
    #include <QtCharts>

    #include "callout.h"
    #ifndef SOIL_H
        #include "soil.h"
    #endif
    #ifndef CROP_H
        #include "crop.h"
    #endif
    #ifndef METEOPOINT_H
        #include "meteoPoint.h"
    #endif

    class TabRootDepth : public QWidget
    {
        Q_OBJECT
    public:
        TabRootDepth();
        void computeRootDepth(Crit3DCrop &myCrop, const Crit3DMeteoPoint &meteoPoint, int firstYear, int lastYear,
                              const QDate &lastSimulationDate, const std::vector<soil::Crit1DLayer> &soilLayers);

        void tooltipRDM(QPointF point, bool state);
        void tooltipRD(QPointF point, bool state);
        Callout *m_tooltip;

    private:
        QChartView *chartView;
        QChart *chart;
        QLineSeries *seriesRootDepth;
        QLineSeries *seriesRootDepthMin;
        QDateTimeAxis *axisX;
        QValueAxis *axisY;
    };

#endif // TABROOTDEPTH_H
