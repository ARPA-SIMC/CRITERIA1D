/*!
    \file crop.cpp

    \abstract
    Crop class functions

    \authors
    Fausto Tomei        ftomei@arpae.it
    Gabriele Antolini   gantolini@arpe.it
    Antonio Volta       avolta@arpae.it

    \copyright
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
*/


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <algorithm>

#include "crit3dDate.h"
#include "commonConstants.h"
#include "crop.h"
#include "root.h"
#include "development.h"


Crit3DCrop::Crit3DCrop()
    : idCrop{""}
{
    type = HERBACEOUS_ANNUAL;

    /*!
     * \brief crop cycle
     */
    isLiving = false;
    isEmerged = false;
    sowingDoy = NODATA;
    currentSowingDoy = NODATA;
    doyStartSenescence = NODATA;
    plantCycle = NODATA;
    LAImin = NODATA;
    LAImax = NODATA;
    LAIgrass = NODATA;
    LAIcurve_a = NODATA;
    LAIcurve_b = NODATA;
    LAIstartSenescence = NODATA;
    thermalThreshold = NODATA;
    upperThermalThreshold = NODATA;
    degreeDaysIncrease = NODATA;
    degreeDaysDecrease = NODATA;
    degreeDaysEmergence = NODATA;

    /*!
     * \brief water need
     */
    kcMax  = NODATA;
    psiLeaf = NODATA;
    stressTolerance = NODATA;
    fRAW = NODATA;

    /*!
     * \brief irrigation
     */
    irrigationShift = NODATA;
    irrigationVolume = NODATA;
    degreeDaysStartIrrigation = NODATA;
    degreeDaysEndIrrigation = NODATA;
    doyStartIrrigation = NODATA;
    doyEndIrrigation = NODATA;
    maxSurfacePuddle = NODATA;
    daysSinceIrrigation = NODATA;

    degreeDays = NODATA;
    LAI = NODATA;
}


void Crit3DCrop::initialize( double latitude, unsigned int nrLayers, double totalSoilDepth, int currentDoy)
{
    // initialize root density
    if (roots.rootDensity != nullptr) delete[] roots.rootDensity;
    roots.rootDensity = new double[nrLayers];

    // initialize root depth
    roots.rootDepth = 0;
    if (roots.rootDepthMax > totalSoilDepth)
        roots.rootDepthMax = totalSoilDepth;

    degreeDays = 0;

    if (latitude > 0)
        doyStartSenescence = 305;
    else
        doyStartSenescence = 120;

    LAIstartSenescence = NODATA;
    currentSowingDoy = NODATA;

    daysSinceIrrigation = NODATA;

    // is crop living?
    if (isPluriannual())
        isLiving = true;
    else
    {
        isLiving = isInsideTypicalCycle(currentDoy);

        if (isLiving == true)
            currentSowingDoy = sowingDoy;
    }

    resetCrop(nrLayers);
}


bool Crit3DCrop::updateLAI(double latitude, unsigned int nrLayers, int myDoy)
{
    double degreeDaysLai = 0;
    double myLai = 0;

    if (! isPluriannual())
    {
        if (! isEmerged)
        {
            if (degreeDays < degreeDaysEmergence)
                return true;
            else if (myDoy - sowingDoy >= MIN_EMERGENCE_DAYS)
            {
                isEmerged = true;
                degreeDaysLai = degreeDays - degreeDaysEmergence;
            }
            else
                return true;
        }
        else
        {
            degreeDaysLai = degreeDays - degreeDaysEmergence;
        }

        if (degreeDaysLai > 0)
            myLai = leafDevelopment::getLAICriteria(this, degreeDaysLai);
    }
    else
    {
        if (type == GRASS)
            // grass cut
            if (degreeDays >= degreeDaysIncrease)
                resetCrop(nrLayers);

        if (degreeDays > 0)
            myLai = leafDevelopment::getLAICriteria(this, degreeDays);
        else
            myLai = LAImin;

        if (type == FRUIT_TREE)
        {
            bool isLeafFall;
            if (latitude > 0)   // north
            {
                isLeafFall = (myDoy >= doyStartSenescence);
            }
            else                // south
            {
                isLeafFall = ((myDoy >= doyStartSenescence) && (myDoy < 182));
            }

            if (isLeafFall)
            {
                if (myDoy == doyStartSenescence || int(LAIstartSenescence) == int(NODATA))
                    LAIstartSenescence = myLai;
                else
                    myLai = leafDevelopment::getLAISenescence(LAImin, LAIstartSenescence, myDoy - doyStartSenescence);
            }

            myLai += LAIgrass;
        }
    }

    LAI = myLai;
    return true;
}


