#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <qglobal.h>
#include <qlist.h>

#include "heat1D.h"
#include "soilFluxes3D.h"
#include "types.h"
#include "boundary.h"
#include "soilPhysics.h"
#include "commonConstants.h"

Crit3DOut output;
Heat1DSimulation myHeat1D;

meteo::meteo()
{
    airTemperature = NODATA;
    precipitation = NODATA;
    relativeHumidity = NODATA;
    windSpeed = NODATA;
    netIrradiance = NODATA;
}

void Heat1DSimulation::cleanMeteo()
{
    meteoValues.clear();
};


void setSoil(double thetaS_, double thetaR_, double clay_, double organicMatter_)
{
    myHeat1D.ThetaS = thetaS_;
    myHeat1D.ThetaR = thetaR_;
    myHeat1D.Clay = clay_;
    myHeat1D.OrganicMatter = organicMatter_;
}

void setTotalDepth(double myValue)
{   myHeat1D.TotalDepth = myValue;}

void setThickness(double myValue)
{   myHeat1D.Thickness = myValue;}

void setInitialSaturation(double myValueTop, double myValueBottom)
{   myHeat1D.initialSaturationTop = myValueTop;
    myHeat1D.initialSaturationBottom = myValueBottom;
}

void setInitialTemperature(double myValueTop, double myValueBottom)
{   myHeat1D.initialTemperatureTop = myValueTop;
    myHeat1D.initialTemperatureBottom = myValueBottom;
}

void setBottomTemperature(double myValue, double myDepth)
{
    myHeat1D.bottomTemperature = myValue;
    myHeat1D.bottomTemperatureDepth = myDepth;
}

void setProcesses(bool computeWaterProcess, bool computeHeat_, bool computeSolutesProcess)
{
    myHeat1D.computeWater = computeWaterProcess;
    myHeat1D.computeHeat = computeHeat_;
    myHeat1D.computeSolutes = computeSolutesProcess;
}

void setProcessesHeat(bool computeLatent_, bool computeAdvection_)
{
    myHeat1D.computeLatent = computeLatent_;
    myHeat1D.computeAdvection = computeAdvection_;
}

long getNodesNumber()
{   return (myHeat1D.NodesNumber - 1);}

void setSoilHorizonNumber(int myNumber)
{   myHeat1D.myHorizonNumber = myNumber;}

void setSurface(double myArea, double myRoughness, double minWaterRunoff, double myRoughnessHeat)
{
    myHeat1D.surfaceArea = myArea;
    myHeat1D.Roughness = myRoughness;
    myHeat1D.Plough = minWaterRunoff;
    myHeat1D.RoughnessHeat = myRoughnessHeat;

    int myResult = soilFluxes3D::setSurfaceProperties(0, myHeat1D.Roughness, myHeat1D.Plough);
    if (myResult != CRIT3D_OK) printf("\n error in SetSurfaceProperties!");
}


bool initializeSoil(bool useInputSoils)
{
    int myResult;

    // loam (Troy soil db)
    double VG_he        = 0.023;    //m
    double VG_alpha     = 1.76;     //m-1
    double VG_n         = 1.21;
    double mualemTort   = 0.5;
    double KSat         = 1. / (3600. * 100.);

    if (useInputSoils)
    {
        for (int mySoilIndex = 0; mySoilIndex < myHeat1D.myHorizonNumber; mySoilIndex++)
            myResult = soilFluxes3D::setSoilProperties(0,mySoilIndex,
                                          myInputSoils[mySoilIndex].VG_alfa,
                                          myInputSoils[mySoilIndex].VG_n,
                                          myInputSoils[mySoilIndex].VG_m,
                                          myInputSoils[mySoilIndex].VG_he,
                                          myInputSoils[mySoilIndex].Theta_r,
                                          myInputSoils[mySoilIndex].Theta_s,
                                          myInputSoils[mySoilIndex].K_sat,
                                          myInputSoils[mySoilIndex].Mualem_L,
                                          myInputSoils[mySoilIndex].OrganicMatter,
                                          myInputSoils[mySoilIndex].Clay);
    }
    else
        myResult = soilFluxes3D::setSoilProperties(0, 1, VG_alpha, VG_n, (1. - 1. / VG_n), VG_he, myHeat1D.ThetaR, myHeat1D.ThetaS, KSat, mualemTort, myHeat1D.OrganicMatter/100., myHeat1D.Clay/100.);

    if (myResult != CRIT3D_OK) {
        printf("\n error in SetSoilProperties");
        return(false);
    }
    else return true;
}

