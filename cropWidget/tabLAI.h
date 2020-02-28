#ifndef TABLAI_H
#define TABLAI_H

    #include <QtWidgets>
#ifndef METEOPOINT_H
    #include "meteoPoint.h"
#endif
#ifndef CROP_H
    #include "crop.h"
#endif
#include <QtCharts>
#include "crit3DChartView.h"
#include "callout.h"

    class TabLAI : public QWidget
    {
        Q_OBJECT
    public:
        TabLAI();
        void computeLAI(Crit3DCrop* myCrop, Crit3DMeteoPoint *meteoPoint, int currentYear, const std::vector<soil::Crit3DLayer>& soilLayers);
        void tooltipLAI(QPointF point, bool state);
        void tooltipPE(QPointF point, bool state);
        void tooltipME(QPointF point, bool state);
        void tooltipMT(QPointF point, bool state);
    private:
        int year;
        Crit3DChartView *chartView;
        QChart *chart;
        QLineSeries *seriesLAI;
        QLineSeries *seriesPotentialEvap;
        QLineSeries *seriesMaxEvap;
        QLineSeries *seriesMaxTransp;
        QDateTimeAxis *axisX;
        QValueAxis *axisY;
        QValueAxis *axisYdx;
        Callout *m_tooltip;
    };

#endif // TABLAI_H
