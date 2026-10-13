#ifndef GRAPHFUNCTIONS_H
#define GRAPHFUNCTIONS_H

#include <vector>

#include <QString>
#include <QPointF>
#include <qdatetime.h>

class Crit3DColorScale;

enum outputGroup
{
    soilTemperature = 0,
    soilWater = 1,
    totalHeatFlux = 2,
    diffusiveHeatFlux = 3,
    latentHeatFluxIso = 4,
    latentHeatFluxTherm = 5,
    waterIsothLiquidFlux = 6,
    waterThermLiquidFlux = 7,
    waterIsothVaporFlux = 8,
    waterThermVaporFlux = 9,
    energyBalance = 10,
    surfaceResistances = 11,
    soilHeatConductivity = 12,
    bottom = 13,
    waterStorage = 14,
    errorBalance = 15

};

enum outputVar
{
    surfaceNetIrradiance = 0,
    surfaceSensibleHeat = 1,
    surfaceLatentHeat = 2,
    aerodynamicResistence = 3,
    soilSurfaceResistence = 4,
    MBR_heat = 5,
    MBR_water = 6,
    bottomDrainage = 7,
    storedWater = 8
};

/*
static const char * outputVarString[] =
{
  "net irradiance",
  "sensible heat",
  "latent heat",
  "aerodynamic resistence",
  "soil surface resistence",
  "MBR heat",
  "MBE heat",
  "MBR water"
};*/

enum outputType
{
    profile = 0,
    single = 1
};

struct profileStatus{
    std::vector<QPointF> temperature;
    std::vector<QPointF> waterContent;
    std::vector<QPointF> heatConductivity;
    std::vector<QPointF> totalHeatFlux;
    std::vector<QPointF> diffusiveHeatFlux;
    std::vector<QPointF> isothermalLatentHeatFlux;
    std::vector<QPointF> thermalLatentHeatFlux;
    std::vector<QPointF> advectiveheatFlux;
    std::vector<QPointF> waterIsothermalLiquidFlux;
    std::vector<QPointF> waterThermalLiquidFlux;
    std::vector<QPointF> waterIsothermalVaporFlux;
    std::vector<QPointF> waterThermalVaporFlux;
};

struct landSurfaceStatus{
    QPointF sensibleHeat;
    QPointF latentHeat;
    QPointF netRadiation;
    QPointF aeroResistance;
    QPointF soilResistance;
};

struct heatErrors{
    QDateTime myTime;
    QPointF heatMBR;
    QPointF waterMBR;
};

struct bottomFlux{
    QPointF drainage;
};

struct waterStored{
    QPointF waterStord;
};

class Crit3DOut
{
public:

    Crit3DOut();

    int nrValues;
    int nrLayers;

    float layerThickness;

    void clean();

    QVector<landSurfaceStatus> landSurfaceOutput;
    QVector<profileStatus> profileOutput;
    QVector<heatErrors> errorOutput;
    QVector<bottomFlux> bottomFluxes;
    QVector<waterStored> waterStorageOutput;

    QString getTextOutput(outputGroup myOutGroup);
};

void setColorScale(Crit3DColorScale* myColorScale, outputGroup outGroup, Crit3DOut *myOut, bool *graphLinear);
QVector<QPointF> getSingleSeries(Crit3DOut* myOut, outputVar myVar, float &minSeries, float &maxSeries);
QVector<QPointF> getProfileSeries(Crit3DOut* myOut, outputGroup myVar, int layerIndex, float &minSeries, float &maxSeries);



#endif // GRAPHFUNCTIONS_H
