#ifndef LINEARGRAPH_H
#define LINEARGRAPH_H

    #include <QtWidgets>
    #include <QtCharts>
    #include "callout.h"
    #include "color.h"
    #include "graphFunctions.h"

    class LinearGraph: public QWidget
    {
        Q_OBJECT
    public:
        LinearGraph();
        void draw(Crit3DColorScale colorScale, Crit3DOut* out, outputGroup graphType);
        void setProperties(outputGroup graphType);
        void resetAll();
        void tooltipLineSeries(QPointF point, bool state);

    private:
        QChartView *chartView;
        QChart *chart;
        QList<QLineSeries*> curveList;
        QValueAxis *axisX;
        QValueAxis *axisY;
        Callout *m_tooltip;

        QStringList myCurveNames;
        Crit3DColorScale myColorScale;
        Crit3DOut* myOut;
        int startIndex;
    };

#endif // LINEARCECURVE_H
