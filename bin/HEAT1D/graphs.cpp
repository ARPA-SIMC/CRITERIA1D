#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_plot_dict.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_plot_dict.h>

#include "gis.h"
#include "graphs.h"
#include "commonConstants.h"

Plot::Plot( QWidget *parent ):
    QwtPlot( parent )
{
    canvas()->setStyleSheet(
        "border: 1px solid Black;"
        "border-radius: 15px;"
        "background-color: white;"
    );

    // panning with the left mouse button
    (void )new QwtPlotPanner( canvas() );

    // zoom in/out with the wheel
    QwtPlotMagnifier *magnifier = new QwtPlotMagnifier( canvas() );
    magnifier->setMouseButton( Qt::NoButton );

    QwtLegend *legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box|QFrame::Sunken);
    this->insertLegend(legend, QwtPlot::BottomLegend);
}

QwtPlotGappedCurve::QwtPlotGappedCurve(const QString &title, double gapValue)
:	QwtPlotCurve(title),
    gapValue_(gapValue)
{
}

void QwtPlotGappedCurve::drawSeries(QPainter *painter, const QwtScaleMap &xMap,
                                                          const QwtScaleMap &yMap, const QRectF &canvRect, int from, int to) const
{
    if ( !painter || (int)dataSize() <= 0 )
        return;

    if (to < 0)
        to = (int)dataSize() - 1;

    int i = from;
    while (i < to)
    {
        // If data begins with missed values,
        // we need to find first non-missed point.
            double y = sample(i).y();
            while ((i < to) && (y == gapValue_))
            {
                ++i;
                y = sample(i).y();
            }
        // First non-missed point will be the start of curve section.
            int start = i;
            y = sample(i).y();
        // Find the last non-missed point, it will be the end of curve section.
            while ((i < to) && (y != gapValue_))
            {
                ++i;
                y = sample(i).y();
            }
        // Correct the end of the section if it is at missed point
            int end = (y == gapValue_) ? i - 1 : i;

        // Draw the curve section
            if (start <= end)
                    QwtPlotCurve::drawSeries(painter, xMap, yMap, canvRect, start, end);
    }
}

outputType getOutputType(outputGroup myOut)
{
    if (myOut == soilTemperature ||
        myOut == soilWater ||
        myOut == totalHeatFlux ||
        myOut == diffusiveHeatFlux ||
        myOut == latentHeatFluxIso ||
        myOut == latentHeatFluxTherm ||
        myOut == waterIsothLiquidFlux ||
        myOut == waterThermLiquidFlux ||
        myOut == waterIsothVaporFlux ||
        myOut == waterThermVaporFlux ||
        myOut == soilHeatConductivity)

        return outputType::profile;
    else if (myOut == energyBalance ||
             myOut == errorBalance)
        return outputType::single;
    else
        return outputType::single;

}

void Plot::addCurve(QString myTitle, QwtPlotGappedCurve::CurveStyle myStyle, QPen myPen, QVector<QPointF> &samples, bool isSecondaryAxis)
{
    QwtPlotGappedCurve* myCurve = new QwtPlotGappedCurve(myTitle, NODATA);

    myCurve->setPen(myPen);
    myCurve->setStyle(myStyle);
    myCurve->setSamples(samples);

    if (isSecondaryAxis)
        myCurve->setYAxis(QwtPlot::yRight);

    myCurve->attach( this );
}

