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

//structure
long NodesNumber;
double TotalDepth ;
double Thickness;
long SimulationStart, SimulationStop;

//surface
double mySurface, Roughness, Plough, RoughnessHeat;

//bottom boundary
double bottomTemperature, bottomTemperatureDepth;

//meteo
double airRelativeHumidity, airTemperature, windSpeed, globalRad;
double precHourlyAmount, precHours, precIniHour, precTemperature;

//initialization
double initialSaturationTop, initialSaturationBottom, initialTemperatureTop, initialTemperatureBottom;

//processes
bool computeWater, computeSolutes, computeHeat;
bool computeAdvection, computeLatent;

long CurrentHour;

//soils
int myHorizonNumber;
double ThetaS, ThetaR, Clay, OrganicMatter;

void setTotalDepth(double myValue)
{   TotalDepth = myValue;}

void setThickness(double myValue)
{   Thickness = myValue;}

void setSimulationStart(int myValue)
{   SimulationStart = myValue;}

void setSimulationStop(int myValue)
{   SimulationStop = myValue;}

void setInitialSaturation(double myValueTop, double myValueBottom)
{   initialSaturationTop = myValueTop;
    initialSaturationBottom = myValueBottom;
}

void setInitialTemperature(double myValueTop, double myValueBottom)
{   initialTemperatureTop = myValueTop;
    initialTemperatureBottom = myValueBottom;
}

void setBottomTemperature(double myValue, double myDepth)
{
    bottomTemperature = myValue;
    bottomTemperatureDepth = myDepth;
}

void setSoil(double thetaS_, double thetaR_, double clay_, double organicMatter_)
{
    ThetaS = thetaS_;
    ThetaR = thetaR_;
    Clay = clay_;
    OrganicMatter = organicMatter_;
}

void setProcesses(bool computeWaterProcess, bool computeHeat_, bool computeSolutesProcess)
{
    computeWater = computeWaterProcess;
    computeHeat = computeHeat_;
    computeSolutes = computeSolutesProcess;
}

void setProcessesHeat(bool computeLatent_, bool computeAdvection_)
{
    computeLatent = computeLatent_;
    computeAdvection = computeAdvection_;

}

long getNodesNumber()
{   return (NodesNumber - 1);}

void setSoilHorizonNumber(int myNumber)
{   myHorizonNumber = myNumber;}

void setSurface(double myArea, double myRoughness, double minWaterRunoff, double myRoughnessHeat)
{
    mySurface = myArea;
    Roughness = myRoughness;
    Plough = minWaterRunoff;
    RoughnessHeat = myRoughnessHeat;

    int myResult = soilFluxes3D::setSurfaceProperties(0, Roughness, Plough);
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
        for (int mySoilIndex = 0; mySoilIndex < myHorizonNumber; mySoilIndex++)
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
        myResult = soilFluxes3D::setSoilProperties(0, 1, VG_alpha, VG_n, (1. - 1. / VG_n), VG_he, ThetaR, ThetaS, KSat, mualemTort, OrganicMatter/100., Clay/100.);

    if (myResult != CRIT3D_OK) {
        printf("\n error in SetSoilProperties");
        return(false);
    }
    else return true;
}

