/*!
    \copyright 2018 Fausto Tomei, Gabriele Antolini,
    Alberto Pistocchi, Marco Bittelli, Antonio Volta, Laura Costantini

    This file is part of CRITERIA3D.
    CRITERIA3D has been developed under contract issued by ARPAE Emilia-Romagna

    CRITERIA3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CRITERIA3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CRITERIA3D.  If not, see <http://www.gnu.org/licenses/>.

    contacts:
    fausto.tomei@gmail.com
    ftomei@arpae.it
    gantolini@arpae.it
*/

#include <math.h>
#include <algorithm>

#include "commonConstants.h"
#include "water1D.h"
#include "soil.h"
#include "basicMath.h"
#include "crop.h"
#include "soilFluxes3D.h"


/*!
 * \brief Initialize soil water content
 * \param soilLayers
 */
bool initializeWater(std::vector<soil::Crit1DLayer> &soilLayers)
{
    if (soilLayers.empty())
        return false;

    // TODO water content as function of month
    double initialAW = 0.8;             /*!<  [-] fraction of available Water  */

    soilLayers[0].waterContent = 0.0;
    for (unsigned int i = 1; i < soilLayers.size(); i++)
    {
        soilLayers[i].waterContent = soil::getWaterContentFromAW(initialAW, soilLayers[i]);
    }

    return true;
}


/*!
 * \brief Water infiltration and redistribution 1D
 * \param soilLayers
 * \param inputWater        [mm] precipitation + irrigation
 * \param ploughedSoilDepth [m]  depth of the ploughed soil
 * \return drainage         [mm] deep drainage
 * \author Margot van Soetendael
 * \note P.M.Driessen, 1986, "The water balance of soil"
 * TODO extend to geometric soilLayers
 */
