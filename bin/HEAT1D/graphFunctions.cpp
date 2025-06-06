#include "gis.h"
#include "graphFunctions.h"
#include "commonConstants.h"

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
             myOut == errorBalance ||
             myOut == bottom ||
             myOut == waterStorage)
        return outputType::single;
    else
        return outputType::single;

}


void setColorScale(Crit3DColorScale* myColorScale, outputGroup outGroup, Crit3DOut *myOut, bool* graphLinear)
{
    outputType outType = getOutputType(outGroup);
    myColorScale->initialize(3, 256);

    myColorScale->keyColor[0] = Crit3DColor(0, 0, 255);         /*!< blue */
    myColorScale->keyColor[1] = Crit3DColor(255, 255, 0);       /*!< yellow */
    myColorScale->keyColor[2] = Crit3DColor(255, 0, 0);         /*!< red */

    myColorScale->setMinimum(0);

    if (outType == outputType::profile)
    {
        myColorScale->setMaximum(float(myOut->nrLayers-1));
        *graphLinear = false;
    }
    else
    {
        if (outGroup == outputGroup::errorBalance)
            myColorScale->setMaximum(2);
        else if (outGroup ==outputGroup::energyBalance)
            myColorScale->setMaximum(3);
        else if (outGroup == outputGroup::surfaceResistances)
            myColorScale->setMaximum(2);
        else if (outGroup == outputGroup::bottom)
            myColorScale->setMaximum(1);
        else if (outGroup == outputGroup::waterStorage)
            myColorScale->setMaximum(1);

        *graphLinear = true;
    }

    myColorScale->classify();
}


QVector<QPointF> getSingleSeries(Crit3DOut* myOut, outputVar myVar, float* minSeries, float* maxSeries)
{
    QVector<QPointF> mySeries;
    QPointF myPoint;
    qreal myVal;

    *minSeries = NODATA;
    *maxSeries = NODATA;

    for (int i=0; i<myOut->nrValues; i++)
    {        
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

            case outputVar::bottomDrainage :
                myVal = myOut->bottomFluxes[i].drainage.y();
                break;

            case outputVar::storedWater :
                myVal = myOut->waterStorageOutput[i].waterStord.y();
                break;
        }

        myPoint.setX(myOut->landSurfaceOutput[i].netRadiation.x());
        myPoint.setY(myVal);
        mySeries.push_back(myPoint);
        *minSeries = (*minSeries == NODATA) ? myVal : ((myVal < *minSeries) ? myVal : *minSeries);
        *maxSeries = (*maxSeries == NODATA) ? myVal : ((myVal > *maxSeries) ? myVal : *maxSeries);
    }

    return mySeries;
}

QVector<QPointF> getProfileSeries(Crit3DOut* myOut, outputGroup myVar, int layerIndex, float* minSeries, float* maxSeries)
{
    QVector<QPointF> mySeries;
    QPointF myPoint;
    qreal myVal, myX;

    *minSeries = NODATA;
    *maxSeries = NODATA;

    for (int i=0; i<myOut->nrValues; i++)
    {
        myX = NODATA;
        myVal = NODATA;

        switch (myVar)
        {
            case outputGroup::soilTemperature :
                myX = myOut->profileOutput[i].temperature[layerIndex].x();
                myVal = myOut->profileOutput[i].temperature[layerIndex].y();
                break;

            case outputGroup::soilWater :
                myX = myOut->profileOutput[i].waterContent[layerIndex].x();
                myVal = myOut->profileOutput[i].waterContent[layerIndex].y();
                break;

            case outputGroup::soilHeatConductivity :
                myX = myOut->profileOutput[i].heatConductivity[layerIndex].x();
                myVal = myOut->profileOutput[i].heatConductivity[layerIndex].y();
                break;

            case outputGroup::totalHeatFlux :
                myX = myOut->profileOutput[i].totalHeatFlux[layerIndex].x();
                myVal = myOut->profileOutput[i].totalHeatFlux[layerIndex].y();
                break;

            case outputGroup::latentHeatFluxIso :
                myX = myOut->profileOutput[i].isothermalLatentHeatFlux[layerIndex].x();
                myVal = myOut->profileOutput[i].isothermalLatentHeatFlux[layerIndex].y();
                break;

            case outputGroup::latentHeatFluxTherm :
                myX = myOut->profileOutput[i].thermalLatentHeatFlux[layerIndex].x();
                myVal = myOut->profileOutput[i].thermalLatentHeatFlux[layerIndex].y();
                break;

            case outputGroup::diffusiveHeatFlux :
                myX = myOut->profileOutput[i].diffusiveHeatFlux[layerIndex].x();
                myVal = myOut->profileOutput[i].diffusiveHeatFlux[layerIndex].y();
                break;

            case outputGroup::waterIsothLiquidFlux :
                myX = myOut->profileOutput[i].waterIsothermalLiquidFlux[layerIndex].x();
                myVal = myOut->profileOutput[i].waterIsothermalLiquidFlux[layerIndex].y();
                break;

            case outputGroup::waterThermLiquidFlux :
                myX = myOut->profileOutput[i].waterThermalLiquidFlux[layerIndex].x();
                myVal = myOut->profileOutput[i].waterThermalLiquidFlux[layerIndex].y();
                break;

            case outputGroup::waterIsothVaporFlux :
                myX = myOut->profileOutput[i].waterIsothermalVaporFlux[layerIndex].x();
                myVal = myOut->profileOutput[i].waterIsothermalVaporFlux[layerIndex].y();
                break;

            case outputGroup::waterThermVaporFlux :
                myX = myOut->profileOutput[i].waterThermalVaporFlux[layerIndex].x();
                myVal = myOut->profileOutput[i].waterThermalVaporFlux[layerIndex].y();
                break;
        }

        myPoint.setX(myX);
        myPoint.setY(myVal);
        mySeries.push_back(myPoint);
        *minSeries = (*minSeries == NODATA) ? myVal : ((myVal < *minSeries) ? myVal : *minSeries);
        *maxSeries = (*maxSeries == NODATA) ? myVal : ((myVal > *maxSeries) ? myVal : *maxSeries);
    }

    return mySeries;
}