QVector<QPointF> getSingleSeries(Crit3DOut* myOut, outputVar myVar, float* MINVALUE, float* MAXVALUE)
{
    QVector<QPointF> mySeries;
    QPointF myPoint;
    qreal myVal;

    *MINVALUE = NODATA;
    *MAXVALUE = NODATA;

    for (int i=0; i<myOut->nrValues; i++)
    {
        myPoint.setX(i);

        switch (myVar)
        {
            case outputVar::surfaceNetIrradiance :
                myVal = myOut->landSurfaceOutput[i].netRadiation.y();
                break;

            case outputVar::surfaceSensibleHeat :
                myVal = myOut->landSurfaceOutput[i].sensibleHeat.y();
                break;

            case outputVar::surfaceLatentHeat :
                myVal = myOut->landSurfaceOutput[i].latentHeat.y();
                break;

            case outputVar::aerodynamicResistence :
                myVal = myOut->landSurfaceOutput[i].aeroResistance.y();
                break;

            case outputVar::soilSurfaceResistence :
                myVal = myOut->landSurfaceOutput[i].soilResistance.y();
                break;

            case outputVar::MBR_heat :
                myVal = myOut->errorOutput[i].heatMBR.y();
                break;

            case outputVar::MBR_water :
                myVal = myOut->errorOutput[i].waterMBR.y();
                break;

            case outputVar::MBE_heat :
                myVal = myOut->errorOutput[i].heatMBE.y();
                break;
        }

        myPoint.setY(myVal);
        mySeries.push_back(myPoint);
        *MINVALUE = (*MINVALUE == NODATA) ? myVal : ((myVal < *MINVALUE) ? myVal : *MINVALUE);
        *MAXVALUE = (*MAXVALUE == NODATA) ? myVal : ((myVal > *MAXVALUE) ? myVal : *MAXVALUE);
    }

    return mySeries;
}

QVector<QPointF> getProfileSeries(Crit3DOut* myOut, outputGroup myVar, int layerIndex, float* MINVALUE, float* MAXVALUE)
{
    QVector<QPointF> mySeries;
    QPointF myPoint;
    qreal myVal;

    *MINVALUE = NODATA;
    *MAXVALUE = NODATA;

    for (int i=0; i<myOut->nrValues; i++)
    {
        myPoint.setX(i);

        switch (myVar)
        {
            case outputGroup::soilTemperature :
                myVal = myOut->profileOutput[i].temperature[layerIndex].y();
                break;

            case outputGroup::soilWater :
                myVal = myOut->profileOutput[i].waterContent[layerIndex].y();
                break;

            case outputGroup::soilHeatConductivity :
                myVal = myOut->profileOutput[i].heatConductivity[layerIndex].y();
                break;

            case outputGroup::totalHeatFlux :
                myVal = myOut->profileOutput[i].totalHeatFlux[layerIndex].y();
                break;

            case outputGroup::latentHeatFluxIso :
                myVal = myOut->profileOutput[i].isothermalLatentHeatFlux[layerIndex].y();
                break;

            case outputGroup::latentHeatFluxTherm :
                myVal = myOut->profileOutput[i].thermalLatentHeatFlux[layerIndex].y();
                break;

            case outputGroup::diffusiveHeatFlux :
                myVal = myOut->profileOutput[i].diffusiveHeatFlux[layerIndex].y();
                break;

            case outputGroup::waterIsothLiquidFlux :
                myVal = myOut->profileOutput[i].waterIsothermalLiquidFlux[layerIndex].y();
                break;

            case outputGroup::waterThermLiquidFlux :
                myVal = myOut->profileOutput[i].waterThermalLiquidFlux[layerIndex].y();
                break;

            case outputGroup::waterIsothVaporFlux :
                myVal = myOut->profileOutput[i].waterIsothermalVaporFlux[layerIndex].y();
                break;

            case outputGroup::waterThermVaporFlux :
                myVal = myOut->profileOutput[i].waterThermalVaporFlux[layerIndex].y();
                break;
        }

        myPoint.setY(myVal);
        mySeries.push_back(myPoint);
        *MINVALUE = (*MINVALUE == NODATA) ? myVal : ((myVal < *MINVALUE) ? myVal : *MINVALUE);
        *MAXVALUE = (*MAXVALUE == NODATA) ? myVal : ((myVal > *MAXVALUE) ? myVal : *MAXVALUE);
    }

    return mySeries;
}

