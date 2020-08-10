#ifndef COLORMAPGRAPH_H
#define COLORMAPGRAPH_H

#include <QtWidgets>
#include "qcustomplot.h"
#include "color.h"
#include "graphFunctions.h"

class ColorMapGraph : public QWidget
{
    Q_OBJECT
public:
    ColorMapGraph();
    void draw(Crit3DOut* out, outputGroup graphType);
    void setProperties(outputGroup graphType);

private:
    QString title;
    QCustomPlot *graphic;
    QCPColorMap *colorMap;
    QCPColorScale *colorScale;
    QCPColorGradient gradient;
    Crit3DOut* myOut;
    int nx;
    int ny;

};

#endif // COLORMAP_H