bool initializeHeat1D(long *myHourIni, long *myHourFin, bool useInputSoils)
{
    int myResult = 0;
    long indexNode;

    int boundaryType;
    float x = 0.;
    float y = 0.;
    double myThetaS, myThetaR;

    NodesNumber = ceil(TotalDepth / Thickness) + 1;
    double *myDepth = (double *) calloc(NodesNumber, sizeof(double));
    double *myThickness = (double *) calloc(NodesNumber, sizeof(double));

    myThickness[0] = 0;
    myDepth[0] = 0.;

    int myNodeHorizon = 0;

    for (indexNode = 1 ; indexNode<NodesNumber ; indexNode++ )
    {
        myThickness[indexNode] = Thickness;
        if (indexNode == 1) myDepth[indexNode] = myDepth[indexNode-1] - (Thickness / 2) ;
        else  myDepth[indexNode] = myDepth[indexNode-1] - Thickness ;
    }

    myResult = soilFluxes3D::initialize(NodesNumber, (short) NodesNumber, 0, computeWater, computeHeat, computeSolutes);
    if (myResult != CRIT3D_OK) printf("\n error in initialize");

    if (computeHeat) soilFluxes3D::initializeHeat(SAVE_HEATFLUXES_ALL, computeAdvection, computeLatent);

    if (! initializeSoil(useInputSoils)) printf("\n error in setSoilProperties");
    soilFluxes3D::setHydraulicProperties(MODIFIEDVANGENUCHTEN, MEAN_LOGARITHMIC, 10.);
    soilFluxes3D::setNumericalParameters((float)0.1, 600., 100, 10, 12, 6);

    for (indexNode = 0 ; indexNode<NodesNumber ; indexNode++ )
    {
        // elemento superficiale
        if (indexNode == 0)
        {
            myResult = soilFluxes3D::setNode(indexNode, x, y, myDepth[indexNode], mySurface, true, false, BOUNDARY_NONE, 0.0);
            if (myResult != CRIT3D_OK) printf("\n error in setNode!");

            myResult = soilFluxes3D::setNodeSurface(0, 0) ;
            if (myResult != CRIT3D_OK) printf("\n error in setNodeSurface!");

            if (computeWater)
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
                if (computeHeat)
                    boundaryType = BOUNDARY_HEAT_SURFACE;
                else
                    boundaryType = BOUNDARY_NONE;

                myResult = soilFluxes3D::setNode(indexNode, x, y, myDepth[indexNode], myThickness[indexNode] * mySurface, false, true, boundaryType, 0.0);
                if (myResult != CRIT3D_OK) printf("\n error in setNode!");
            }
            else if (indexNode == NodesNumber - 1)
            {
                myResult = soilFluxes3D::setNode(indexNode, x ,y, myDepth[indexNode], myThickness[indexNode] * mySurface, false, true, BOUNDARY_FREEDRAINAGE, 0.0);
                if (myResult != CRIT3D_OK) printf("\n error in setNode!");

                if (computeHeat)
                {
                    myResult = soilFluxes3D::setFixedTemperature(indexNode, 273.16 + bottomTemperature, bottomTemperatureDepth);
                    if (myResult != CRIT3D_OK) printf("\n error in setFixedTemperature!");
                }
            }
            else
            {
                myResult = soilFluxes3D::setNode(indexNode, x, y, myDepth[indexNode], myThickness[indexNode] * mySurface, false, false, BOUNDARY_NONE, 0.0);																		   
                if (myResult != CRIT3D_OK) printf("\n error in setNode!");
			}											  
									  
            myResult = soilFluxes3D::setNodeLink(indexNode, indexNode - 1 , UP, mySurface);	
            if (myResult != CRIT3D_OK) printf("\n error in setNodeLink!");

            if (useInputSoils)
            {
                if (myDepth[indexNode]<myInputSoils[myNodeHorizon].profInf)
                    if (myNodeHorizon < myHorizonNumber-1)
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
                myThetaS = ThetaS;
                myThetaR = ThetaR;
            }

            myResult = soilFluxes3D::setWaterContent(indexNode, ((indexNode-1)*(initialSaturationBottom - initialSaturationTop)/(NodesNumber-2)+initialSaturationTop) * (myThetaS - myThetaR) + myThetaR);
            if (myResult != CRIT3D_OK) printf("\n error in SetWaterContent!");

			if (computeHeat)
			{
				myResult = soilFluxes3D::setTemperature(indexNode,
                    273.16 + ((indexNode-1)*(initialTemperatureBottom - initialTemperatureTop)/(NodesNumber-2)+initialTemperatureTop));

				if (myResult != CRIT3D_OK) printf("\n error in SetTemperature!");
			}
        }

		// tutti tranne ultimo nodo confinano con nodo sotto
		if (indexNode < NodesNumber - 1)
		{
			myResult = soilFluxes3D::setNodeLink(indexNode, indexNode + 1 , DOWN, mySurface);
            if (myResult != CRIT3D_OK) printf("\n error in SetNode sotto!");
		}																 
    }

    soilFluxes3D::initializeBalance();

    *myHourIni = SimulationStart;
    *myHourFin = SimulationStop;

	return (true);
}