double computeInfiltration(std::vector<soil::Crit1DLayer> &soilLayers, double inputWater, double ploughedSoilDepth)
{
    if (soilLayers.empty())
        return NODATA;

    unsigned int reached;                   // [-] index of reached soilLayers for surpuls water
    double fluxLayer = NODATA;              // [mm]
    double residualFlux = NODATA;           // [mm]
    double localFlux = NODATA;              // [mm]
    double waterSurplus = NODATA;           // [mm]
    double waterDeficit = NODATA;           // [mm]
    double localWater = NODATA;             // [mm]
    double distrH2O = NODATA;               // [mm] la quantità di acqua (=imax dello strato sotto) che potrebbe saturare il profilo sotto lo strato in surplus
    double drainage = 0;                    // [mm]

    // assign precipitation (surface pond)
    soilLayers[0].waterContent += inputWater;
    soilLayers[0].flux = 0.0;
    soilLayers[0].maxInfiltration = 0.0;

    const unsigned nrLayers = unsigned(soilLayers.size());

    // No soil
    if (nrLayers == 1)
        return 0.0;

    // average degree of saturation (ploughed soil)
    unsigned int nrPloughLayers = 0;
    double avgPloughSatDegree = 0.0;

    for (unsigned int i = 1; i < nrLayers; i++)
    {
        soilLayers[i].flux = 0.0;
        soilLayers[i].maxInfiltration = 0.0;

        if (soilLayers[i].depth < ploughedSoilDepth)
        {
            nrPloughLayers++;
            avgPloughSatDegree += (soilLayers[i].waterContent / soilLayers[i].SAT);
        }
    }

    if (nrPloughLayers > 0)
        avgPloughSatDegree /= nrPloughLayers;
    else
        avgPloughSatDegree = 0.0;

    // maximum infiltration - due to gravitational force and permeability (Driessen 1986, eq.34)
    for (unsigned int i = 1; i < nrLayers; i++)
    {
        // [cm day-1] -> [mm day-1]
        soilLayers[i].maxInfiltration = 10 * soilLayers[i].horizonPtr->Driessen.gravConductivity;

        if (soilLayers[i].depth < ploughedSoilDepth)
        {
            // [cm day-1] -> [mm day-1]
            soilLayers[i].maxInfiltration += 10 * (1 - avgPloughSatDegree) * soilLayers[i].horizonPtr->Driessen.maxSorptivity;
        }
    }

    soilLayers[0].maxInfiltration = std::min(soilLayers[0].waterContent, soilLayers[1].maxInfiltration);
    soilLayers[0].flux = 0;

    for (int layerIndex = signed(nrLayers)-1; layerIndex >= 0; layerIndex--)
    {
        const unsigned int l = unsigned(layerIndex);

        // find soilLayers in water surplus
        if (soilLayers[l].waterContent > soilLayers[l].critical)
        {
            fluxLayer = std::min(soilLayers[l].maxInfiltration, soilLayers[l].waterContent - soilLayers[l].critical);
            soilLayers[l].flux += fluxLayer;
            soilLayers[l].waterContent -= fluxLayer;

            // find the arrival point of the wetting front
            // by virtually saturating the underlying profile with the Imax amount;
            // it considers the fluxes already passed through the underlying layers by taking the minimum;
            // each step subtracts the portion needed to saturate the layer
            if (l == (nrLayers-1))
                reached = l;
            else
            {
                distrH2O = soilLayers[l+1].maxInfiltration;
                unsigned int i = l+1;
                while ((i < nrLayers-1) && (distrH2O > 0.0))
                {
                    distrH2O -= (soilLayers[i].SAT - soilLayers[i].waterContent);
                    distrH2O = std::min(distrH2O, soilLayers[i].maxInfiltration);
                    if (distrH2O > 0.0) i++;
                }
                reached = i;
            }

            if ((l == reached) && (l < (nrLayers -1)))
                    reached += 1;

            while ((reached < (nrLayers -1))
                   && (soilLayers[reached].waterContent >= soilLayers[reached].SAT))
                    reached += 1;

            // move water and compute fluxes
            for (unsigned int i = l+1; i <= reached; i++)
            {
                // compute fluxLayer based on the water status of the underlying layer:
                // below Field Capacity, subtract the deficit from the fluxLayer;
                // in case of water surplus, add the surplus to the fluxLayer
                if (soilLayers[i].waterContent > soilLayers[i].critical)
                {
                    // layers above critical point (it depends on watertable depth: default is FC)
                    waterSurplus = soilLayers[i].waterContent - soilLayers[i].critical;
                    fluxLayer += waterSurplus;
                    soilLayers[i].waterContent -= waterSurplus;
                }
                else
                {
                    // layers before critical point
                    waterDeficit = soilLayers[i].critical - soilLayers[i].waterContent;
                    localWater = std::min(fluxLayer, waterDeficit);
                    fluxLayer -= localWater;
                    soilLayers[i].waterContent += localWater;
                    if (fluxLayer <= 0.0) break;
                }

                residualFlux = soilLayers[i].maxInfiltration - soilLayers[i].flux;
                residualFlux = std::max(residualFlux, 0.0);

                if (residualFlux >= fluxLayer)
                {
                    soilLayers[i].flux += fluxLayer;
                }
                else
                {
                    // local surplus (localflux)
                    localFlux = fluxLayer - residualFlux;
                    fluxLayer = residualFlux;
                    soilLayers[i].flux += fluxLayer;

                    // surplus management
                    if (localFlux <= (soilLayers[i].SAT - soilLayers[i].waterContent))
                    {
                        // there is available space for water in the layer
                        soilLayers[i].waterContent += localFlux;
                        localFlux = 0.0;
                    }
                    else
                    {
                        // not enough space for water, upper layers are involved
                        unsigned int j;
                        for (j = i; j >= l + 1; j--)
                        {
                            if (localFlux <= 0.0)
                                break;

                            localWater = std::min(localFlux, soilLayers[j].SAT - soilLayers[j].waterContent);
                            if (j < i)
                                soilLayers[j].flux -= localFlux;

                            localFlux -= localWater;
                            soilLayers[j].waterContent += localWater;
                        }

                        // residual water
                        if ((localFlux > 0.0) && (j == l))
                        {
                            soilLayers[l].waterContent += localFlux;
                            soilLayers[l].flux -= localFlux;
                        }
                    }
                }
            }

            // drainage
            if ((reached == nrLayers-1) && (fluxLayer > 0))
            {
                drainage += fluxLayer;
                fluxLayer = 0;
            }

            // surplus distribution (saturated layers)
            unsigned int i = reached;
            while ((fluxLayer > 0) && (i >= l+1))
            {
                localWater = std::min(fluxLayer, soilLayers[i].SAT - soilLayers[i].waterContent);
                fluxLayer -= localWater;
                soilLayers[i].flux -= localWater;
                soilLayers[i].waterContent += localWater;
                i--;
            }

            // first layer (pond on surface)
            if ((fluxLayer != 0.) && (i == l))
            {
                soilLayers[l].waterContent += fluxLayer;
                soilLayers[l].flux -= fluxLayer;
            }

        }
    }

    // update water potential
    for (unsigned i=1; i < nrLayers; i++)
    {
        soilLayers[i].waterPotential = soilLayers[i].getWaterPotential();           // [kPa]
    }

    return drainage;
}


