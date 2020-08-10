#include "colorMapGraph.h"
#include "commonConstants.h"


ColorMapGraph::ColorMapGraph()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QVBoxLayout *plotLayout = new QVBoxLayout;
    graphic = new QCustomPlot();

    // configure axis rect:
    graphic->axisRect()->setupFullAxesBox(true);
    graphic->xAxis->setLabel(QString("Time [%1]").arg(QString("hours")));

    // set up the QCPColorMap:
    colorMap = new QCPColorMap(graphic->xAxis2, graphic->yAxis);
    nx = 200;
    ny = 20;
    colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points

    // add a color scale:
    colorScale = new QCPColorScale(graphic);
    colorScale->setDataRange(QCPRange(0,1));
    graphic->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale

    // set the color gradient of the color map to one of the presets:
    //colorMap->setGradient(QCPColorGradient::gpPolar);
    // create a QCPColorGradient instance and added own colors to the gradient
    gradient.clearColorStops();
    gradient.setColorStopAt(0, QColor(128, 0, 128));
    gradient.setColorStopAt(0.25, QColor(255, 0, 0));
    gradient.setColorStopAt(0.5, QColor(255, 255, 0));
    gradient.setColorStopAt(0.75, QColor(64, 196, 64));
    gradient.setColorStopAt(1, QColor(0, 0, 255));
    colorMap->setGradient(gradient);

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
    colorMap->rescaleDataRange();

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(graphic);
    graphic->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    // rescale the key (x) and value (y) axes so the whole color map is visible:
    graphic->rescaleAxes();

    plotLayout->addWidget(graphic);
    mainLayout->addLayout(plotLayout);
    setLayout(mainLayout);
}


void ColorMapGraph::draw(Crit3DOut* out, outputGroup graphType)
{

    myOut = out;
    setProperties(graphType);

    QVector<QPointF> mySeries;
    float minGraph, maxGraph, minSeries, maxSeries;
    minGraph = maxGraph = minSeries = maxSeries = NODATA;

    float maxDepth = myOut->layerThickness * ((float)(ny-1) + 0.5);
    colorMap->data()->setSize(nx, ny);
    colorMap->data()->setRange(QCPRange(0, nx), QCPRange(maxDepth,0));
    graphic->xAxis->setRange(0, nx);

    for (unsigned int z = 1; z <= ny; z++)
    {
        mySeries.clear();
        mySeries = getProfileSeries(myOut, graphType, z, &minSeries, &maxSeries);
        for (unsigned int j = 0; j < mySeries.size(); j++)
        {
            colorMap->data()->setCell(mySeries[j].x(), z-1, mySeries[j].y());
        }
        minGraph = (minGraph == NODATA) ? minSeries : ((minSeries != NODATA && minSeries < minGraph) ? minSeries : minGraph);
        maxGraph = (maxGraph == NODATA) ? maxSeries : ((maxSeries != NODATA && maxSeries > maxGraph) ? maxSeries : maxGraph);
    }

    double eps = 0.0001;
    if (fabs(minGraph-maxGraph) < eps)
    {
        minGraph -= eps;
        maxGraph += eps;
    }

    colorScale->setDataRange(QCPRange(minGraph, maxGraph));
    graphic->rescaleAxes();
    graphic->replot();
}