void Plot::drawSingle(outputGroup graphType, Crit3DOut* myOut, Crit3DColorScale myColorScale)
{
    QPen myPen;
    Crit3DColor* myColor;
    QColor myQColor;

    QVector<QPointF> mySeries;

    float minGraph, maxGraph, minSeries, maxSeries, minGraph2, maxGraph2;
    minGraph = maxGraph = minSeries = maxSeries = minGraph2 = maxGraph2 = NODATA;

    bool yRightAxisActive = false;

    int myIndex = 0;

    if (graphType == outputGroup::energyBalance)
    {
        myColor = myColorScale.getColor(myIndex);
        myQColor = QColor(myColor->red, myColor->green, myColor->blue);
        myPen.setColor(myQColor);
        mySeries = getSingleSeries(myOut, outputVar::surfaceNetIrradiance, &minSeries, &maxSeries);
        addCurve(outputVarString[(int)outputVar::surfaceNetIrradiance], QwtPlotCurve::Lines, myPen, mySeries, false);
        minGraph = (minGraph == NODATA) ? minSeries : ((minSeries != NODATA && minSeries < minGraph) ? minSeries : minGraph);
        maxGraph = (maxGraph == NODATA) ? maxSeries : ((maxSeries != NODATA && maxSeries > maxGraph) ? maxSeries : maxGraph);

        myIndex++;
        myColor = myColorScale.getColor(myIndex);
        myQColor = QColor(myColor->red, myColor->green, myColor->blue);
        myPen.setColor(myQColor);
        mySeries = getSingleSeries(myOut, outputVar::surfaceSensibleHeat, &minSeries, &maxSeries);
        addCurve(outputVarString[(int)outputVar::surfaceSensibleHeat], QwtPlotCurve::Lines, myPen, mySeries, false);
        minGraph = (minGraph == NODATA) ? minSeries : ((minSeries != NODATA && minSeries < minGraph) ? minSeries : minGraph);
        maxGraph = (maxGraph == NODATA) ? maxSeries : ((maxSeries != NODATA && maxSeries > maxGraph) ? maxSeries : maxGraph);

        myIndex++;
        myColor = myColorScale.getColor(myIndex);
        myQColor = QColor(myColor->red, myColor->green, myColor->blue);
        myPen.setColor(myQColor);
        mySeries = getSingleSeries(myOut, outputVar::surfaceLatentHeat, &minSeries, &maxSeries);
        addCurve(outputVarString[(int)outputVar::surfaceLatentHeat], QwtPlotCurve::Lines, myPen, mySeries, false);
        minGraph = (minGraph == NODATA) ? minSeries : ((minSeries != NODATA && minSeries < minGraph) ? minSeries : minGraph);
        maxGraph = (maxGraph == NODATA) ? maxSeries : ((maxSeries != NODATA && maxSeries > maxGraph) ? maxSeries : maxGraph);
    }
    else if (graphType == outputGroup::errorBalance)
    {
        myColor = myColorScale.getColor(myIndex);
        myQColor = QColor(myColor->red, myColor->green, myColor->blue);
        myPen.setColor(myQColor);
        mySeries = getSingleSeries(myOut, outputVar::MBR_heat, &minSeries, &maxSeries);
        addCurve(outputVarString[(int)outputVar::MBR_heat], QwtPlotCurve::Lines, myPen, mySeries, false);
        minGraph = (minGraph == NODATA) ? minSeries : ((minSeries != NODATA && minSeries < minGraph) ? minSeries : minGraph);
        maxGraph = (maxGraph == NODATA) ? maxSeries : ((maxSeries != NODATA && maxSeries > maxGraph) ? maxSeries : maxGraph);

        myIndex++;
        myColor = myColorScale.getColor(myIndex);
        myQColor = QColor(myColor->red, myColor->green, myColor->blue);
        myPen.setColor(myQColor);
        mySeries = getSingleSeries(myOut, outputVar::MBR_water, &minSeries, &maxSeries);
        addCurve(outputVarString[(int)outputVar::MBR_water], QwtPlotCurve::Lines, myPen, mySeries, false);
        minGraph = (minGraph == NODATA) ? minSeries : ((minSeries != NODATA && minSeries < minGraph && minSeries != 0.) ? minSeries : minGraph);
        maxGraph = (maxGraph == NODATA) ? maxSeries : ((maxSeries != NODATA && maxSeries > maxGraph) ? maxSeries : maxGraph);

        /*
        myIndex++;
        myColor = myColorScale.getColor(myIndex);
        myQColor = QColor(myColor->red, myColor->green, myColor->blue);
        myPen.setColor(myQColor);
        mySeries = getSingleSeries(myOut, outputVar::MBE_heat, &minSeries, &maxSeries);
        addCurve(outputVarString[(int)outputVar::MBE_heat], QwtPlotCurve::Lines, myPen, mySeries, true);
        minGraph2 = (minGraph2 == NODATA) ? minSeries : ((minSeries < minGraph2) ? minSeries : minGraph2);
        maxGraph2 = (maxGraph2 == NODATA) ? maxSeries : ((maxSeries > maxGraph2) ? maxSeries : maxGraph2);
        */

        yRightAxisActive = false;
    }
    else if (graphType == outputGroup::surfaceResistances)
    {
        myColor = myColorScale.getColor(myIndex);
        myQColor = QColor(myColor->red, myColor->green, myColor->blue);
        myPen.setColor(myQColor);
        mySeries = getSingleSeries(myOut, outputVar::aerodynamicResistence, &minSeries, &maxSeries);
        addCurve(outputVarString[(int)outputVar::aerodynamicResistence], QwtPlotCurve::Lines, myPen, mySeries, false);
        minGraph = (minGraph == NODATA) ? minSeries : ((minSeries != NODATA && minSeries < minGraph) ? minSeries : minGraph);
        maxGraph = (maxGraph == NODATA) ? maxSeries : ((maxSeries != NODATA && maxSeries > maxGraph) ? maxSeries : maxGraph);

        myIndex++;
        myColor = myColorScale.getColor(myIndex);
        myQColor = QColor(myColor->red, myColor->green, myColor->blue);
        myPen.setColor(myQColor);
        mySeries = getSingleSeries(myOut, outputVar::soilSurfaceResistence, &minSeries, &maxSeries);
        addCurve(outputVarString[(int)outputVar::soilSurfaceResistence], QwtPlotCurve::Lines, myPen, mySeries, false);
        minGraph = (minGraph == NODATA) ? minSeries : ((minSeries != NODATA && minSeries < minGraph) ? minSeries : minGraph);
        maxGraph = (maxGraph == NODATA) ? maxSeries : ((maxSeries != NODATA && maxSeries > maxGraph) ? maxSeries : maxGraph);
    }

    double eps = 0.0001;
    if (fabs(minGraph-maxGraph) < eps)
    {
        minGraph -= eps;
        maxGraph += eps;
    }

    if (fabs(minGraph2-maxGraph2) < eps)
    {
        minGraph2 -= eps;
        maxGraph2 += eps;
    }
    this->enableAxis(this->yRight, yRightAxisActive);

    this->setAxisScale(this->yLeft, minGraph, maxGraph);
    this->setAxisScale(this->yRight, minGraph2, maxGraph2);
}

