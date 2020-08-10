#ifndef GRAPHFUNCTIONS_H
#define GRAPHFUNCTIONS_H

#include <QString>
#include <QVector>
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
    errorBalance = 13
};

enum outputVar
{
    surfaceNetIrradiance = 0,
    surfaceSensibleHeat = 1,
    surfaceLatentHeat = 2,
    aerodynamicResistence = 3,
    soilSurfaceResistence = 4,
    MBR_heat = 5,
    MBR_water = 6
};

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
};

enum outputType
{
    profile = 0,
    single = 1
};

struct profileStatus{
    QVector<QPointF> temperature;
    QVector<QPointF> waterContent;
    QVector<QPointF> heatConductivity;
    QVector<QPointF> totalHeatFlux;
    QVector<QPointF> diffusiveHeatFlux;
    QVector<QPointF> isothermalLatentHeatFlux;
    QVector<QPointF> thermalLatentHeatFlux;
    QVector<QPointF> advectiveheatFlux;
    QVector<QPointF> waterIsothermalLiquidFlux;
    QVector<QPointF> waterThermalLiquidFlux;
    QVector<QPointF> waterIsothermalVaporFlux;
    QVector<QPointF> waterThermalVaporFlux;
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

    QString getTextOutput(outputGroup myOutGroup);
};

void setColorScale(Crit3DColorScale* myColorScale, outputGroup outGroup, Crit3DOut *myOut, bool *graphLinear);
QVector<QPointF> getSingleSeries(Crit3DOut* myOut, outputVar myVar, float* MINVALUE, float* MAXVALUE);
QVector<QPointF> getProfileSeries(Crit3DOut* myOut, outputGroup myVar, int layerIndex, float* MINVALUE, float* MAXVALUE);



#endif // GRAPHFUNCTIONS_H
