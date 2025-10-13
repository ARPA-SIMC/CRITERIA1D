#include "tabRootDensity.h"
#include "commonConstants.h"
#include "criteria1DProject.h"
#include "meteoPoint.h"

TabRootDensity::TabRootDensity()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QVBoxLayout *sliderLayout = new QVBoxLayout;
    QHBoxLayout *dateLayout = new QHBoxLayout;
    QVBoxLayout *plotLayout = new QVBoxLayout;

    dateLayout->setAlignment(Qt::AlignHCenter);
    m_currentDateEdit = new QDateEdit;
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setMinimum(1);
    QDate middleDate(m_currentDateEdit->date().year(),06,30);
    m_slider->setMaximum(QDate(middleDate.year(),12,31).dayOfYear());
    m_slider->setValue(middleDate.dayOfYear());
    m_currentDateEdit->setDate(middleDate);
    m_currentDateEdit->setDisplayFormat("MMM dd");
    m_currentDateEdit->setMaximumWidth(this->width()/5);
    m_yearComboBox.addItem(QString::number(QDate::currentDate().year()));
    m_yearComboBox.setMaximumWidth(this->width()/5);
    chart = new QChart();
    chartView = new QChartView(chart);
    chartView->setChart(chart);

    seriesRootDensity = new QHorizontalBarSeries();
    seriesRootDensity->setName("Root density");
    set = new QBarSet("");
    seriesRootDensity->append(set);
    chart->addSeries(seriesRootDensity);

    axisX = new QValueAxis();
    axisY = new QBarCategoryAxis();

    axisX->setTitleText("Root density [%]");
    axisX->setRange(0,2.2);
    axisX->setTickCount(12);
    axisX->setLabelFormat("%.1f");
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
    m_nrLayers = 0;

    m_tooltip = new Callout(chart);
    m_tooltip->hide();

    connect(m_currentDateEdit, &QDateEdit::dateChanged, this, &TabRootDensity::updateRootDensity);
    connect(m_slider, &QSlider::valueChanged, this, &TabRootDensity::updateDate);
    connect(seriesRootDensity, &QHorizontalBarSeries::hovered, this, &TabRootDensity::tooltip);
    connect(&m_yearComboBox, &QComboBox::currentTextChanged, this, &TabRootDensity::on_actionChooseYear);
    plotLayout->addWidget(chartView);
    sliderLayout->addWidget(m_slider);
    dateLayout->addWidget(&m_yearComboBox);
    dateLayout->addWidget(m_currentDateEdit);
    mainLayout->addLayout(sliderLayout);
    mainLayout->addLayout(dateLayout);
    mainLayout->addLayout(plotLayout);
    setLayout(mainLayout);
}


void TabRootDensity::computeRootDensity(const Crit1DProject &myProject, int firstYear, int lastYear)
{
    m_meteoPoint =  myProject.myCase.meteoPoint;
    m_crop = myProject.myCase.crop;
    m_layers = myProject.myCase.soilLayers;
    m_nrLayers = unsigned(m_layers.size());
    m_lastMeteoDate = myProject.lastSimulationDate;

    m_yearComboBox.blockSignals(true);
    m_yearComboBox.clear();
    for (int i = firstYear; i<=lastYear; i++)
    {
        m_yearComboBox.addItem(QString::number(i));
    }
    m_year = m_yearComboBox.currentText().toInt();
    m_yearComboBox.blockSignals(false);

    if (m_year == myProject.lastSimulationDate.year())
    {
        m_slider->setMaximum(myProject.lastSimulationDate.dayOfYear());
        m_currentDateEdit->setDate(myProject.lastSimulationDate);
    }
    else
    {
        QDate middleDate(m_currentDateEdit->date().year(), 06, 30);
        QDate lastDate(m_year,12,31);
        m_slider->setMaximum(lastDate.dayOfYear());
        m_currentDateEdit->setDate(middleDate);
    }

    double totalSoilDepth = 0;
    if (m_nrLayers > 0) totalSoilDepth = myProject.myCase.soilLayers[m_nrLayers-1].depth
                                        + myProject.myCase.soilLayers[m_nrLayers-1].thickness / 2;

    axisY->clear();
    categories.clear();
    m_depthLayers.clear();
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
        m_depthLayers.append(value);
    }

    axisY->append(categories);

    int currentDoy = 1;
    m_crop.initialize(myProject.myCase.meteoPoint.latitude, m_nrLayers, totalSoilDepth, currentDoy);

    updateRootDensity();
}