void Plot::drawProfile(outputGroup graphType, Crit3DOut* myOut, Crit3DColorScale myColorScale)
{
    QPen myPen;
    Crit3DColor* myColor;
    QColor myQColor;

    float myDepth;

    QVector<QPointF> mySeries;

    float minGraph, maxGraph, minSeries, maxSeries;
    minGraph = maxGraph = minSeries = maxSeries = NODATA;

    for (int z=0; z <= myOut->nrLayers; z++)
    {
        myDepth = - myOut->layerThickness * ((float)z - 0.5);

        myColor = myColorScale.getColor(z);
        myQColor = QColor(myColor->red, myColor->green, myColor->blue);
        myPen.setColor(myQColor);

        mySeries = getProfileSeries(myOut, graphType, z, &minSeries, &maxSeries);
        minGraph = (minGraph == NODATA) ? minSeries : ((minSeries != NODATA && minSeries < minGraph) ? minSeries : minGraph);
        maxGraph = (maxGraph == NODATA) ? maxSeries : ((maxSeries != NODATA && maxSeries > maxGraph) ? maxSeries : maxGraph);

        addCurve(QString::number(myDepth,'f',3), QwtPlotCurve::Lines, myPen, mySeries, false);
    }

    double eps = 0.0001;
    if (fabs(minGraph-maxGraph) < eps)
    {
        minGraph -= eps;
        maxGraph += eps;
    }

    this->setAxisScale(this->yLeft, minGraph, maxGraph);
}