double getCurrentPrec(long myHour)
{ 
        if ((myHour >= precIniHour) && myHour < precIniHour + precHours)
                return (precHourlyAmount);
	else
		return (0.);
}

void setHour(long myHour)
{   CurrentHour = myHour;}

void setSinkSources(double myHourlyPrec)
{
    for (long i=0; i<NodesNumber; i++)
    {
        if (computeHeat && i > 0) soilFluxes3D::setHeatSinkSource(i, 0);

        if (computeWater)
        {
            if (i == 0)
                soilFluxes3D::setWaterSinkSource(i, (double)myHourlyPrec * mySurface / 3.6e06);
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

void getHourlyOutputAllPeriod(long firstIndex, long lastIndex, Crit3DOut *output)
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
        if (isValid(fluxTot)) fluxTot /= mySurface;
        else fluxTot = NODATA;
        myPoint.setY(fluxTot);
        output->profileOutput[output->nrValues-1].totalHeatFlux.push_back(myPoint);

        fluxDiff = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_DIFFUSIVE);
        if (isValid(fluxDiff)) fluxDiff /= mySurface;
        else fluxDiff = NODATA;

        fluxLtntIso = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_LATENT_ISOTHERMAL);
        if (isValid(fluxLtntIso)) fluxLtntIso /= mySurface;
        else fluxLtntIso = NODATA;

        fluxLtntTh = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_LATENT_THERMAL);
        if (isValid(fluxLtntTh)) fluxLtntTh /= mySurface;
        else fluxLtntTh = NODATA;

        fluxAdv = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_ADVECTIVE);
        if (isValid(fluxAdv)) fluxAdv /= mySurface;
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
    myPoint.setY(myValue);
    output->landSurfaceOutput[output->nrValues-1].netRadiation = myPoint;

    // sensible heat (positive upward)
    myValue = -soilFluxes3D::getBoundarySensibleFlux(1);
    myPoint.setY(myValue);
    output->landSurfaceOutput[output->nrValues-1].sensibleHeat = myPoint;

    // latent heat (positive upward)
    myValue = -soilFluxes3D::getBoundaryLatentFlux(1);
    myPoint.setY(myValue);
    output->landSurfaceOutput[output->nrValues-1].latentHeat = myPoint;

    //aerodynamic resistance
    myValue = soilFluxes3D::getBoundaryAerodynamicConductance(1);
    myPoint.setY(1./myValue);
    output->landSurfaceOutput[output->nrValues-1].aeroResistance = myPoint;

    //soil surface resistance
    myValue = soilFluxes3D::getBoundarySoilConductance(1);
    myPoint.setY(1./myValue);
    output->landSurfaceOutput[output->nrValues-1].soilResistance = myPoint;

    //errors
    myValue = soilFluxes3D::getHeatMBR();
    myPoint.setY(myValue);
    output->errorOutput[output->nrValues-1].heatMBR = myPoint;

    myValue = soilFluxes3D::getWaterMBR();
    myPoint.setY(myValue);
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
                 double myHourlyPrec)
{
    //double currentRoughness;
    //double surfaceWaterHeight;
    //double roughnessWater = 0.005;

    setSinkSources(myHourlyPrec);

    if (computeHeat)
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

        soilFluxes3D::setHeatBoundaryRoughness(1, RoughnessHeat);
    }

    soilFluxes3D::computePeriod(HOUR_SECONDS);

	return (true);
}


