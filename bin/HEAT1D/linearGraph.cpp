#include "linearGraph.h"
#include "commonConstants.h"


LinearGraph::LinearGraph()
{

    QHBoxLayout *mainLayout = new QHBoxLayout;
    QVBoxLayout *plotLayout = new QVBoxLayout;

    chart = new QChart();
    chartView = new QChartView();
    chartView->setChart(chart);

    axisX = new QValueAxis();
    axisX->setTitleText(QString("Time [%1]").arg(QString("hours")));
    axisX->setRange(0, 200);

    axisY = new QValueAxis();
    axisY->setRange(0, 350);

    QFont font = axisY->titleFont();
    font.setPointSize(11);
    font.setBold(true);
    axisX->setTitleFont(font);
    axisY->setTitleFont(font);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    chart->legend()->setVisible(false);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAcceptHoverEvents(true);

    chart->legend()->setVisible(false);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAcceptHoverEvents(true);

    m_tooltip = new Callout(chart);
    m_tooltip->hide();

    plotLayout->addWidget(chartView);
    mainLayout->addLayout(plotLayout);

    setLayout(mainLayout);

}

void LinearGraph::resetAll()
{
    // delete all Widgets
    if (!curveList.isEmpty())
    {
        qDeleteAll(curveList);
        curveList.clear();
    }

    myCurveNames.clear();
    chart->removeAllSeries();
    delete m_tooltip;
    m_tooltip = new Callout(chart);
    m_tooltip->hide();
    chart->legend()->setVisible(false);

}


void LinearGraph::draw(Crit3DColorScale colorScale, Crit3DOut* out, outputGroup graphType)
{

    resetAll();

    myColorScale = colorScale;
    myOut = out;
    setProperties(graphType);

    Crit3DColor* myColor;
    QColor color;
    QVector<QPointF> mySeries;
    float minGraph, maxGraph, minSeries, maxSeries;
    minGraph = maxGraph = minSeries = maxSeries = NODATA;

    for (unsigned int i = 0; i < myCurveNames.size(); i++)
    {
        mySeries.clear();
        myColor = myColorScale.getColor(i);
        color = QColor(myColor->red, myColor->green, myColor->blue);
        QLineSeries* curve = new QLineSeries();
        curve->setColor(color);
        curve->setName(myCurveNames[i]);
        mySeries = getSingleSeries(myOut, outputVar(i+startIndex), &minSeries, &maxSeries);
        for (unsigned int j = 0; j < mySeries.size(); j++)
        {
            curve->append(mySeries[j]);
        }
        minGraph = (minGraph == NODATA) ? minSeries : ((minSeries != NODATA && minSeries < minGraph) ? minSeries : minGraph);
        maxGraph = (maxGraph == NODATA) ? maxSeries : ((maxSeries != NODATA && maxSeries > maxGraph) ? maxSeries : maxGraph);
        curveList.push_back(curve);
        chart->addSeries(curve);
        curve->attachAxis(axisX);
        curve->attachAxis(axisY);
        connect(curve, &QLineSeries::hovered, this, &LinearGraph::tooltipLineSeries);
    }

    double eps = 0.0001;
    if (fabs(minGraph-maxGraph) < eps)
    {
        minGraph -= eps;
        maxGraph += eps;
    }

    axisY->setRange(minGraph, maxGraph);
    chart->legend()->setVisible(true);
}

void LinearGraph::setProperties(outputGroup graphType)
{
    if (graphType == outputGroup::energyBalance)
    {
        axisY->setTitleText(QString("Surface energy balance [%1]").arg(QString("W m-2")));

        myCurveNames.append("net irradiance");
        myCurveNames.append("sensible heat");
        myCurveNames.append("latent heat");
        startIndex = 0;

    }
    else if (graphType == outputGroup::surfaceResistances)
    {
        axisY->setTitleText(QString("Surface resistance [%1]").arg(QString("s m-1")));

        myCurveNames.append("aerodynamic resistance");
        myCurveNames.append("soil surface resistance");
        startIndex = 3;
    }
    else if (graphType == outputGroup::errorBalance)
    {
        axisY->setTitleText(QString("Error Balance [%1]").arg(QString("-")));

        myCurveNames.append("MBR heat");
        myCurveNames.append("MBR water");
        startIndex = 5;

    }
}

void LinearGraph::tooltipLineSeries(QPointF point, bool state)
{

    auto serie = qobject_cast<QLineSeries *>(sender());
    int index = curveList.indexOf(serie);
    if (state)
    {
        double xValue = point.x();
        double yValue = point.y();

        m_tooltip->setText(QString("%1 \n%2 %3 ").arg(myCurveNames[index]).arg(xValue, 0, 'f', 1).arg(yValue, 0, 'f', 1));
        m_tooltip->setSeries(serie);
        m_tooltip->setAnchor(point);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();
    }
    else
    {
        m_tooltip->hide();
    }
}