void ColorMapGraph::setProperties(outputGroup graphType)
{

    nx = myOut->nrValues;
    ny = myOut->nrLayers;

    graphic->yAxis->setLabel("Depth [m]");
    graphic->yAxis->setRangeReversed(true);

    if (graphType == outputGroup::soilTemperature)
    {
        gradient.clearColorStops();
        gradient.setColorStopAt(0, QColor(0, 0, 255));
        gradient.setColorStopAt(0.25, QColor(64, 196, 64));
        gradient.setColorStopAt(0.5, QColor(255, 255, 0));
        gradient.setColorStopAt(0.75, QColor(255, 0, 0));
        gradient.setColorStopAt(1, QColor(128, 0, 128));
        colorScale->axis()->setLabel(QString("Soil temperature [%1]").arg(QString("degC")));
    }
    else if (graphType == outputGroup::soilWater)
    {

        gradient.clearColorStops();
        gradient.setColorStopAt(0, QColor(128, 0, 128));
        gradient.setColorStopAt(0.25, QColor(255, 0, 0));
        gradient.setColorStopAt(0.5, QColor(255, 255, 0));
        gradient.setColorStopAt(0.75, QColor(64, 196, 64));
        gradient.setColorStopAt(1, QColor(0, 0, 255));
        colorScale->axis()->setLabel(QString("Soil water content [%1]").arg(QString("m3 m-3")));
    }
    else if (graphType == soilHeatConductivity)
    {
        gradient.clearColorStops();
        gradient.setColorStopAt(0, QColor(0, 0, 255));
        gradient.setColorStopAt(0.25, QColor(64, 196, 64));
        gradient.setColorStopAt(0.5, QColor(255, 255, 0));
        gradient.setColorStopAt(0.75, QColor(255, 0, 0));
        gradient.setColorStopAt(1, QColor(128, 0, 128));
        colorScale->axis()->setLabel(QString("Heat conductivity [%1]").arg(QString("W m-1 K-1")));
    }
    else if (graphType == totalHeatFlux)
    {
        gradient.clearColorStops();
        gradient.setColorStopAt(0, QColor(0, 0, 255));
        gradient.setColorStopAt(0.25, QColor(64, 196, 64));
        gradient.setColorStopAt(0.5, QColor(255, 255, 0));
        gradient.setColorStopAt(0.75, QColor(255, 0, 0));
        gradient.setColorStopAt(1, QColor(128, 0, 128));
        colorScale->axis()->setLabel(QString("Total heat flux [%1]").arg(QString("W m-2")));
    }
    else if (graphType == latentHeatFluxIso)
    {
        gradient.clearColorStops();
        gradient.setColorStopAt(0, QColor(0, 0, 255));
        gradient.setColorStopAt(0.25, QColor(64, 196, 64));
        gradient.setColorStopAt(0.5, QColor(255, 255, 0));
        gradient.setColorStopAt(0.75, QColor(255, 0, 0));
        gradient.setColorStopAt(1, QColor(128, 0, 128));
        colorScale->axis()->setLabel(QString("Isotermal latent heat flux [%1]").arg(QString("W m-2")));
    }
    else if (graphType == latentHeatFluxTherm)
    {
        gradient.clearColorStops();
        gradient.setColorStopAt(0, QColor(0, 0, 255));
        gradient.setColorStopAt(0.25, QColor(64, 196, 64));
        gradient.setColorStopAt(0.5, QColor(255, 255, 0));
        gradient.setColorStopAt(0.75, QColor(255, 0, 0));
        gradient.setColorStopAt(1, QColor(128, 0, 128));
        colorScale->axis()->setLabel(QString("Thermal latent heat flux [%1]").arg(QString("W m-2")));
    }
    else if (graphType == diffusiveHeatFlux)
    {
        gradient.clearColorStops();
        gradient.setColorStopAt(0, QColor(0, 0, 255));
        gradient.setColorStopAt(0.25, QColor(64, 196, 64));
        gradient.setColorStopAt(0.5, QColor(255, 255, 0));
        gradient.setColorStopAt(0.75, QColor(255, 0, 0));
        gradient.setColorStopAt(1, QColor(128, 0, 128));
        colorScale->axis()->setLabel(QString("Diffuse heat flux [%1]").arg(QString("W m-2")));
    }
    else if (graphType == waterIsothLiquidFlux)
    {
        gradient.clearColorStops();
        gradient.setColorStopAt(0, QColor(0, 0, 255));
        gradient.setColorStopAt(0.25, QColor(64, 196, 64));
        gradient.setColorStopAt(0.5, QColor(255, 255, 0));
        gradient.setColorStopAt(0.75, QColor(255, 0, 0));
        gradient.setColorStopAt(1, QColor(128, 0, 128));
        colorScale->axis()->setLabel(QString("Isothermal water liquid flux [%1]").arg(QString("l s-1")));
    }
    else if (graphType == waterThermLiquidFlux)
    {
        gradient.clearColorStops();
        gradient.setColorStopAt(0, QColor(0, 0, 255));
        gradient.setColorStopAt(0.25, QColor(64, 196, 64));
        gradient.setColorStopAt(0.5, QColor(255, 255, 0));
        gradient.setColorStopAt(0.75, QColor(255, 0, 0));
        gradient.setColorStopAt(1, QColor(128, 0, 128));
        colorScale->axis()->setLabel(QString("Thermal water liquid flux [%1]").arg(QString("l s-1")));
    }
    else if (graphType == waterIsothVaporFlux)
    {
        gradient.clearColorStops();
        gradient.setColorStopAt(0, QColor(0, 0, 255));
        gradient.setColorStopAt(0.25, QColor(64, 196, 64));
        gradient.setColorStopAt(0.5, QColor(255, 255, 0));
        gradient.setColorStopAt(0.75, QColor(255, 0, 0));
        gradient.setColorStopAt(1, QColor(128, 0, 128));
        colorScale->axis()->setLabel(QString("Isothermal water vapor flux [%1]").arg(QString("l s-1")));
    }
    else if (graphType == waterThermVaporFlux)
    {
        gradient.clearColorStops();
        gradient.setColorStopAt(0, QColor(0, 0, 255));
        gradient.setColorStopAt(0.25, QColor(64, 196, 64));
        gradient.setColorStopAt(0.5, QColor(255, 255, 0));
        gradient.setColorStopAt(0.75, QColor(255, 0, 0));
        gradient.setColorStopAt(1, QColor(128, 0, 128));
        colorScale->axis()->setLabel(QString("Thermal water vapor flux [%1]").arg(QString("l s-1")));
    }
    colorMap->setColorScale(colorScale);
    colorMap->setGradient(gradient);

}