/*!
 * \brief compute capillary rise due to watertable
 * and compute current threshold for vertical drainage (soilLayers[i].critical)
 * \param soilLayers
 * \param waterTableDepth [m] depth of the water table (NODATA: no water table)
 * \return capillary rise [mm]
 * \note depth of each layer is assigned at its center:
 *       layers with center below the water table are saturated
 * \note upward flux of each layer: k(psi) * (dPsi/dz - 1), reduced by REDUCTION_FACTOR
 *       to take into account the soil inertia
 */
double computeCapillaryRise(std::vector<soil::Crit1DLayer> &soilLayers, double waterTableDepth)
{
    // reduction of the upward flux (soil inertia). Set 1.0 to disable.
    const double REDUCTION_FACTOR = 0.5;

    const int nrLayers = int(soilLayers.size());

    // surface layer + at least one soil layer
    if (nrLayers < 2)
        return 0.0;

    const int lastLayer = nrLayers - 1;

    // no water table, wrong data or water table too deep
    if (isEqual(waterTableDepth, NODATA) || waterTableDepth <= 0.0
        || waterTableDepth > (soilLayers[lastLayer].depth + 5.0))
    {
        // re-initialize threshold for vertical drainage
        for (int i = 1; i <= lastLayer; i++)
            soilLayers[i].critical = soilLayers[i].FC;

        return 0.0;
    }

    double capillaryRiseSum = 0.0;                      // [mm]

    // boundary layer: last layer with the center above the water table
    // (0 means that all the soil layers are below the water table)
    int boundaryLayer = lastLayer;
    while (boundaryLayer > 0 && waterTableDepth <= soilLayers[boundaryLayer].depth)
        boundaryLayer--;

    // layers below the water table: saturated
    for (int i = boundaryLayer + 1; i <= lastLayer; i++)
    {
        soilLayers[i].critical = soilLayers[i].SAT;

        if (soilLayers[i].waterContent < soilLayers[i].SAT)
        {
            capillaryRiseSum += (soilLayers[i].SAT - soilLayers[i].waterContent);
            soilLayers[i].waterContent = soilLayers[i].SAT;
        }
    }

    // no unsaturated layers
    if (boundaryLayer == 0)
        return capillaryRiseSum;

    // air entry point of the boundary layer
    const double he_boundary = soilLayers[boundaryLayer].horizonPtr->vanGenuchten.he;       // [kPa]

    // above the water table: threshold for vertical drainage
    for (int i = 1; i <= boundaryLayer; i++)
    {
        const double dz = waterTableDepth - soilLayers[i].depth;                            // [m]
        const double psiThreshold = soil::metersTokPa(dz) + he_boundary;                    // [kPa]

        soilLayers[i].critical = soil::getWaterContentFromPsi(psiThreshold, soilLayers[i]);

        if (soilLayers[i].critical < soilLayers[i].FC)
        {
            soilLayers[i].critical = soilLayers[i].FC;
        }
    }

    // above the water table: capillary rise, from the boundary layer upwards.
    // Each layer is evaluated with its own suction and conductivity:
    // a layer in equilibrium does NOT stop the rise towards the drier layers above.
    double previousPsi = soilLayers[boundaryLayer].getWaterPotential();                     // [kPa]

    for (int i = boundaryLayer; i > 0; i--)
    {
        const double psi = soilLayers[i].getWaterPotential();                               // [kPa]

        // the layer above is wetter than the layer below: inverted gradient, the rise stops
        if (i < boundaryLayer && psi < previousPsi)
            break;

        const double dPsi = soil::kPaToMeters(psi - he_boundary);                           // [m]
        const double dz = std::max(waterTableDepth - soilLayers[i].depth, EPSILON);         // [m]

        double currentPsi = psi;

        // suction greater than the gravity head: upward flux
        if (dPsi > dz)
        {
            // [mm day-1]
            const double kPsi = soilLayers[i].getWaterConductivity() * 10.0 * REDUCTION_FACTOR;
            const double maxFlux = kPsi * ((dPsi / dz) - 1.0);

            // [mm]
            const double maxStorage = std::max(0.0, soilLayers[i].critical - soilLayers[i].waterContent);
            const double capillaryRise = std::min(maxFlux, maxStorage);

            if (capillaryRise > 0.0)
            {
                soilLayers[i].waterContent += capillaryRise;
                capillaryRiseSum += capillaryRise;

                currentPsi = soilLayers[i].getWaterPotential();
            }
        }

        previousPsi = currentPsi;
    }

    return capillaryRiseSum;
}