void Plot::drawOutput(outputGroup outGroup, Crit3DOut *myOut)
{
    detachItems(QwtPlotItem::Rtti_PlotCurve, true);

    Crit3DColorScale myColorScale;

    outputType outType = getOutputType(outGroup);
    myColorScale.nrKeyColors = 3;
    myColorScale.nrColors = 256;
    myColorScale.keyColor = new Crit3DColor[myColorScale.nrKeyColors];
    myColorScale.color = new Crit3DColor[myColorScale.nrColors];
    myColorScale.classification = classificationMethod::EqualInterval;
    myColorScale.keyColor[0] = Crit3DColor(0, 0, 255);         /*!< blue */
    myColorScale.keyColor[1] = Crit3DColor(255, 255, 0);       /*!< yellow */
    myColorScale.keyColor[2] = Crit3DColor(255, 0, 0);         /*!< red */
    myColorScale.minimum = 0;

    if (outType == outputType::profile)
    {
        myColorScale.maximum = myOut->nrLayers-1;
        myColorScale.classify();
        drawProfile(outGroup, myOut, myColorScale);
    }
    else
    {
        if (outGroup == outputGroup::errorBalance)
            myColorScale.maximum = 2;
        else if (outGroup ==outputGroup::energyBalance)
            myColorScale.maximum = 3;
        else if (outGroup == outputGroup::surfaceResistances)
            myColorScale.maximum = 2;

        myColorScale.classify();
        drawSingle(outGroup, myOut, myColorScale);
    }

    replot();
}

void setColorScale(Crit3DColorScale* myColorScale, outputGroup outGroup, Crit3DOut *myOut)
{

    outputType outType = getOutputType(outGroup);
    myColorScale->nrKeyColors = 3;
    myColorScale->nrColors = 256;
    myColorScale->keyColor = new Crit3DColor[myColorScale->nrKeyColors];
    myColorScale->color = new Crit3DColor[myColorScale->nrColors];
    myColorScale->classification = classificationMethod::EqualInterval;
    myColorScale->keyColor[0] = Crit3DColor(0, 0, 255);         /*!< blue */
    myColorScale->keyColor[1] = Crit3DColor(255, 255, 0);       /*!< yellow */
    myColorScale->keyColor[2] = Crit3DColor(255, 0, 0);         /*!< red */
    myColorScale->minimum = 0;

    if (outType == outputType::profile)
    {
        myColorScale->maximum = myOut->nrLayers-1;
        myColorScale->classify();
        //drawProfile(outGroup, myOut, myColorScale);
    }
    else
    {
        if (outGroup == outputGroup::errorBalance)
            myColorScale->maximum = 2;
        else if (outGroup ==outputGroup::energyBalance)
            myColorScale->maximum = 3;
        else if (outGroup == outputGroup::surfaceResistances)
            myColorScale->maximum = 2;

        myColorScale->classify();
        //drawSingle(outGroup, myOut, myColorScale);
    }

}







