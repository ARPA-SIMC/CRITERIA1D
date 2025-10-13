#ifndef TABROOTDEPTH_H
#define TABROOTDEPTH_H

    #include <QtWidgets>
    #include <QtCharts>

    #include "callout.h"
    class Crit1DCase;

    class TabRootDepth : public QWidget
    {
        Q_OBJECT
    public:
        TabRootDepth();
        void computeRootDepth(Crit1DCase &myCase, int firstYear, int lastYear, const QDate &lastDBMeteoDate);

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
