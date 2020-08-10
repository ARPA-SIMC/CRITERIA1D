#ifndef GRAPHS_H
#define GRAPHS_H

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include <qdatetime.h>

class QwtPlotCurve;
class QwtSymbol;
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
    MBE_heat = 6,
    MBR_water = 7
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
    QPointF heatMBE;
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

QVector<QPointF> getSingleSeries(Crit3DOut* myOut, outputVar myVar, float* MINVALUE, float* MAXVALUE);
void setColorScale(Crit3DColorScale* myColorScale, outputGroup outGroup, Crit3DOut *myOut);
////////////////////////////////////////////////////////////////////////////////
/// This class is a plot curve that can have gaps at points
/// with specific Y value.
/// This specific "gap" value can be passed to constructors,
/// it's zero by default.
class QwtPlotGappedCurve: public QwtPlotCurve
{
public:
    /// gapValue is an Y value which denotes missed data
    QwtPlotGappedCurve(const QString &title, double gapValue = 0);

    /// Override draw method to create gaps for missed Y values
    virtual void drawSeries(QPainter *painter, const QwtScaleMap &xMap,
                                                const QwtScaleMap &yMap, const QRectF &canvRect, int from, int to) const;

private:
    /// Value that denotes missed Y data at point
    double gapValue_;
};


class Plot : public QwtPlot
{
    Q_OBJECT

public:
    Plot( QWidget *parent = NULL );

    void drawOutput(outputGroup outGroup, Crit3DOut* myOut);
    void addCurve(QString myTitle, QwtPlotGappedCurve::CurveStyle myStyle, QPen myPen, QVector<QPointF> &samples, bool isSecondaryAxis);
    void drawSingle(outputGroup graphType, Crit3DOut* myOut, Crit3DColorScale myColorScale);
    void drawProfile(outputGroup graphType, Crit3DOut* myOut, Crit3DColorScale myColorScale);
};

#endif // GRAPHS_H