bool initializeHeat1D(bool useInputSoils)
{
    int myResult = 0;
    long indexNode;

    int boundaryType;
    float x = 0.;
    float y = 0.;
    double myThetaS, myThetaR;

    myHeat1D.NodesNumber = ceil(myHeat1D.TotalDepth / myHeat1D.Thickness) + 1;
    double *myDepth = (double *) calloc(myHeat1D.NodesNumber, sizeof(double));
    double *myThickness = (double *) calloc(myHeat1D.NodesNumber, sizeof(double));

    myThickness[0] = 0;
    myDepth[0] = 0.;

    int myNodeHorizon = 0;

    for (indexNode = 1 ; indexNode<myHeat1D.NodesNumber ; indexNode++ )
    {
        myThickness[indexNode] = myHeat1D.Thickness;
        if (indexNode == 1) myDepth[indexNode] = myDepth[indexNode-1] - (myHeat1D.Thickness / 2) ;
        else  myDepth[indexNode] = myDepth[indexNode-1] - myHeat1D.Thickness ;
    }

    myResult = soilFluxes3D::initialize(myHeat1D.NodesNumber, (short) myHeat1D.NodesNumber, 0, myHeat1D.computeWater, myHeat1D.computeHeat, myHeat1D.computeSolutes);
    if (myResult != CRIT3D_OK) printf("\n error in initialize");

    if (myHeat1D.computeHeat) soilFluxes3D::initializeHeat(SAVE_HEATFLUXES_ALL, myHeat1D.computeAdvection, myHeat1D.computeLatent);

    if (! initializeSoil(useInputSoils)) printf("\n error in setSoilProperties");
    soilFluxes3D::setHydraulicProperties(MODIFIEDVANGENUCHTEN, MEAN_LOGARITHMIC, 10.);
    soilFluxes3D::setNumericalParameters((float)0.1, 600., 100, 10, 12, 6);

    for (indexNode = 0 ; indexNode < myHeat1D.NodesNumber ; indexNode++ )
    {
        // elemento superficiale
        if (indexNode == 0)
        {
            myResult = soilFluxes3D::setNode(indexNode, x, y, myDepth[indexNode], myHeat1D.surfaceArea, true, false, BOUNDARY_NONE, 0.0);
            if (myResult != CRIT3D_OK) printf("\n error in setNode!");

            myResult = soilFluxes3D::setNodeSurface(0, 0) ;
            if (myResult != CRIT3D_OK) printf("\n error in setNodeSurface!");

            if (myHeat1D.computeWater)
            {
                myResult = soilFluxes3D::setWaterContent(indexNode, 0.);
                if (myResult != CRIT3D_OK) printf("\n error in setWaterContent!");
            }
        }

        // elementi sottosuperficiali
        else
        {
            if (indexNode == 1)
            {
                if (myHeat1D.computeHeat)
                    boundaryType = BOUNDARY_HEAT_SURFACE;
                else
                    boundaryType = BOUNDARY_NONE;

                myResult = soilFluxes3D::setNode(indexNode, x, y, myDepth[indexNode], myThickness[indexNode] * myHeat1D.surfaceArea, false, true, boundaryType, 0.0);
                if (myResult != CRIT3D_OK) printf("\n error in setNode!");
            }
            else if (indexNode == myHeat1D.NodesNumber - 1)
            {
                myResult = soilFluxes3D::setNode(indexNode, x ,y, myDepth[indexNode], myThickness[indexNode] * myHeat1D.surfaceArea, false, true, BOUNDARY_FREEDRAINAGE, 0.0);
                if (myResult != CRIT3D_OK) printf("\n error in setNode!");

                if (myHeat1D.computeHeat)
                {
                    myResult = soilFluxes3D::setFixedTemperature(indexNode, 273.16 + myHeat1D.bottomTemperature, myHeat1D.bottomTemperatureDepth);
                    if (myResult != CRIT3D_OK) printf("\n error in setFixedTemperature!");
                }
            }
            else
            {
                myResult = soilFluxes3D::setNode(indexNode, x, y, myDepth[indexNode], myThickness[indexNode] * myHeat1D.surfaceArea, false, false, BOUNDARY_NONE, 0.0);
                if (myResult != CRIT3D_OK) printf("\n error in setNode!");
			}											  
									  
            myResult = soilFluxes3D::setNodeLink(indexNode, indexNode - 1 , UP, myHeat1D.surfaceArea);
            if (myResult != CRIT3D_OK) printf("\n error in setNodeLink!");

            if (useInputSoils)
            {
                if (myDepth[indexNode]<myInputSoils[myNodeHorizon].profInf)
                    if (myNodeHorizon < myHeat1D.myHorizonNumber-1)
                        myNodeHorizon++;
            }																									
            else
                myNodeHorizon = 1;

			myResult = soilFluxes3D::setNodeSoil(indexNode, 0, myNodeHorizon);
			if (myResult != CRIT3D_OK) printf("\n error in setNodeSoil!");
																																  
            if (useInputSoils)
            {
                myThetaS = myInputSoils[myNodeHorizon].Theta_s;
                myThetaR = myInputSoils[myNodeHorizon].Theta_r;
            }
            else
            {
                myThetaS = myHeat1D.ThetaS;
                myThetaR = myHeat1D.ThetaR;
            }

            myResult = soilFluxes3D::setWaterContent(indexNode, ((indexNode-1)*(myHeat1D.initialSaturationBottom - myHeat1D.initialSaturationTop)/(myHeat1D.NodesNumber-2)+myHeat1D.initialSaturationTop) * (myThetaS - myThetaR) + myThetaR);
            if (myResult != CRIT3D_OK) printf("\n error in SetWaterContent!");

            if (myHeat1D.computeHeat)
			{
				myResult = soilFluxes3D::setTemperature(indexNode,
                    273.16 + ((indexNode-1)*(myHeat1D.initialTemperatureBottom - myHeat1D.initialTemperatureTop)/(myHeat1D.NodesNumber-2)+myHeat1D.initialTemperatureTop));

				if (myResult != CRIT3D_OK) printf("\n error in SetTemperature!");
			}
        }

		// tutti tranne ultimo nodo confinano con nodo sotto
        if (indexNode < myHeat1D.NodesNumber - 1)
		{
            myResult = soilFluxes3D::setNodeLink(indexNode, indexNode + 1 , DOWN, myHeat1D.surfaceArea);
            if (myResult != CRIT3D_OK) printf("\n error in SetNode sotto!");
		}																 
    }

    soilFluxes3D::initializeBalance();

	return (true);
}

