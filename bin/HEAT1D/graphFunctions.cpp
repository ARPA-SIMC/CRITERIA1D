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
             myOut == errorBalance)
        return outputType::single;
    else
        return outputType::single;

}

void setColorScale(Crit3DColorScale* myColorScale, outputGroup outGroup, Crit3DOut *myOut, bool* graphLinear)
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
        *graphLinear = false;
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
        *graphLinear = true;
    }

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
        myPoint.setX(i+1);

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







