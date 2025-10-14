#include <QMessageBox>
#include <QLegendMarker>
#include <algorithm>

#include "commonConstants.h"
#include "tabDegreeDays.h"
#include "formInfo.h"
#include "criteria1DCase.h"


TabDegreeDays::TabDegreeDays()
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    QVBoxLayout *plotLayout = new QVBoxLayout;
    chart = new QChart();
    chartView = new QChartView(chart);
    chartView->setChart(chart);

    seriesLAI = new QLineSeries();
    seriesDegreeDays = new QLineSeries();

    seriesLAI->setName("Leaf Area Index [m2 m-2] ");
    seriesDegreeDays->setName("Degree days sum [°] ");

    QPen pen;
    pen.setWidth(2);
    seriesLAI->setPen(pen);

    seriesLAI->setColor(QColor(0, 200, 0, 255));
    seriesDegreeDays->setColor(QColor(Qt::red));

    axisX = new QDateTimeAxis();
    axisY = new QValueAxis();
    axisY_dx = new QValueAxis();

    chart->addSeries(seriesLAI);
    chart->addSeries(seriesDegreeDays);

    QDate first(QDate::currentDate().year(), 1, 1);
    QDate last(QDate::currentDate().year(), 12, 31);
    axisX->setTitleText("Date");
    axisX->setFormat("MMM dd <br> yyyy");
    axisX->setMin(QDateTime(first, QTime(0,0,0)));
    axisX->setMax(QDateTime(last, QTime(0,0,0)));
    axisX->setTickCount(13);
    chart->addAxis(axisX, Qt::AlignBottom);
    seriesLAI->attachAxis(axisX);
    seriesDegreeDays->attachAxis(axisX);

    QFont font = axisX->titleFont();

    qreal maximum = 8;
    axisY->setTitleText("Leaf Area Index [m2 m-2]");
    axisY->setTitleFont(font);
    axisY->setRange(0, maximum);
    axisY->setTickCount(maximum+1);

    axisY_dx->setTitleText("Degree days sum [°]");
    axisY_dx->setTitleFont(font);
    axisY_dx->setRange(0, maximum*100);
    axisY_dx->setTickCount(maximum+1);

    chart->addAxis(axisY, Qt::AlignLeft);
    chart->addAxis(axisY_dx, Qt::AlignRight);
    seriesLAI->attachAxis(axisY);
    seriesDegreeDays->attachAxis(axisY_dx);

    chart->legend()->setVisible(true);
    QFont legendFont = chart->legend()->font();
    legendFont.setPointSize(8);
    legendFont.setBold(true);
    chart->legend()->setFont(legendFont);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAcceptHoverEvents(true);

    m_tooltip = new Callout(chart);
    m_tooltip->hide();

    connect(seriesLAI, &QLineSeries::hovered, this, &TabDegreeDays::tooltipLAI);
    connect(seriesDegreeDays, &QLineSeries::hovered, this, &TabDegreeDays::tooltipDegreeDays);

    foreach(QLegendMarker* marker, chart->legend()->markers())
    {
        QObject::connect(marker, &QLegendMarker::clicked, this, &TabDegreeDays::handleMarkerClicked);
    }

    plotLayout->addWidget(chartView);
    mainLayout->addLayout(plotLayout);
    setLayout(mainLayout);
}