/*!
 * \brief compute capillary rise due to watertable
 * and compute current threshold for vertical drainage (soilLayers[i].critical)
 * \param soilLayers
 * \param waterTableDepth [m]
 * \return capillary rise
 */
double computeCapillaryRise_old(std::vector<soil::Crit1DLayer> &soilLayers, double waterTableDepth)
{
    double psi, previousPsi;             // [kPa] water potential
    double he_boundary;                  // [kPa] air entry point boundary soilLayers
    double k_psi;                        // [cm day-1] water conductivity

    double capillaryRiseSum = 0;         // [mm day-1]

    const double REDUCTION_FACTOR = 0.5;

    const unsigned nrLayers = unsigned(soilLayers.size());
    const int lastLayer = nrLayers-1;
    if (nrLayers == 0)
        return 0;

    // No WaterTable, wrong data or watertable too depth
    if ( isEqual(waterTableDepth, NODATA) || waterTableDepth <= 0
            || waterTableDepth > (soilLayers[lastLayer].depth + 5.0) )
    {
        // re-initialize threshold for vertical drainage
        for (unsigned int i = 1; i < nrLayers; i++)
            soilLayers[i].critical = soilLayers[i].FC;

        return 0;
    }

    // search boundary layer: first soil layer over watertable
    // depth is assigned at center of layer
    int boundaryLayer = lastLayer;
    while ((boundaryLayer > 1) && (waterTableDepth <= soilLayers[boundaryLayer].depth))
            boundaryLayer--;

    boundaryLayer = std::max(boundaryLayer, 1);

    // layer below watertable: saturated
    if (boundaryLayer < lastLayer)
    {
        for (int i = boundaryLayer+1; i <= lastLayer; i++)
        {
            soilLayers[i].critical = soilLayers[i].SAT;

            if (soilLayers[i].waterContent < soilLayers[i].SAT)
            {
                capillaryRiseSum += (soilLayers[i].SAT - soilLayers[i].waterContent);
                soilLayers[i].waterContent = soilLayers[i].SAT;
            }
        }
    }

    // air entry point of the boundary layer
    he_boundary = soilLayers[boundaryLayer].horizonPtr->vanGenuchten.he;       // [kPa]

    // above watertable: assign water content threshold for vertical drainage
    for (int i = 1; i <= boundaryLayer; i++)
    {
        const double dz = (waterTableDepth - soilLayers[i].depth);                       // [m]
        const double psi = soil::metersTokPa(dz) + he_boundary;                          // [kPa]

        soilLayers[i].critical = soil::getWaterContentFromPsi(psi, soilLayers[i]);

        if (soilLayers[i].critical < soilLayers[i].FC)
        {
            soilLayers[i].critical = soilLayers[i].FC;
        }
    }

    // above watertable: capillary rise
    previousPsi = soilLayers[boundaryLayer].getWaterPotential();
    for (int i = boundaryLayer; i > 0; i--)
    {
        psi = soilLayers[i].getWaterPotential();                        // [kPa]

        if (i < boundaryLayer && psi < previousPsi)
            break;

        const double dPsi = soil::kPaToMeters(psi - he_boundary);                    // [m]
        const double dz = std::max(EPSILON, waterTableDepth - soilLayers[i].depth);  // [m]

        if (dPsi > dz)
        {
            // [mm day-1]
            k_psi = soilLayers[i].getWaterConductivity() * 10.;
            // reduce capillary rise to consider the soil inertia
            k_psi *= REDUCTION_FACTOR;
            // [mm day-1]
            double capillaryRise = k_psi * ((dPsi / dz) - 1);
            // [mm day-1]
            const double maxCapillaryRise = std::max(0.0, soilLayers[i].critical - soilLayers[i].waterContent);

            capillaryRise = std::min(capillaryRise, maxCapillaryRise);

            // update water content
            soilLayers[i].waterContent += capillaryRise;
            capillaryRiseSum += capillaryRise;

            previousPsi = soilLayers[i].getWaterPotential();            // [kPa]
        }
        else
        {
            previousPsi = psi;
        }
    }

    return capillaryRiseSum;
}


