#include "tabRootDensity.h"
#include "commonConstants.h"

TabRootDensity::TabRootDensity()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QVBoxLayout *sliderLayout = new QVBoxLayout;
    QVBoxLayout *dateLayout = new QVBoxLayout;
    QVBoxLayout *plotLayout = new QVBoxLayout;

    dateLayout->setAlignment(Qt::AlignHCenter);
    currentDate = new QDateEdit;
    slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(1);
    QDate middleDate(currentDate->date().year(),06,30);
    slider->setMaximum(QDate(middleDate.year(),12,31).dayOfYear());
    slider->setValue(middleDate.dayOfYear());
    currentDate->setDate(middleDate);
    currentDate->setDisplayFormat("MMM dd");
    currentDate->setMaximumWidth(this->width()/5);
    chart = new QChart();
    chartView = new QChartView(chart);
    chartView->setChart(chart);

    seriesRootDensity = new QHorizontalBarSeries();
    seriesRootDensity->setName("Rooth density");
    set = new QBarSet("");
    seriesRootDensity->append(set);
    chart->addSeries(seriesRootDensity);

    axisX = new QValueAxis();
    axisY = new QBarCategoryAxis();

    axisX->setTitleText("Rooth density [%]");
    axisX->setRange(0,2.2);
    axisX->setTickCount(12);
    axisX->setLabelFormat("%.2f");
    chart->addAxis(axisX, Qt::AlignBottom);
    seriesRootDensity->attachAxis(axisX);

    axisY->setTitleText("Depth [m]");

    double i = 1.95;
    while (i > 0)
    {
        categories.append(QString::number(i, 'f', 2));
        i = i-0.1;
    }
    axisY->append(categories);
    chart->addAxis(axisY, Qt::AlignLeft);
    seriesRootDensity->attachAxis(axisY);

    chart->legend()->setVisible(false);
    nrLayers = 0;

    connect(currentDate, &QDateEdit::dateChanged, this, &TabRootDensity::updateRootDensity);
    connect(slider, &QSlider::valueChanged, this, &TabRootDensity::updateDate);
    connect(seriesRootDensity, &QHorizontalBarSeries::hovered, this, &TabRootDensity::tooltip);
    plotLayout->addWidget(chartView);
    sliderLayout->addWidget(slider);
    dateLayout->addWidget(currentDate);
    mainLayout->addLayout(sliderLayout);
    mainLayout->addLayout(dateLayout);
    mainLayout->addLayout(plotLayout);
    setLayout(mainLayout);
}

void TabRootDensity::computeRootDensity(Crit3DCrop* myCrop, Crit3DMeteoPoint *meteoPoint, int currentYear, const std::vector<soil::Crit3DLayer> &soilLayers)
{

    crop = myCrop;
    mp = meteoPoint;
    layers = soilLayers;
    nrLayers = unsigned(soilLayers.size());
    year = currentYear;

    QDate lastDate(year,12,31);
    slider->setMaximum(lastDate.dayOfYear());

    double totalSoilDepth = 0;
    if (nrLayers > 0) totalSoilDepth = soilLayers[nrLayers-1].depth + soilLayers[nrLayers-1].thickness / 2;

    axisY->clear();
    categories.clear();
    depthLayers.clear();
    double i = totalSoilDepth-0.05;
    while (i >= 0)
    {
        categories.append(QString::number(i, 'f', 2));
        i = i-0.1;
    }

    double n = totalSoilDepth/0.02;
    double value;
    for (int i = n; i>0; i--)
    {
        value = (i-1)*0.02 + 0.01;
        depthLayers.append(value);
    }

    axisY->append(categories);

    int currentDoy = 1;
    myCrop->initialize(meteoPoint->latitude, nrLayers, totalSoilDepth, currentDoy);
    updateRootDensity();
}

void TabRootDensity::updateDate()
{
    int doy = slider->value();
    QDate newDate = QDate(year, 1, 1).addDays(doy - 1);
    if (newDate != currentDate->date())
    {
        currentDate->setDate(newDate);
    }

}

void TabRootDensity::updateRootDensity()
{

    QDate newDate(year,currentDate->date().month(),currentDate->date().day());
    slider->blockSignals(true);
    slider->setValue(newDate.dayOfYear());
    slider->blockSignals(false);
    if (crop == nullptr || mp == nullptr || nrLayers == 0)
    {
        return;
    }
    if (set != nullptr)
    {
        seriesRootDensity->remove(set);
        chart->removeSeries(seriesRootDensity);
    }
    set = new QBarSet("");

    std::string error;
    int prevYear = year - 1;
    Crit3DDate firstDate = Crit3DDate(1, 1, prevYear);
    Crit3DDate lastDate = Crit3DDate(currentDate->date().day(), currentDate->date().month(), year);
    double waterTableDepth = NODATA;
    double tmin;
    double tmax;
    double maxRootDensity = 0;
    for (Crit3DDate myDate = firstDate; myDate <= lastDate; ++myDate)
    {
        tmin = mp->getMeteoPointValueD(myDate, dailyAirTemperatureMin);
        tmax = mp->getMeteoPointValueD(myDate, dailyAirTemperatureMax);

        if (!crop->dailyUpdate(myDate, mp->latitude, layers, tmin, tmax, waterTableDepth, error))
        {
            QMessageBox::critical(nullptr, "Error!", QString::fromStdString(error));
            return;
        }

        // display only current doy
        if (myDate == lastDate)
        {
            for (int i = 0; i<depthLayers.size(); i++)
            {
                int layerIndex;
                double rootDensity;
                double rootDensityAdj;
                if (depthLayers[i] <= 2)
                {
                    layerIndex = getSoilLayerIndex(layers, depthLayers[i]);
                    if (layerIndex != NODATA)
                    {
                        rootDensity = crop->roots.rootDensity[layerIndex]*100;
                        rootDensityAdj = rootDensity/layers[layerIndex].thickness*0.02;
                        *set << rootDensityAdj;

                        if (rootDensityAdj > maxRootDensity)
                        {
                            maxRootDensity = rootDensityAdj;
                        }
                    }
                }
            }
        }
    }

    maxRootDensity = ceil(maxRootDensity);
    axisX->setRange(0, maxRootDensity);
    seriesRootDensity->append(set);
    chart->addSeries(seriesRootDensity);
}

void TabRootDensity::tooltip(bool state, int index, QBarSet *barset)
{

    qDebug() << "index " << index;
    qDebug() << "set->at(index) " << set->at(index);
/*
    if (m_tooltip == nullptr)
        m_tooltip = new Callout(chart);

    if (state) {
        m_tooltip->setText(QString("%1 \n ").arg(set->at(index)));
        QPointF point(barset->at(index), index);
        m_tooltip->setAnchor(point);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();

    }
    else {
        m_tooltip->hide();
    }
    */
    /*
    if (m_tooltip == nullptr)
        m_tooltip = new Callout(chart);

    if (state) {

        int yindex=m_barSetList.indexOf(barset);
        double indexbarset=m_barSetList.indexOf(barset)-1;
        indexbarset=indexbarset/6;
        QBarSet *set=m_barSetList.at(yindex);
        QString yname=set->label();
        m_tooltip->setText("from:"+categories.at(index)+"\n to :"+QString("\n: %2 ").arg(set->at(index)));
        QPointF point(index+indexbarset, barset->at(index));
        qDebug()<<"index:"<<indexbarset<<index<< " "<<barset->at(index);
        m_tooltip->setAnchor(point);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();

    }
    else {
        m_tooltip->hide();
    }
        */
}
