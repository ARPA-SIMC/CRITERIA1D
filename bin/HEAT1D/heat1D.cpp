#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <iostream>
#include <qglobal.h>
#include <functional>
#include <vector>

#include "commonConstants.h"
#include "basicMath.h"
#include "soilFluxes3D.h"
#include "heat1D.h"

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
}


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

void setBottomTemperature(double myValue, double depth)
{
    myHeat1D.bottomTemperature = myValue;
    myHeat1D.bottomTemperatureDepth = depth;
}

void setWaterTable(bool isWaterTable, double depth)
{
    myHeat1D.isWaterTable = isWaterTable;
    myHeat1D.waterTableDepth = depth;
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


using namespace soilFluxes3D;

void setSurface(double myArea, double myRoughness, double minWaterRunoff, double myRoughnessHeat)
{
    myHeat1D.surfaceArea = myArea;
    myHeat1D.Roughness = myRoughness;
    myHeat1D.Plough = minWaterRunoff;
    myHeat1D.RoughnessHeat = myRoughnessHeat;

    SF3Derror_t result = soilFluxes3D::setSurfaceProperties(0, myHeat1D.Roughness);
    if (result != SF3Derror_t::SF3Dok)
        printf("\n Error in SetSurfaceProperties!");
}


bool initializeSoil(bool useInputSoils)
{
    SF3Derror_t result = SF3Derror_t::SF3Dok;

    if (useInputSoils)
    {
        for (int mySoilIndex = 0; mySoilIndex < myHeat1D.myHorizonNumber; mySoilIndex++)
            result = soilFluxes3D::setSoilProperties(0, mySoilIndex,
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
    {
        // default: loam
        double VG_he        = 0.23;     // m
        double VG_alpha     = 1.60;     // m-1
        double VG_n         = 1.25;
        double mualemTort   = 0.5;
        double KSat         = 0.4 * 0.01 / HOUR_SECONDS;  // [cm h-1] -> [m s-1]

        result = soilFluxes3D::setSoilProperties(0, 1, VG_alpha, VG_n, (1. - 1. / VG_n), VG_he,
                                                 myHeat1D.ThetaR, myHeat1D.ThetaS, KSat, mualemTort,
                                                 myHeat1D.OrganicMatter/100., myHeat1D.Clay/100.);
    }

    if (result != SF3Derror_t::SF3Dok)
    {
        printf("\n Error in SetSoilProperties");
        return false;
    }

    return true;
}


bool initializeHeat1D(bool useInputSoils)
{
    float x = 0.;
    float y = 0.;
    double myThetaS, myThetaR;

    myHeat1D.NodesNumber = ceil(myHeat1D.TotalDepth / myHeat1D.Thickness) + 1;

    std::vector<double>depth, thickness;
    depth.resize(myHeat1D.NodesNumber);
    thickness.resize(myHeat1D.NodesNumber);

    thickness[0] = 0;
    depth[0] = 0.;

    int nodeHorizon = 0;
    for (long indexNode = 1 ; indexNode < myHeat1D.NodesNumber ; indexNode++ )
    {
        thickness[indexNode] = myHeat1D.Thickness;
        if (indexNode == 1) depth[indexNode] = depth[indexNode-1] - (myHeat1D.Thickness / 2) ;
        else  depth[indexNode] = depth[indexNode-1] - myHeat1D.Thickness ;
    }

    unsigned int nrSurfaceNodes = 1;
    unsigned char nrLateralLinks = 0;
    SF3Derror_t result = soilFluxes3D::initializeSF3D(myHeat1D.NodesNumber, nrSurfaceNodes, nrLateralLinks,
                                      myHeat1D.computeWater, myHeat1D.computeHeat, myHeat1D.computeSolutes, heatFluxSaveMode_t::All);
    if (result != SF3Derror_t::SF3Dok)
        printf("\n error in initialize");

    if (myHeat1D.computeHeat)
        soilFluxes3D::initializeHeatFlag(heatFluxSaveMode_t::All, myHeat1D.computeAdvection, myHeat1D.computeLatent);

    if (! initializeSoil(useInputSoils))
        printf("\n error in setSoilProperties");

    soilFluxes3D::setHydraulicProperties(WRCModel::VanGenuchten, meanType_t::Logarithmic, 4.);
    soilFluxes3D::setNumericalParameters(0.1, HOUR_SECONDS, 200, 10, 12, 4);
    soilFluxes3D::setThreadsNumber(1);

    boundaryType_t boundaryType;
    for (long indexNode = 0 ; indexNode < myHeat1D.NodesNumber ; indexNode++ )
    {
        // surface
        if (indexNode == 0)
        {
            result = soilFluxes3D::setNode(indexNode, x, y, depth[indexNode], myHeat1D.surfaceArea,
                                           true, boundaryType_t::NoBoundary, 0, 0);
            if (result != SF3Derror_t::SF3Dok)
                printf("\n error in setNode!");

            result = soilFluxes3D::setNodeSurface(0, 0) ;
            if (result != SF3Derror_t::SF3Dok)
                printf("\n error in setNodeSurface!");

            result = soilFluxes3D::setNodePond(0, myHeat1D.Plough);
            if (result != SF3Derror_t::SF3Dok)
                printf("\nError in setNodePond!");

            if (myHeat1D.computeWater)
            {
                result = soilFluxes3D::setNodeWaterContent(indexNode, 0.);
                if (result != SF3Derror_t::SF3Dok)
                    printf("\n error in setWaterContent!");
            }
        }

        // sub-surface
        else
        {
            if (indexNode == 1)
            {
                if (myHeat1D.computeHeat)
                    boundaryType = boundaryType_t::HeatSurface;
                else
                    boundaryType = boundaryType_t::NoBoundary;

                result = soilFluxes3D::setNode(indexNode, x, y, depth[indexNode], thickness[indexNode] * myHeat1D.surfaceArea,
                                               false, boundaryType, 0, myHeat1D.surfaceArea);
                if (result != SF3Derror_t::SF3Dok)
                    printf("\n error in setNode!");
            }
            else if (indexNode == myHeat1D.NodesNumber - 1)
            {
                // last element
                if (myHeat1D.isWaterTable)
                {
                    // water table
                    result = soilFluxes3D::setNode(indexNode, x ,y, depth[indexNode], thickness[indexNode] * myHeat1D.surfaceArea,
                                                   false, boundaryType_t::PrescribedTotalWaterPotential, 0, myHeat1D.surfaceArea);

                    soilFluxes3D::setNodePrescribedTotalPotential(indexNode, -myHeat1D.waterTableDepth);
                }
                else
                {
                    // free drainage
                    result = soilFluxes3D::setNode(indexNode, x ,y, depth[indexNode], thickness[indexNode] * myHeat1D.surfaceArea,
                                               false, boundaryType_t::FreeDrainage, 0, myHeat1D.surfaceArea);
                }
                if (result != SF3Derror_t::SF3Dok)
                    printf("\n error in setNode!");

                if (myHeat1D.computeHeat)
                {
                    result = soilFluxes3D::setNodeBoundaryFixedTemperature(indexNode, 273.16 + myHeat1D.bottomTemperature,
                                                                           myHeat1D.bottomTemperatureDepth);
                    if (result != SF3Derror_t::SF3Dok)
                        printf("\n error in setNodeBoundaryFixedTemperature!");
                }
            }
            else
            {
                result = soilFluxes3D::setNode(indexNode, x, y, depth[indexNode], thickness[indexNode] * myHeat1D.surfaceArea,
                                               false, boundaryType_t::NoBoundary, 0, 0);
                if (result != SF3Derror_t::SF3Dok)
                    printf("\n error in setNode!");
			}											  
									  
            result = soilFluxes3D::setNodeLink(indexNode, indexNode - 1 , linkType_t::Up, myHeat1D.surfaceArea);
            if (result != SF3Derror_t::SF3Dok)
                printf("\n error in setNodeLink!");

            if (useInputSoils)
            {
                if (depth[indexNode] < myInputSoils[nodeHorizon].profInf)
                    if (nodeHorizon < myHeat1D.myHorizonNumber-1)
                        nodeHorizon++;
            }																									
            else
                nodeHorizon = 1;

            result = soilFluxes3D::setNodeSoil(indexNode, 0, nodeHorizon);
            if (result != SF3Derror_t::SF3Dok)
                printf("\n error in setNodeSoil!");
																																  
            if (useInputSoils)
            {
                myThetaS = myInputSoils[nodeHorizon].Theta_s;
                myThetaR = myInputSoils[nodeHorizon].Theta_r;
            }
            else
            {
                myThetaS = myHeat1D.ThetaS;
                myThetaR = myHeat1D.ThetaR;
            }

            result = soilFluxes3D::setNodeWaterContent(indexNode, ((indexNode-1)*(myHeat1D.initialSaturationBottom - myHeat1D.initialSaturationTop)
                                                                   / (myHeat1D.NodesNumber-2)+myHeat1D.initialSaturationTop) * (myThetaS - myThetaR) + myThetaR);
            if (result != SF3Derror_t::SF3Dok)
                printf("\n error in setNodeWaterContent!");

            if (myHeat1D.computeHeat)
			{
                result = soilFluxes3D::setNodeTemperature(indexNode, 273.16 + ((indexNode-1) *(myHeat1D.initialTemperatureBottom - myHeat1D.initialTemperatureTop)
                                                                    / (myHeat1D.NodesNumber-2) + myHeat1D.initialTemperatureTop));
                if (result != SF3Derror_t::SF3Dok)
                    printf("\n error in setNodeTemperature!");
			}
        }

        // all elements, except last, are connected with the one below
        if (indexNode < myHeat1D.NodesNumber - 1)
		{
            result = soilFluxes3D::setNodeLink(indexNode, indexNode + 1 , linkType_t::Down, myHeat1D.surfaceArea);
            if (result != SF3Derror_t::SF3Dok)
                printf("\n error in setNodeLink down!");
		}																 
    }

    soilFluxes3D::initializeBalance();
    std::cout << "Criteria3D model initialized." << std::endl;

    return true;
}


void setSinkSources(double myHourlyPrec)
{
    for (long i=0; i < myHeat1D.NodesNumber; i++)
    {
        if (myHeat1D.computeHeat && i > 0)
            soilFluxes3D::setNodeHeatSinkSource(i, 0);

        if (myHeat1D.computeWater)
        {
            if (i == 0)
            {
                double waterSinkSource = myHourlyPrec * 0.001 * myHeat1D.surfaceArea / HOUR_SECONDS;  // [m3 s-1]
                soilFluxes3D::setNodeWaterSinkSource(i, waterSinkSource);
            }
            else
            {
                soilFluxes3D::setNodeWaterSinkSource(i, 0);
            }
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
    return ( !isEqual(myValue, MEMORY_ERROR)
            && !isEqual(myValue, MISSING_DATA_ERROR)
            && !isEqual(myValue, INDEX_ERROR) );
}

void getOutputAllPeriod(long firstIndex, long lastIndex, Crit3DOut *output, double timeH)
{
    double fluxDiff, fluxLtntIso, fluxLtntTh, fluxAdv, fluxTot;
    double watFluxIsoLiq, watFluxThLiq, watFluxIsoVap, watFluxThVap;

    QPointF myPoint;
    profileStatus myProfile;
    landSurfaceStatus mySurfaceOutput;
    heatErrors myErrors;
    bottomFlux myBottomFluxes;
    waterStored myWaterStored;

    output->nrValues++;
    output->profileOutput.push_back(myProfile);
    output->landSurfaceOutput.push_back(mySurfaceOutput);
    output->errorOutput.push_back(myErrors);
    output->bottomFluxes.push_back(myBottomFluxes);
    output->waterStorageOutput.push_back(myWaterStored);

    for (long myIndex = firstIndex ; myIndex <= lastIndex ; myIndex++ )
    {
        myPoint.setX(timeH);

        double myValue = soilFluxes3D::getNodeTemperature(myIndex);
        if (isValid(myValue))
            myValue -= 273.16;
        else
            myValue = NODATA;
        myPoint.setY(myValue);
        output->profileOutput[output->nrValues-1].temperature.push_back(myPoint);

        myValue = soilFluxes3D::getNodeWaterContent(myIndex);
        if (! isValid(myValue))
            myValue = NODATA;
        myPoint.setY(myValue);
        output->profileOutput[output->nrValues-1].waterContent.push_back(myPoint);

        myValue = soilFluxes3D::getNodeHeatConductivity(myIndex);
        if (! isValid(myValue))
            myValue = NODATA;
        myPoint.setY(myValue);
        output->profileOutput[output->nrValues-1].heatConductivity.push_back(myPoint);

        fluxTot = soilFluxes3D::getNodeHeatMaxFlux(myIndex, linkType_t::Down, fluxTypes_t::HeatTotal);
        if (isValid(fluxTot))
            fluxTot /= myHeat1D.surfaceArea;
        else
            fluxTot = NODATA;
        myPoint.setY(fluxTot);
        output->profileOutput[output->nrValues-1].totalHeatFlux.push_back(myPoint);

        fluxDiff = soilFluxes3D::getNodeHeatMaxFlux(myIndex, linkType_t::Down, fluxTypes_t::HeatDiffusive);
        if (isValid(fluxDiff))
            fluxDiff /= myHeat1D.surfaceArea;
        else
            fluxDiff = NODATA;

        fluxLtntIso = soilFluxes3D::getNodeHeatMaxFlux(myIndex, linkType_t::Down, fluxTypes_t::HeatLatentIsothermal);
        if (isValid(fluxLtntIso))
            fluxLtntIso /= myHeat1D.surfaceArea;
        else
            fluxLtntIso = NODATA;

        fluxLtntTh = soilFluxes3D::getNodeHeatMaxFlux(myIndex, linkType_t::Down, fluxTypes_t::HeatLatentThermal);
        if (isValid(fluxLtntTh))
            fluxLtntTh /= myHeat1D.surfaceArea;
        else
            fluxLtntTh = NODATA;

        fluxAdv = soilFluxes3D::getNodeHeatMaxFlux(myIndex, linkType_t::Down, fluxTypes_t::HeatAdvective);
        if (isValid(fluxAdv))
            fluxAdv /= myHeat1D.surfaceArea;
        else
            fluxAdv = NODATA;

        myPoint.setY(fluxDiff);
        output->profileOutput[output->nrValues-1].diffusiveHeatFlux.push_back(myPoint);

        myPoint.setY(fluxLtntIso);
        output->profileOutput[output->nrValues-1].isothermalLatentHeatFlux.push_back(myPoint);

        myPoint.setY(fluxLtntTh);
        output->profileOutput[output->nrValues-1].thermalLatentHeatFlux.push_back(myPoint);

        myPoint.setY(fluxAdv);
        output->profileOutput[output->nrValues-1].advectiveheatFlux.push_back(myPoint);

        watFluxIsoLiq = soilFluxes3D::getNodeHeatMaxFlux(myIndex, linkType_t::Down, fluxTypes_t::WaterLiquidIsothermal);
        if (isValid(watFluxIsoLiq))
            watFluxIsoLiq *= 1000.;
        else watFluxIsoLiq = NODATA;

        watFluxThLiq  = soilFluxes3D::getNodeHeatMaxFlux(myIndex, linkType_t::Down, fluxTypes_t::WaterLiquidThermal);
        if (isValid(watFluxThLiq))
            watFluxThLiq *= 1000.;
        else
            watFluxThLiq = NODATA;

        watFluxIsoVap = soilFluxes3D::getNodeHeatMaxFlux(myIndex, linkType_t::Down, fluxTypes_t::WaterVaporIsothermal);
        if (isValid(watFluxIsoVap))
            watFluxIsoVap *= 1000. / WATER_DENSITY;
        else
            watFluxIsoVap = NODATA;

        watFluxThVap = soilFluxes3D::getNodeHeatMaxFlux(myIndex, linkType_t::Down, fluxTypes_t::WaterVaporThermal);
        if (isValid(watFluxThVap))
            watFluxThVap *= 1000. / WATER_DENSITY;
        else
            watFluxThVap = NODATA;

        myPoint.setY(watFluxIsoLiq);
        output->profileOutput[output->nrValues-1].waterIsothermalLiquidFlux.push_back(myPoint);

        myPoint.setY(watFluxThLiq);
        output->profileOutput[output->nrValues-1].waterThermalLiquidFlux.push_back(myPoint);

        myPoint.setY(watFluxIsoVap);
        output->profileOutput[output->nrValues-1].waterIsothermalVaporFlux.push_back(myPoint);

        myPoint.setY(watFluxThVap);
        output->profileOutput[output->nrValues-1].waterThermalVaporFlux.push_back(myPoint);
    }

    myPoint.setX(timeH);

    // net radiation (positive downward)
    double myValue = soilFluxes3D::getNodeBoundaryRadiativeFlux(1);
    if (isValid(myValue))
        myPoint.setY(myValue);
    else
        myPoint.setY(NODATA);
    output->landSurfaceOutput[output->nrValues-1].netRadiation = myPoint;

    // sensible heat (positive upward)
    myValue = soilFluxes3D::getNodeBoundarySensibleFlux(1);
    if (isValid(myValue))
        myPoint.setY(-myValue);
    else
        myPoint.setY(NODATA);
    output->landSurfaceOutput[output->nrValues-1].sensibleHeat = myPoint;

    // latent heat (positive upward)
    myValue = soilFluxes3D::getNodeBoundaryLatentFlux(1);
    if (isValid(myValue))
        myPoint.setY(-myValue);
    else
        myPoint.setY(NODATA);
    output->landSurfaceOutput[output->nrValues-1].latentHeat = myPoint;

    //aerodynamic resistance
    myValue = soilFluxes3D::getNodeBoundaryAerodynamicConductance(1);
    if (isValid(myValue) && myValue != 0.0)
        myPoint.setY(1. / myValue);
    else
        myPoint.setY(NODATA);
    output->landSurfaceOutput[output->nrValues-1].aeroResistance = myPoint;

    //soil surface resistance
    myValue = soilFluxes3D::getNodeBoundarySoilConductance(1);
    if (isValid(myValue) && myValue != 0.0)
        myPoint.setY(1. / myValue);
    else
        myPoint.setY(NODATA);
    output->landSurfaceOutput[output->nrValues-1].soilResistance = myPoint;

    //errors
    myValue = soilFluxes3D::getHeatMBR();
    if (isValid(myValue))
        myPoint.setY(myValue);
    else
        myPoint.setY(NODATA);
    output->errorOutput[output->nrValues-1].heatMBR = myPoint;

    myValue = soilFluxes3D::getWaterMBR();
    if (isValid(myValue))
        myPoint.setY(myValue);
    else
        myPoint.setY(NODATA);
    output->errorOutput[output->nrValues-1].waterMBR = myPoint;

    //bottom fluxes
    myValue = soilFluxes3D::getNodeBoundaryWaterFlow(lastIndex);
    if (isValid(myValue))
        myPoint.setY(myValue);
    else
        myPoint.setY(NODATA);
    output->bottomFluxes[output->nrValues-1].drainage = myPoint;

    //water storage
    myValue = soilFluxes3D::getWaterStorage();
    if (isValid(myValue))
        myPoint.setY(myValue);
    else
        myPoint.setY(NODATA);
    output->waterStorageOutput[output->nrValues-1].waterStord = myPoint;
}


namespace
{
using ProfileMember = std::vector<QPointF> profileStatus::*;

struct ProfileGroup
{
    const char* prefix;
    int firstLayer;         // 0 = includes the surface node
    ProfileMember member;
};

// returns false if the group is not a profile group
bool getProfileGroup(outputGroup group, ProfileGroup& pg)
{
    switch (group)
    {
    case outputGroup::soilTemperature:       pg = {"TempSoil_-",          1, &profileStatus::temperature};                 return true;
    case outputGroup::soilWater:             pg = {"WaterContent_-",      0, &profileStatus::waterContent};                return true;
    case outputGroup::totalHeatFlux:         pg = {"HeatFluxTot_-",       1, &profileStatus::totalHeatFlux};               return true;
    case outputGroup::diffusiveHeatFlux:     pg = {"HeatFluxDiff_-",      1, &profileStatus::diffusiveHeatFlux};           return true;
    case outputGroup::latentHeatFluxIso:     pg = {"HeatFluxLatIso_-",    1, &profileStatus::isothermalLatentHeatFlux};    return true;
    case outputGroup::latentHeatFluxTherm:   pg = {"HeatFluxLatTherm_-",  1, &profileStatus::thermalLatentHeatFlux};       return true;
    case outputGroup::waterIsothLiquidFlux:  pg = {"WaterFluxLiqIso_-",   1, &profileStatus::waterIsothermalLiquidFlux};   return true;
    case outputGroup::waterThermLiquidFlux:  pg = {"WaterFluxLiqTherm_-", 1, &profileStatus::waterThermalLiquidFlux};      return true;
    case outputGroup::waterIsothVaporFlux:   pg = {"WaterFluxVapIso_-",   1, &profileStatus::waterIsothermalVaporFlux};    return true;
    case outputGroup::waterThermVaporFlux:   pg = {"WaterFluxVapTherm_-", 1, &profileStatus::waterThermalVaporFlux};       return true;
    default:                                 return false;
    }
}
}


QString Crit3DOut::getTextOutput(outputGroup outGroup)
{
    QStringList header;
    std::vector<std::function<double(int)>> getters;   // one per column, argument = time index
    int decimals = 4;

    ProfileGroup pg;
    if (getProfileGroup(outGroup, pg))
    {
        for (int j = pg.firstLayer; j <= nrLayers; j++)
        {
            header << QString(pg.prefix) + QString::number(j * layerThickness);

            getters.push_back([this, pg, j](int i) -> double
                {
                  const std::vector<QPointF>& v = profileOutput[i].*(pg.member);
                  return (j < static_cast<int>(v.size())) ? v[j].y() : NODATA;
                });
        }
    }
    else
    {
        auto addColumn = [&](const QString& name, std::function<double(int)> getter)
        {
            header << name;
            getters.push_back(std::move(getter));
        };

        switch (outGroup)
        {
        case outputGroup::energyBalance:
            decimals = 2;
            addColumn("srfNetIrrad",  [this](int i) { return landSurfaceOutput[i].netRadiation.y(); });
            addColumn("srfSnsblHeat", [this](int i) { return landSurfaceOutput[i].sensibleHeat.y(); });
            addColumn("srfLtntHeat",  [this](int i) { return landSurfaceOutput[i].latentHeat.y(); });
            break;

        case outputGroup::surfaceResistances:
            decimals = 6;
            addColumn("aeroResistance", [this](int i) { return landSurfaceOutput[i].aeroResistance.y(); });
            addColumn("soilResistance", [this](int i) { return landSurfaceOutput[i].soilResistance.y(); });
            break;

        case outputGroup::errorBalance:
            decimals = 12;
            addColumn("HeatMBR",  [this](int i) { return errorOutput[i].heatMBR.y(); });
            addColumn("WaterMBR", [this](int i) { return errorOutput[i].waterMBR.y(); });
            break;

        case outputGroup::bottom:
            decimals = 8;
            addColumn("drainage", [this](int i) { return bottomFluxes[i].drainage.y(); });
            break;

        case outputGroup::waterStorage:
            decimals = 8;
            addColumn("waterStorage", [this](int i) { return waterStorageOutput[i].waterStord.y(); });
            break;

        default:
            break;
        }
    }

    QString text = header.join(',') + "\n";

    for (int i = 0; i < nrValues; i++)
    {
        QStringList row;
        for (const auto& getValue : getters)
            row << QString::number(getValue(i), 'f', decimals);

        text += row.join(',') + "\n";
    }

    return text;
}


bool runHeat1D(double hourlyTemperature,  double hourlyRelativeHumidity,
               double hourlyWindSpeed, double hourlyNetIrradiance,
               double hourlyPrec, int maxTimeStepSeconds)
{
    //double currentRoughness;
    //double surfaceWaterHeight;
    //double roughnessWater = 0.005;

    setSinkSources(hourlyPrec);

    if (myHeat1D.computeHeat)
    {
        soilFluxes3D::setNodeBoundaryHeightWind(1, 2.0);
        soilFluxes3D::setNodeBoundaryHeightTemperature(1, 1.5);
        soilFluxes3D::setNodeBoundaryTemperature(1, hourlyTemperature);
        soilFluxes3D::setNodeBoundaryRelativeHumidity(1, hourlyRelativeHumidity);
        soilFluxes3D::setNodeBoundaryWindSpeed(1, hourlyWindSpeed);
        soilFluxes3D::setNodeBoundaryNetIrradiance(1, hourlyNetIrradiance);

        /*
        surfaceWaterHeight = soilFluxes3D::getWaterContent(0);
        if (surfaceWaterHeight > RoughnessHeat)
            currentRoughness = 0.005;
        else if (surfaceWaterHeight > 0.)
            currentRoughness = (roughnessWater - RoughnessHeat) / RoughnessHeat * surfaceWaterHeight + RoughnessHeat;
        else
            currentRoughness = RoughnessHeat;
        */

        soilFluxes3D::setNodeBoundaryRoughness(1, myHeat1D.RoughnessHeat);
    }

    soilFluxes3D::computePeriod(maxTimeStepSeconds);

    return true;
}