void TabDegreeDays::computeDegreeDays(Crit1DCase &myCase, int firstYear, int lastYear, const QDate &lastDBMeteoDate)
{
    unsigned int nrLayers = unsigned(myCase.soilLayers.size());
    double totalSoilDepth = 0;
    if (nrLayers > 0) totalSoilDepth = myCase.soilLayers[nrLayers-1].depth + myCase.soilLayers[nrLayers-1].thickness / 2;

    int prevYear = firstYear - 1;

    Crit3DDate firstDate = Crit3DDate(1, 1, prevYear);
    Crit3DDate lastDate;
    if (lastYear != lastDBMeteoDate.year())
    {
        lastDate = Crit3DDate(31, 12, lastYear);
    }
    else
    {
        lastDate = Crit3DDate(lastDBMeteoDate.day(), lastDBMeteoDate.month(), lastYear);
    }

    double tmin, tmax, waterTableDepth;
    QDateTime x;
    std::string errorStr;

    chart->removeSeries(seriesLAI);
    chart->removeSeries(seriesDegreeDays);

    seriesLAI->clear();
    seriesDegreeDays->clear();

    int currentDoy = 1;
    myCase.crop.initialize(myCase.meteoPoint.latitude, nrLayers, totalSoilDepth, currentDoy);
    double maximumDD = 0;

    // cycle on days
    for (Crit3DDate myDate = firstDate; myDate <= lastDate; ++myDate)
    {
        tmin = myCase.meteoPoint.getMeteoPointValueD(myDate, dailyAirTemperatureMin);
        tmax = myCase.meteoPoint.getMeteoPointValueD(myDate, dailyAirTemperatureMax);
        waterTableDepth = myCase.meteoPoint.getMeteoPointValueD(myDate, dailyWaterTableDepth);

        if (! myCase.crop.dailyUpdate(myDate, myCase.meteoPoint.latitude, myCase.soilLayers, tmin, tmax, waterTableDepth, errorStr))
        {
            QMessageBox::critical(nullptr, "Error!", QString::fromStdString(errorStr));
            return;
        }

        // display from firstYear
        if (myDate.year >= firstYear)
        {
            x.setDate(QDate(myDate.year, myDate.month, myDate.day));

            seriesLAI->append(x.toMSecsSinceEpoch(), myCase.crop.LAI);
            double currentDegreeDays = 0;

            if (myCase.crop.degreeDays != NODATA)
            {
                currentDegreeDays = myCase.crop.degreeDays;
                maximumDD = std::max(maximumDD, currentDegreeDays);
            }
            seriesDegreeDays->append(x.toMSecsSinceEpoch(), currentDegreeDays);
        }
    }

    // update x axis
    QDate first(firstYear, 1, 1);
    QDate last(lastDate.year, lastDate.month, lastDate.day);
    axisX->setMin(QDateTime(first, QTime(0,0,0)));
    axisX->setMax(QDateTime(last, QTime(0,0,0)));

    chart->addSeries(seriesLAI);
    chart->addSeries(seriesDegreeDays);

    seriesLAI->attachAxis(axisY);
    float maximum_sx = axisY->max();
    int step_dx = int(maximumDD / (maximum_sx * 100)) + 1;
    float maximum_dx = step_dx * maximum_sx * 100;
    axisY_dx->setMax(maximum_dx);
    seriesDegreeDays->attachAxis(axisY_dx);

    foreach(QLegendMarker* marker, chart->legend()->markers())
    {
        QObject::connect(marker, &QLegendMarker::clicked, this, &TabDegreeDays::handleMarkerClicked);
    }
}


void TabDegreeDays::tooltipLAI(QPointF point, bool state)
{
    if (state)
    {
        QDateTime xDate;
        xDate.setMSecsSinceEpoch(point.x());
        m_tooltip->setText(QString("%1\nLAI: %2 [m2 m-2]").arg(xDate.date().toString("yyyy-MM-dd")).arg(point.y(), 0, 'f', 1));
        m_tooltip->setAnchor(point);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();
    } else {
        m_tooltip->hide();
    }
}


void TabDegreeDays::tooltipDegreeDays(QPointF point, bool state)
{
    if (state)
    {
        QDateTime xDate;
        xDate.setMSecsSinceEpoch(point.x());
        m_tooltip->setText(QString("%1\nDegree days: %2 ").arg(xDate.date().toString("yyyy-MM-dd")).arg(point.y(), 0, 'd'));
        m_tooltip->setAnchor(point);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();
    } else {
        m_tooltip->hide();
    }
}


void TabDegreeDays::handleMarkerClicked()
{
    QLegendMarker* marker = qobject_cast<QLegendMarker*> (sender());

    if(marker->type() != QLegendMarker::LegendMarkerTypeXY)
        return;

    // Toggle visibility of series
    marker->series()->setVisible(! marker->series()->isVisible());

    // Turn legend marker back to visible, since otherwise hiding series also hides the marker
    marker->setVisible(true);

    // change marker alpha, if series is not visible
    qreal alpha = 1.0;

    if (! marker->series()->isVisible()) {
        alpha = 0.5;
    }

    QColor color;
    QBrush brush = marker->labelBrush();
    color = brush.color();
    color.setAlphaF(alpha);
    brush.setColor(color);
    marker->setLabelBrush(brush);

    brush = marker->brush();
    color = brush.color();
    color.setAlphaF(alpha);
    brush.setColor(color);
    marker->setBrush(brush);

    QPen pen = marker->pen();
    color = pen.color();
    color.setAlphaF(alpha);
    pen.setColor(color);
    marker->setPen(pen);
}