bool Crit3DCrop::isWaterSurplusResistant()
{
    return (idCrop == "RICE" || idCrop == "KIWIFRUIT" || type == GRASS || type == FALLOW);
}


int Crit3DCrop::getDaysFromTypicalSowing(int myDoy)
{
    return (myDoy - sowingDoy) % 365;
}


int Crit3DCrop::getDaysFromCurrentSowing(int myDoy)
{
    if (currentSowingDoy != NODATA)
        return (myDoy - currentSowingDoy) % 365;
    else
        return getDaysFromTypicalSowing(myDoy);
}


bool Crit3DCrop::isInsideTypicalCycle(int myDoy)
{
    return ((myDoy >= sowingDoy) && (getDaysFromTypicalSowing(myDoy) < plantCycle));
}


bool Crit3DCrop::isPluriannual()
{
    return (type == HERBACEOUS_PERENNIAL ||
            type == GRASS ||
            type == FALLOW ||
            type == FRUIT_TREE);
}


bool Crit3DCrop::needReset(Crit3DDate myDate, double latitude, double waterTableDepth)
{
    int currentDoy = getDoyFromDate(myDate);

    if (isPluriannual())
    {
        // pluriannual crop: reset at the end of year (january at north / july at south)
        if ((latitude >= 0 && myDate.month == 1 && myDate.day == 1)
            || (latitude < 0 && myDate.month == 7 && myDate.day == 1))
        {
            isLiving = true;
            return true;
        }
    }
    else
    {
        // annual crop
        if (isLiving)
        {
            // living crop: check end of crop cycle
            double cycleDD = degreeDaysEmergence + degreeDaysIncrease + degreeDaysDecrease;

            if ((degreeDays > cycleDD) || (getDaysFromCurrentSowing(currentDoy) > plantCycle))
            {
                isLiving = false;
                return true;
            }
        }
        else
        {
            // naked soil: check sowing
            int sowingDoyPeriod = 30;
            int daysFromSowing = getDaysFromTypicalSowing(currentDoy);

            // is sowing possible? (check period and watertable depth)
            if (daysFromSowing >= 0 && daysFromSowing <= sowingDoyPeriod)
            {
                double waterTableThreshold = 0.2;

                if (isWaterSurplusResistant()
                        || int(waterTableDepth) == int(NODATA)
                        || waterTableDepth >= waterTableThreshold)
                {
                    isLiving = true;
                    // update sowing doy
                    currentSowingDoy = sowingDoy + daysFromSowing;
                    return true;
                }
            }
        }
    }

    return false;
}


// reset of (already initialized) crop
// TODO: partenza intelligente (usando sowing doy e ciclo)
void Crit3DCrop::resetCrop(unsigned int nrLayers)
{
    // roots
    if (! isPluriannual())
    {
        roots.rootDensity[0] = 0.0;
        for (unsigned int i = 1; i < nrLayers; i++)
            roots.rootDensity[i] = 0;
    }

    isEmerged = false;

    if (isLiving)
    {
        degreeDays = 0;

        // LAI
        LAI = LAImin;

        if (type == FRUIT_TREE)
            LAI += LAIgrass;
    }
    else
    {
        degreeDays = NODATA;
        LAI = NODATA;
        currentSowingDoy = NODATA;

        // roots
        roots.rootLength = 0.0;
        roots.rootDepth = NODATA;
    }

    LAIstartSenescence = NODATA;
    daysSinceIrrigation = NODATA;
}