void setSinkSources(double myHourlyPrec, int timeStepSeconds)
{
    for (long i=0; i < myHeat1D.NodesNumber; i++)
    {
        if (myHeat1D.computeHeat && i > 0) soilFluxes3D::setHeatSinkSource(i, 0);

        if (myHeat1D.computeWater)
        {
            if (i == 0)
                soilFluxes3D::setWaterSinkSource(i, (double)myHourlyPrec * myHeat1D.surfaceArea / timeStepSeconds / 1000);
            else
                soilFluxes3D::setWaterSinkSource(i, 0);
        }
    }
}


void Crit3DOut::clean()
{
    nrValues = 0;
    nrLayers = 0;

    errorOutput.clear();
    landSurfaceOutput.clear();
    profileOutput.clear();
}

Crit3DOut::Crit3DOut()
{
    nrValues = 0;
    nrLayers = 0;
    layerThickness = 0.;
}

bool isValid(double myValue)
{
    return (myValue != MEMORY_ERROR && myValue != MISSING_DATA_ERROR && myValue != INDEX_ERROR);
}

void getOutputAllPeriod(long firstIndex, long lastIndex, Crit3DOut *output)
{
    long myIndex;
    double myValue;
    double fluxDiff, fluxLtntIso, fluxLtntTh, fluxAdv, fluxTot;
    double watFluxIsoLiq, watFluxThLiq, watFluxIsoVap, watFluxThVap;

    QPointF myPoint;
    profileStatus myProfile;
    landSurfaceStatus mySurfaceOutput;
    heatErrors myErrors;

    output->nrValues++;
    output->profileOutput.push_back(myProfile);
    output->landSurfaceOutput.push_back(mySurfaceOutput);
    output->errorOutput.push_back(myErrors);

    for (myIndex = firstIndex ; myIndex <= lastIndex ; myIndex++ )
    {
        myPoint.setX(myIndex);

        myValue = soilFluxes3D::getTemperature(myIndex);
        if (isValid(myValue)) myValue -= 273.16;
        else myValue = NODATA;
        myPoint.setY(myValue);
        output->profileOutput[output->nrValues-1].temperature.push_back(myPoint);

        myValue = soilFluxes3D::getWaterContent(myIndex);
        if (! isValid(myValue)) myValue = NODATA;
        myPoint.setY(myValue);
        output->profileOutput[output->nrValues-1].waterContent.push_back(myPoint);

        myValue = soilFluxes3D::getHeatConductivity(myIndex);
        if (! isValid(myValue)) myValue = NODATA;
        myPoint.setY(myValue);
        output->profileOutput[output->nrValues-1].heatConductivity.push_back(myPoint);

        fluxTot = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_TOTAL);
        if (isValid(fluxTot)) fluxTot /= myHeat1D.surfaceArea;
        else fluxTot = NODATA;
        myPoint.setY(fluxTot);
        output->profileOutput[output->nrValues-1].totalHeatFlux.push_back(myPoint);

        fluxDiff = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_DIFFUSIVE);
        if (isValid(fluxDiff)) fluxDiff /= myHeat1D.surfaceArea;
        else fluxDiff = NODATA;

        fluxLtntIso = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_LATENT_ISOTHERMAL);
        if (isValid(fluxLtntIso)) fluxLtntIso /= myHeat1D.surfaceArea;
        else fluxLtntIso = NODATA;

        fluxLtntTh = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_LATENT_THERMAL);
        if (isValid(fluxLtntTh)) fluxLtntTh /= myHeat1D.surfaceArea;
        else fluxLtntTh = NODATA;

        fluxAdv = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_ADVECTIVE);
        if (isValid(fluxAdv)) fluxAdv /= myHeat1D.surfaceArea;
        else fluxAdv = NODATA;

        myPoint.setY(fluxDiff);
        output->profileOutput[output->nrValues-1].diffusiveHeatFlux.push_back(myPoint);

        myPoint.setY(fluxLtntIso);
        output->profileOutput[output->nrValues-1].isothermalLatentHeatFlux.push_back(myPoint);

        myPoint.setY(fluxLtntTh);
        output->profileOutput[output->nrValues-1].thermalLatentHeatFlux.push_back(myPoint);

        myPoint.setY(fluxAdv);
        output->profileOutput[output->nrValues-1].advectiveheatFlux.push_back(myPoint);

        watFluxIsoLiq = soilFluxes3D::getHeatFlux(myIndex, DOWN, WATERFLUX_LIQUID_ISOTHERMAL);
        if (isValid(watFluxIsoLiq)) watFluxIsoLiq *= 1000.;
        else watFluxIsoLiq = NODATA;

        watFluxThLiq  = soilFluxes3D::getHeatFlux(myIndex, DOWN, WATERFLUX_LIQUID_THERMAL);
        if (isValid(watFluxThLiq)) watFluxThLiq *= 1000.;
        else watFluxThLiq = NODATA;

        watFluxIsoVap = soilFluxes3D::getHeatFlux(myIndex, DOWN, WATERFLUX_VAPOR_ISOTHERMAL);
        if (isValid(watFluxIsoVap)) watFluxIsoVap *= 1000. / WATER_DENSITY;
        else watFluxIsoVap = NODATA;

        watFluxThVap = soilFluxes3D::getHeatFlux(myIndex, DOWN, WATERFLUX_VAPOR_THERMAL);
        if (isValid(watFluxThVap)) watFluxThVap *= 1000. / WATER_DENSITY;
        else watFluxThVap = NODATA;

        myPoint.setY(watFluxIsoLiq);
        output->profileOutput[output->nrValues-1].waterIsothermalLiquidFlux.push_back(myPoint);

        myPoint.setY(watFluxThLiq);
        output->profileOutput[output->nrValues-1].waterThermalLiquidFlux.push_back(myPoint);

        myPoint.setY(watFluxIsoVap);
        output->profileOutput[output->nrValues-1].waterIsothermalVaporFlux.push_back(myPoint);

        myPoint.setY(watFluxIsoLiq);
        output->profileOutput[output->nrValues-1].waterThermalVaporFlux.push_back(myPoint);
    }

    myPoint.setX(output->landSurfaceOutput.size() + 1);

    // net radiation (positive downward)
    myValue = soilFluxes3D::getBoundaryRadiativeFlux(1);
    if (isValid(myValue)) myPoint.setY(myValue);
    output->landSurfaceOutput[output->nrValues-1].netRadiation = myPoint;

    // sensible heat (positive upward)
    myValue = soilFluxes3D::getBoundarySensibleFlux(1);
    if (isValid(myValue)) myPoint.setY(-myValue);
    output->landSurfaceOutput[output->nrValues-1].sensibleHeat = myPoint;

    // latent heat (positive upward)
    myValue = soilFluxes3D::getBoundaryLatentFlux(1);
    if (isValid(myValue)) myPoint.setY(-myValue);
    output->landSurfaceOutput[output->nrValues-1].latentHeat = myPoint;

    //aerodynamic resistance
    myValue = soilFluxes3D::getBoundaryAerodynamicConductance(1);
    if (isValid(myValue)) myPoint.setY(1./myValue);
    output->landSurfaceOutput[output->nrValues-1].aeroResistance = myPoint;

    //soil surface resistance
    myValue = soilFluxes3D::getBoundarySoilConductance(1);
    if (isValid(myValue)) myPoint.setY(1./myValue);
    output->landSurfaceOutput[output->nrValues-1].soilResistance = myPoint;

    //errors
    myValue = soilFluxes3D::getHeatMBR();
    if (isValid(myValue)) myPoint.setY(myValue);
    output->errorOutput[output->nrValues-1].heatMBR = myPoint;

    myValue = soilFluxes3D::getWaterMBR();
    if (isValid(myValue)) myPoint.setY(myValue);
    output->errorOutput[output->nrValues-1].waterMBR = myPoint;
}

