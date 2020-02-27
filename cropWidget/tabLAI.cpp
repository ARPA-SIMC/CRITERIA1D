#include "tabLAI.h"
#include "commonConstants.h"
#include <QMessageBox>


TabLAI::TabLAI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    QVBoxLayout *plotLayout = new QVBoxLayout;
    chart = new QChart();   
    chartView = new Crit3DChartView(chart);
    chart->setTitle("LAI development");
    chartView->setChart(chart);
    series = new QLineSeries();
    axisX = new QDateTimeAxis();
    axisY = new QValueAxis();

    chart->addSeries(series);
    QDate first(QDate::currentDate().year(), 1, 1);
    QDate last(QDate::currentDate().year(), 12, 31);
    axisX->setTitleText("Date");
    axisX->setFormat("MMM dd");
    axisX->setMin(QDateTime(first, QTime(0,0,0)));
    axisX->setMax(QDateTime(last, QTime(0,0,0)));
    axisX->setTickCount(13);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY->setTitleText("LAI  [m2 m-2]");
    axisY->setRange(0,6);
    axisY->setTickCount(7);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->setAcceptHoverEvents(true);
    m_tooltip = new Callout(chart);
    connect(series, &QLineSeries::hovered, this, &TabLAI::tooltip);

    plotLayout->addWidget(chartView);
    mainLayout->addLayout(plotLayout);
    setLayout(mainLayout);
}

void TabLAI::computeLAI(Crit3DCrop* myCrop, Crit3DMeteoPoint *meteoPoint, int currentYear, const std::vector<soil::Crit3DLayer> &soilLayers)
{
    unsigned int nrLayers = unsigned(soilLayers.size());
    double totalSoilDepth = 0;
    if (nrLayers > 0) totalSoilDepth = soilLayers[nrLayers-1].depth + soilLayers[nrLayers-1].thickness / 2;

    year = currentYear;
    int prevYear = currentYear - 1;

    double waterTableDepth = NODATA;
    std::string error;

    Crit3DDate firstDate = Crit3DDate(1, 1, prevYear);
    Crit3DDate lastDate = Crit3DDate(31, 12, year);
    double tmin;
    double tmax;
    QDateTime x;

    series->clear();

    int currentDoy = 1;
    myCrop->initialize(meteoPoint->latitude, nrLayers, totalSoilDepth, currentDoy);

    for (Crit3DDate myDate = firstDate; myDate <= lastDate; ++myDate)
    {
        tmin = meteoPoint->getMeteoPointValueD(myDate, dailyAirTemperatureMin);
        tmax = meteoPoint->getMeteoPointValueD(myDate, dailyAirTemperatureMax);

        if (!myCrop->dailyUpdate(myDate, meteoPoint->latitude, soilLayers, tmin, tmax, waterTableDepth, &error))
        {
            QMessageBox::critical(nullptr, "Error!", QString::fromStdString(error));
            return;
        }

        // display only current year
        if (myDate.year == year)
        {
            x.setDate(QDate(myDate.year, myDate.month, myDate.day));
            series->append(x.toMSecsSinceEpoch(), myCrop->LAI);
        }
    }

    // update x axis
    QDate first(year, 1, 1);
    QDate last(year, 12, 31);
    axisX->setMin(QDateTime(first, QTime(0,0,0)));
    axisX->setMax(QDateTime(last, QTime(0,0,0)));
}

void TabLAI::tooltip(QPointF point, bool state)
{
    if (m_tooltip == nullptr)
        m_tooltip = new Callout(chart);

    if (state)
    {
        QDateTime xDate;
        xDate.setMSecsSinceEpoch(point.x());
        m_tooltip->setText(QString("X: %1 \nY: %3 ").arg(xDate.date().toString("MMM dd")).arg(point.y()));
        m_tooltip->setAnchor(point);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();
    } else {
        m_tooltip->hide();
    }
}