bool Crit3DCrop::dailyUpdate(const Crit3DDate& myDate, double latitude, unsigned int nrLayers, double totalDepth,
                             double tmin, double tmax, double waterTableDepth, std::string* myError)
{
    *myError = "";

    if (idCrop == "") return false;

    // check start/end crop cycle (update isLiving)
    if (needReset(myDate, latitude, waterTableDepth))
    {
        resetCrop(nrLayers);
    }

    if (isLiving)
    {
        int currentDoy = getDoyFromDate(myDate);

        // update degree days
        degreeDays += computeDegreeDays(tmin, tmax, thermalThreshold, upperThermalThreshold);

        // update LAI
        if ( !updateLAI(latitude, nrLayers, currentDoy))
        {
            *myError = "Error in updating LAI for crop " + idCrop;
            return false;
        }

        // update roots
        root::computeRootDepth(this, totalDepth, degreeDays, waterTableDepth);
        /* if (! root::computeRootDensity(myCrop, layersVector, nrLayers, totalDepth))
        {
            *myError = "Error in updating roots for crop " + QString::fromStdString(idCrop);
            return false;
        }*/
    }

    return true;
}


// Liangxia Zhang, Zhongmin Hu, Jiangwen Fan, Decheng Zhou & Fengpei Tang, 2014
// A meta-analysis of the canopy light extinction coefficient in terrestrial ecosystems
// "Cropland had the highest value of K (0.62), followed by broadleaf forest (0.59),
// shrubland (0.56), grassland (0.50), and needleleaf forest (0.45)"
double Crit3DCrop::getSurfaceCoverFraction()
{
    double k = 0.6;      // [-] light extinction coefficient

    if (idCrop == "" || ! isLiving || LAI < EPSILON)
    {
        return 0;
    }
    else
    {
        return 1 - exp(-k * LAI);
    }
}


double Crit3DCrop::getMaxEvaporation(double ET0)
{
    const double maxEvapRatio = 0.66;

    double SCF = this->getSurfaceCoverFraction();
    return ET0 * maxEvapRatio * (1 - SCF);
}


double Crit3DCrop::getMaxTranspiration(double ET0)
{
    if (idCrop == "" || ! isLiving || LAI < EPSILON)
        return 0;

    double SCF = this->getSurfaceCoverFraction();
    double kcmaxFactor = 1 + (kcMax - 1) * SCF;
    return ET0 * (SCF * kcmaxFactor);
}


speciesType getCropType(std::string cropType)
{
    // lower case
    std::transform(cropType.begin(), cropType.end(), cropType.begin(), ::tolower);

    if (cropType == "herbaceous")
        return HERBACEOUS_ANNUAL;
    else if (cropType == "herbaceous_perennial")
        return HERBACEOUS_PERENNIAL;
    else if (cropType == "horticultural")
        return HORTICULTURAL;
    else if (cropType == "grass")
        return GRASS;
    else if (cropType == "grass_first_year")
        return GRASS;
    else if (cropType == "fallow")
        return FALLOW;
    else if (cropType == "tree" || cropType == "fruit_tree")
        return FRUIT_TREE;
    else
        return HERBACEOUS_ANNUAL;
}

std::string getCropTypeString(speciesType cropType)
{
    switch (cropType)
    {
    case HERBACEOUS_ANNUAL:
        return "herbaceous";
    case HERBACEOUS_PERENNIAL:
        return "herbaceous_perennial";
    case HORTICULTURAL:
        return "horticultural";
    case GRASS:
        return "grass";
    case FALLOW:
        return "fallow";
    case FRUIT_TREE:
        return "fruit_tree";
    }

    return "No crop type";
}

double computeDegreeDays(double myTmin, double myTmax, double myLowerThreshold, double myUpperThreshold)
{
    return MAXVALUE((myTmin + MINVALUE(myTmax, myUpperThreshold)) / 2. - myLowerThreshold, 0);
}


