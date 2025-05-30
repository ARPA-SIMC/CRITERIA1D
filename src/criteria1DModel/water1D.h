#ifndef WATER1D_H
#define WATER1D_H

    #include <vector>

    #ifndef SOIL_H
        #include "soil.h"
    #endif

    class Crit3DCrop;

    void initializeWater(std::vector<soil::Crit1DLayer> &soilLayers);

    double computeInfiltration(std::vector<soil::Crit1DLayer> &soilLayers, double inputWater, double ploughedSoilDepth);

    double computeEvaporation(std::vector<soil::Crit1DLayer> &soilLayers, double maxEvaporation);
    double computeSurfaceRunoff(const Crit3DCrop &myCrop, std::vector<soil::Crit1DLayer> &soilLayers);
    double computeLateralDrainage(std::vector<soil::Crit1DLayer> &soilLayers, double fieldWidth);
    double computeCapillaryRise(std::vector<soil::Crit1DLayer> &soilLayers, double waterTableDepth);

    double assignOptimalIrrigation(std::vector<soil::Crit1DLayer> &soilLayers, unsigned int lastRootLayer, double irrigationMax);

    double getSoilWaterContentSum(const std::vector<soil::Crit1DLayer> &soilLayers, double computationDepth);

    double getReadilyAvailableWater(const Crit3DCrop &myCrop, const std::vector<soil::Crit1DLayer> &soilLayers);
    
    // test

#endif // WATER1D_H

