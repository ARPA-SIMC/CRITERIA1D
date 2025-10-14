#ifndef TABDEGREEDAYS_H
#define TABDEGREEDAYS_H

    #include <QtCharts>
    #include <QWidget>
    #include "callout.h"

    class Crit1DCase;

    class TabDegreeDays : public QWidget
    {
        Q_OBJECT
    public:
        TabDegreeDays();
        void computeDegreeDays(Crit1DCase &myCase, int firstYear, int lastYear, const QDate &lastDBMeteoDate);

        void tooltipLAI(QPointF point, bool state);
        void tooltipDegreeDays(QPointF point, bool state);

        void handleMarkerClicked();

    private:
        QChartView *chartView;
        QChart *chart;
        QLineSeries *seriesLAI;
        QLineSeries *seriesDegreeDays;
        QDateTimeAxis *axisX;
        QValueAxis *axisY;
        QValueAxis *axisY_dx;
        Callout *m_tooltip;
    };

#endif // TABDEGREEDAYS_H