void TabRootDensity::on_actionChooseYear(const QString &myYear)
{
    m_year = myYear.toInt();
    if (m_year == m_lastMeteoDate.year())
    {
        m_slider->setMaximum(m_lastMeteoDate.dayOfYear());
        m_currentDateEdit->setDate(m_lastMeteoDate);
    }
    else
    {
        QDate middleDate(m_currentDateEdit->date().year(),06,30);
        QDate lastDate(m_year,12,31);
        m_slider->setMaximum(lastDate.dayOfYear());
        m_currentDateEdit->setDate(middleDate);
    }

    updateRootDensity();
}


void TabRootDensity::updateDate()
{
    int doy = m_slider->value();
    QDate newDate = QDate(m_year, 1, 1).addDays(doy - 1);
    if (newDate != m_currentDateEdit->date())
    {
        m_currentDateEdit->setDate(newDate);
    }
}


void TabRootDensity::updateRootDensity()
{
    QDate newDate(m_year,m_currentDateEdit->date().month(),m_currentDateEdit->date().day());
    if (newDate > m_lastMeteoDate)
    {
        m_currentDateEdit->setDate(m_lastMeteoDate);
        return;
    }
    m_slider->blockSignals(true);
    m_slider->setValue(newDate.dayOfYear());
    m_slider->blockSignals(false);

    if (m_nrLayers == 0) return;

    if (set != nullptr)
    {
        seriesRootDensity->remove(set);
        chart->removeSeries(seriesRootDensity);
    }
    set = new QBarSet("");

    std::string errorStr;
    int prevYear = m_year - 1;
    Crit3DDate firstDate = Crit3DDate(1, 1, prevYear);
    Crit3DDate lastDate = Crit3DDate(m_currentDateEdit->date().day(), m_currentDateEdit->date().month(), m_year);

    double tmin, tmax, waterTableDepth;
    double maxRootDensity = 0;

    for (Crit3DDate myDate = firstDate; myDate <= lastDate; ++myDate)
    {
        tmin = m_meteoPoint.getMeteoPointValueD(myDate, dailyAirTemperatureMin);
        tmax = m_meteoPoint.getMeteoPointValueD(myDate, dailyAirTemperatureMax);
        waterTableDepth = m_meteoPoint.getMeteoPointValueD(myDate, dailyWaterTableDepth);

        if (!m_crop.dailyUpdate(myDate, m_meteoPoint.latitude, m_layers, tmin, tmax, waterTableDepth, errorStr))
        {
            QMessageBox::critical(nullptr, "Error!", QString::fromStdString(errorStr));
            return;
        }

        // display only current doy
        if (myDate == lastDate)
        {
            for (int i = 0; i < m_depthLayers.size(); i++)
            {
                int layerIndex;
                double rootDensity;
                double rootDensityAdj;
                if (m_depthLayers[i] <= 2)
                {
                    layerIndex = getSoilLayerIndex(m_layers, m_depthLayers[i]);
                    if (layerIndex != NODATA)
                    {
                        rootDensity = m_crop.roots.rootDensity[layerIndex]*100;
                        rootDensityAdj = rootDensity / m_layers[layerIndex].thickness*0.02;
                        *set << rootDensity;

                        maxRootDensity = std::max(maxRootDensity, rootDensity);
                    }
                }
            }
        }
    }

    axisX->setRange(0, maxRootDensity);
    seriesRootDensity->append(set);
    chart->addSeries(seriesRootDensity);
}


void TabRootDensity::tooltip(bool state, int index, QBarSet *barset)
{
    if (state && barset!=nullptr && index < barset->count())
    {
        QString valueStr = QString::number(barset->at(index), 'f', 2);
        m_tooltip->setText(valueStr + " %");

        QPoint point = QCursor::pos();
        QPoint mapPoint = chartView->mapFromGlobal(point);
        QPointF pointF = chart->mapToValue(mapPoint,seriesRootDensity);

        m_tooltip->setAnchor(pointF);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();
    }
    else
    {
        m_tooltip->hide();
    }
}