double computeEvaporation(std::vector<soil::Crit1DLayer> &soilLayers, double maxEvaporation)
{
    if (soilLayers.empty())
        return NODATA;

    // surface evaporation
    const double surfaceEvaporation = std::min(maxEvaporation, soilLayers[0].waterContent);
    soilLayers[0].waterContent -= surfaceEvaporation;

    double actualEvaporation = surfaceEvaporation;
    double residualEvaporation = maxEvaporation - surfaceEvaporation;

    if (residualEvaporation < EPSILON || soilLayers.size() == 1)
    {
        // only surface evaporation
        return actualEvaporation;
    }

    // soil evaporation
    int nrEvapLayers = 1;
    while (nrEvapLayers < int(soilLayers.size()) - 1 &&
           (soilLayers[nrEvapLayers].depth + soilLayers[nrEvapLayers].thickness * 0.5) < MAX_EVAPORATION_DEPTH)
    {
        nrEvapLayers++;
    }

    std::vector<double> coeffEvap(nrEvapLayers, 0.0);

    const double minDepth = soilLayers[1].depth + soilLayers[1].thickness / 2;
    double sumCoeff = 0;
    for (int i=1; i <= nrEvapLayers; i++)
    {
        const double layerDepth = soilLayers[i].depth + soilLayers[i].thickness / 2.0;

        const double coeffDepth = std::max((layerDepth - minDepth) / (MAX_EVAPORATION_DEPTH - minDepth), 0.0);
        // evaporation coefficient: 1 at depthMin, ~0.1 at maximum depth for evaporation
        coeffEvap[i-1] = exp(-2 * coeffDepth);

        sumCoeff += coeffEvap[i-1];
    }

    // normalize
    std::vector<double> coeffThreshold(nrEvapLayers, 0.0);

    for (int i=0; i < nrEvapLayers; i++)
    {
        coeffThreshold[i] = (1.0 - coeffEvap[i]) * 0.5;
        coeffEvap[i] /= sumCoeff;
    }

    bool isWaterSupply = true;
    int nrIteration = 0;
    while ((residualEvaporation > EPSILON) && isWaterSupply && nrIteration < 3)
    {
        double sumEvap = 0.0;
        isWaterSupply = false;

        for (int i=1; i <= nrEvapLayers; ++i)
        {
            double layerEvaporation = residualEvaporation * coeffEvap[i-1];
            const double evapLayerThreshold = soilLayers[i].HH + (soilLayers[i].FC - soilLayers[i].HH) * coeffThreshold[i-1];

            if (soilLayers[i].waterContent > (evapLayerThreshold + layerEvaporation))
            {
                isWaterSupply = true;
            }
            else
            {
                if (soilLayers[i].waterContent > evapLayerThreshold)
                {
                    layerEvaporation = soilLayers[i].waterContent - evapLayerThreshold;
                }
                else
                    layerEvaporation = 0.0;
            }

            soilLayers[i].waterContent -= layerEvaporation;
            sumEvap += layerEvaporation;
        }

        residualEvaporation -= sumEvap;
        actualEvaporation  += sumEvap;
        nrIteration++;
    }

    return actualEvaporation;
}