QString Crit3DOut::getTextOutput(outputGroup outGroup)
{
    QString myString = "";
    float myValue;    

    if (outGroup == outputGroup::soilTemperature)
        for (int j=1; j<=nrLayers; j++)
        {
            myString.append(QString("TempSoil_-"));
            myString.append(QString::number(j * layerThickness));
            myString.append(QString(","));
        }

    if (outGroup == outputGroup::soilWater)
        for (int j=0; j<=nrLayers; j++)
        {
            myString.append(QString("WaterContent_-"));
            myString.append(QString::number(j * layerThickness));
            myString.append(QString(","));
        }

    if (outGroup == outputGroup::totalHeatFlux)
        for (int j=1; j<=nrLayers; j++)
        {
            myString.append(QString("HeatFluxTot_-"));
            myString.append(QString::number(j * layerThickness));
            myString.append(QString(","));
        }

    if (outGroup == outputGroup::diffusiveHeatFlux)
        for (int j=1; j<=nrLayers; j++)
        {
            myString.append(QString("HeatFluxDiff_-"));
            myString.append(QString::number(j * layerThickness));
            myString.append(QString(","));
        }

    if (outGroup == outputGroup::latentHeatFluxIso)
        for (int j=1; j<=nrLayers; j++)
        {
            myString.append(QString("HeatFluxLatIso_-"));
            myString.append(QString::number(j * layerThickness));
            myString.append(QString(","));
        }

    if (outGroup == outputGroup::latentHeatFluxTherm)
        for (int j=1; j<=nrLayers; j++)
        {
            myString.append(QString("HeatFluxLatTherm_-"));
            myString.append(QString::number(j * layerThickness));
            myString.append(QString(","));
        }

    if (outGroup == outputGroup::waterIsothLiquidFlux)
        for (int j=1; j<=nrLayers; j++)
        {
            myString.append(QString("WaterFluxLiqIso_-"));
            myString.append(QString::number(j * layerThickness));
            myString.append(QString(","));
        }

    if (outGroup == outputGroup::waterThermLiquidFlux)
        for (int j=1; j<=nrLayers; j++)
        {
            myString.append(QString("WaterFluxLiqTherm_-"));
            myString.append(QString::number(j * layerThickness));
            myString.append(QString(","));
        }

    if (outGroup == outputGroup::waterIsothVaporFlux)
        for (int j=1; j<=nrLayers; j++)
        {
            myString.append(QString("WaterFluxVapIso_-"));
            myString.append(QString::number(j * layerThickness));
            myString.append(QString(","));
        }

    if (outGroup == outputGroup::waterThermVaporFlux)
        for (int j=1; j<=nrLayers; j++)
        {
            myString.append(QString("WaterFluxVapTherm_-"));
            myString.append(QString::number(j * layerThickness));
            myString.append(QString(","));
        }

    if (outGroup == outputGroup::energyBalance)
    {
        myString.append(QString("srfNetIrrad,"));
        myString.append(QString("srfSnsblHeat,"));
        myString.append(QString("srfLtntHeat,"));
    }

    if (outGroup == outputGroup::surfaceResistances)
    {
        myString.append(QString("aeroResistance,"));
        myString.append(QString("soilResistance,"));
    }

    if (outGroup == outputGroup::errorBalance)
    {
        myString.append(QString("HeatMBR,"));
        myString.append(QString("WaterMBR"));
    }

    myString.append(QString("\n"));

    for (int i=0; i<nrValues; i++)
    {
        if (outGroup == outputGroup::soilTemperature)
            for (int j=1; j<=nrLayers; j++)
            {
                myValue = profileOutput[i].temperature[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        if (outGroup == outputGroup::soilWater)
            for (int j=0; j<=nrLayers; j++)
            {
                myValue = profileOutput[i].waterContent[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        if (outGroup == outputGroup::energyBalance)
        {
            myValue = landSurfaceOutput[i].netRadiation.y();
            myString.append(QString::number(myValue,'f',2));
            myString.append(QString(","));

            myValue = landSurfaceOutput[i].sensibleHeat.y();
            myString.append(QString::number(myValue,'f',2));
            myString.append(QString(","));

            myValue = landSurfaceOutput[i].latentHeat.y();
            myString.append(QString::number(myValue,'f',2));
            myString.append(QString(","));
        }

        if (outGroup == outputGroup::surfaceResistances)
        {
            myValue = landSurfaceOutput[i].aeroResistance.y();
            myString.append(QString::number(myValue,'f',6));
            myString.append(QString(", "));

            myValue = landSurfaceOutput[i].soilResistance.y();
            myString.append(QString::number(myValue,'f',6));
            myString.append(QString(","));
        }

        if (outGroup == outputGroup::errorBalance)
        {
            myValue = errorOutput[i].heatMBR.y();
            myString.append(QString::number(myValue,'f',12));
            myString.append(QString(","));

            myValue = errorOutput[i].waterMBR.y();
            myString.append(QString::number(myValue,'f',12));
        }

        if (outGroup == outputGroup::totalHeatFlux)
            for (int j=0; j<=nrLayers; j++)
            {
                myValue = profileOutput[i].totalHeatFlux[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        if (outGroup == outputGroup::diffusiveHeatFlux)
            for (int j=0; j<=nrLayers; j++)
            {
                myValue = profileOutput[i].diffusiveHeatFlux[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        if (outGroup == outputGroup::latentHeatFluxIso)
            for (int j=0; j<=nrLayers; j++)
            {
                myValue = profileOutput[i].isothermalLatentHeatFlux[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        if (outGroup == outputGroup::latentHeatFluxTherm)
            for (int j=0; j<=nrLayers; j++)
            {
                myValue = profileOutput[i].thermalLatentHeatFlux[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        if (outGroup == outputGroup::waterIsothLiquidFlux)
            for (int j=0; j<=nrLayers; j++)
            {
                myValue = profileOutput[i].waterIsothermalLiquidFlux[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        if (outGroup == outputGroup::waterThermLiquidFlux)
            for (int j=0; j<=nrLayers; j++)
            {
                myValue = profileOutput[i].waterThermalLiquidFlux[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        if (outGroup == outputGroup::waterIsothVaporFlux)
            for (int j=0; j<=nrLayers; j++)
            {
                myValue = profileOutput[i].waterIsothermalVaporFlux[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        if (outGroup == outputGroup::waterThermVaporFlux)
            for (int j=0; j<=nrLayers; j++)
            {
                myValue = profileOutput[i].waterThermalVaporFlux[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        myString.append(QString("\n"));

    }

    return myString;
}


bool runHeat1D(double myHourlyTemperature,  double myHourlyRelativeHumidity,
                 double myHourlyWindSpeed, double myHourlyNetIrradiance,
                 double myHourlyPrec, int timeStepSeconds)
{
    //double currentRoughness;
    //double surfaceWaterHeight;
    //double roughnessWater = 0.005;

    setSinkSources(myHourlyPrec, timeStepSeconds);

    if (myHeat1D.computeHeat)
    {
        soilFluxes3D::setHeatBoundaryHeightWind(1, 2);
        soilFluxes3D::setHeatBoundaryHeightTemperature(1, 1.5);
        soilFluxes3D::setHeatBoundaryTemperature(1, myHourlyTemperature);
        soilFluxes3D::setHeatBoundaryRelativeHumidity(1, myHourlyRelativeHumidity);
        soilFluxes3D::setHeatBoundaryWindSpeed(1, myHourlyWindSpeed);
        soilFluxes3D::setHeatBoundaryNetIrradiance(1, myHourlyNetIrradiance);

        /*
        surfaceWaterHeight = soilFluxes3D::getWaterContent(0);
        if (surfaceWaterHeight > RoughnessHeat)
            currentRoughness = 0.005;
        else if (surfaceWaterHeight > 0.)
            currentRoughness = (roughnessWater - RoughnessHeat) / RoughnessHeat * surfaceWaterHeight + RoughnessHeat;
        else
            currentRoughness = RoughnessHeat;
        */

        soilFluxes3D::setHeatBoundaryRoughness(1, myHeat1D.RoughnessHeat);
    }

    soilFluxes3D::computePeriod(timeStepSeconds);

	return (true);
}