/*!
 * \brief compute surface runoff [mm]
 */
double computeSurfaceRunoff(const Crit3DCrop &myCrop, std::vector<soil::Crit1DLayer> &soilLayers)
{
    double surfaceRunoff;           // [mm]
    double maxSurfaceWater;         // [mm]

    maxSurfaceWater = myCrop.getSurfaceWaterPonding();
    if (soilLayers[0].waterContent > maxSurfaceWater)
    {
       surfaceRunoff = soilLayers[0].waterContent - maxSurfaceWater;
       soilLayers[0].waterContent = maxSurfaceWater;
    }
    else
       surfaceRunoff = 0.0;

    return surfaceRunoff;
}


/*!
 * \brief Compute lateral drainage
 * \param soilLayers
 * \param fieldWidth       [m]
 * \note P.M.Driessen, 1986, eq.58
 * \return lateralDrainage [mm]
 */
double computeLateralDrainage(std::vector<soil::Crit1DLayer> &soilLayers, double fieldWidth)
{
    if (soilLayers.empty())
        return NODATA;

    const double drainRadius = 0.25;        // [m] radius of drain
    const double drainDepth = 1.0;          // [m] depth of drain

    unsigned nrLayers = unsigned(soilLayers.size());

    double lateralDrainageSum = 0;          // [mm]
    for (unsigned int i = 1; i < nrLayers; i++)
    {
        if (soilLayers[i].depth > drainDepth)
            break;

        const double waterSurplus = soilLayers[i].waterContent - soilLayers[i].critical;    // [mm]
        const double surplusRange = soilLayers[i].SAT - soilLayers[i].critical;             // [mm]
        if (waterSurplus > 0.0 && surplusRange > EPSILON)
        {
            const double satFactor = waterSurplus / surplusRange;                           // [-]

            const double hydrHead = satFactor * (drainDepth - soilLayers[i].depth);         // [m]

            const double maxDrainage = 10 * soilLayers[i].horizonPtr->Driessen.k0 * hydrHead /
                    (hydrHead + (fieldWidth / PI) * log(fieldWidth / (PI * drainRadius)));  // [mm]

            const double layerDrainage = std::min(waterSurplus, maxDrainage);               // [mm]

            soilLayers[i].waterContent -= layerDrainage;
            lateralDrainageSum += layerDrainage;
        }
    }

    return lateralDrainageSum;
}


/*!
 * \name assignOptimalIrrigation
 * \brief assign subirrigation to restore field capacity in the root zone
 * \param soilLayers, lastRootLayer, irrigationMax
 * \return irrigation [mm]
 */
double assignOptimalIrrigation(std::vector<soil::Crit1DLayer> &soilLayers, unsigned int lastRootLayer, double irrigationMax)
{
    const  unsigned int nrLayers = unsigned(soilLayers.size());

    // no soil
    if (nrLayers < 2)
        return 0.0;

    double residualIrrigation = irrigationMax;
    unsigned int i = 1;
    while ((i < nrLayers && i <= lastRootLayer) && residualIrrigation > 0)
    {
        if (soilLayers[i].waterContent < soilLayers[i].FC)
        {
            double deficit = soilLayers[i].FC - soilLayers[i].waterContent;   // [mm]
            deficit = std::min(deficit, residualIrrigation);

            soilLayers[i].waterContent += deficit;
            residualIrrigation -= deficit;
        }
        i++;
    }

    return (irrigationMax - residualIrrigation);
}


/*!
 * \brief getSoilWaterContentSum
 * \param soilLayers
 * \param computationDepth = computation soil depth [cm]
 * \return sum of water content from zero to computationDepth [mm]
 */
double getSoilWaterContentSum(const std::vector<soil::Crit1DLayer> &soilLayers, double computationDepth)
{
    computationDepth /= 100.0;              // [cm] --> [m]

    double waterContentSum = 0;             // [mm]

    for (unsigned int i = 1; i < soilLayers.size(); i++)
    {
        const double lowerDepth = soilLayers[i].depth + soilLayers[i].thickness * 0.5;              // [m]

        if (lowerDepth < computationDepth)
        {
            waterContentSum += soilLayers[i].waterContent;
        }
        else
        {
            const double upperDepth = soilLayers[i].depth - soilLayers[i].thickness * 0.5;          // [m]
            const double currentThick = std::max(0.0, computationDepth - upperDepth);               // [m]
            const double depthFraction = currentThick / soilLayers[i].thickness;                    // [-]
            return waterContentSum + soilLayers[i].waterContent * depthFraction;
        }
    }

    return waterContentSum;
}


/*!
 * \brief getReadilyAvailableWater
 * \param myCrop
 * \param soilLayers
 * \return sum of readily available water in the rooting zone [mm]
 */
double getReadilyAvailableWater(const Crit3DCrop &myCrop, const std::vector<soil::Crit1DLayer> &soilLayers)
{
    if (! myCrop.isLiving)
        return NODATA;
    if (myCrop.roots.rootDepth <= myCrop.roots.rootDepthMin)
        return NODATA;
    if (myCrop.roots.firstRootLayer == NODATA)
        return NODATA;

    double sumRAW = 0.0;
    for (unsigned int i = unsigned(myCrop.roots.firstRootLayer); i <= unsigned(myCrop.roots.lastRootLayer); i++)
    {
        const double thetaWP = soil::thetaFromSignPsi(-soil::cmTokPa(myCrop.psiLeaf), *(soilLayers[i].horizonPtr));
        // [mm]
        const double cropWP = thetaWP * soilLayers[i].thickness * soilLayers[i].soilFraction * 1000.;
        // [mm]
        const double threshold = soilLayers[i].FC - myCrop.fRAW * (soilLayers[i].FC - cropWP);
        // [mm]
        double layerRAW = std::max(0.0, soilLayers[i].waterContent - threshold);

        const double layerMaxDepth = soilLayers[i].depth + soilLayers[i].thickness / 2.0;
        if (myCrop.roots.rootDepth < layerMaxDepth)
        {
            const double layerMinDepth = soilLayers[i].depth - soilLayers[i].thickness / 2.0;
            layerRAW *= (myCrop.roots.rootDepth - layerMinDepth) / soilLayers[i].thickness;
        }

        sumRAW += layerRAW;
    }

    return sumRAW;
}

