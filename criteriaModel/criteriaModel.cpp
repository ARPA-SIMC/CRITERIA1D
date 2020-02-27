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

#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QVariant>
#include <QString>
#include <QDebug>
#include <iostream>
#include <math.h>

#include "commonConstants.h"
#include "criteriaModel.h"
#include "crit3dDate.h"
#include "utilities.h"
#include "dbMeteoCriteria1D.h"
#include "soilDbTools.h"
#include "meteo.h"
#include "root.h"


CriteriaUnit::CriteriaUnit()
{
    this->idCase = "";
    this->idCrop = "";
    this->idSoil = "";
    this->idMeteo = "";

    this->idCropClass = "";
    this->idForecast = "";
    this->idSoilNumber = NODATA;
    this->idCropNumber = NODATA;
}


CriteriaModel::CriteriaModel()
{
    this->idCase = "";
    this->outputString = "";

    this->nrLayers = 0;
    this->layerThickness = 0.02;          /*!<  [m] default thickness = 2 cm  */

    this->maxSimulationDepth = 2.0;       /*!<  [m] default simulation depth = 2 meters  */

    this->isGeometricLayer = false;

    this->optimizeIrrigation = false;

    this->isSeasonalForecast = false;
    this->firstSeasonMonth = NODATA;
    this->nrSeasonalForecasts = 0;
    this->seasonalForecasts = nullptr;

    this->isShortTermForecast = false;
    this->daysOfForecast = NODATA;
}


void CriteriaModelOutput::initializeDailyOutput()
{
    this->dailyPrec = NODATA;
    this->dailyDrainage = NODATA;
    this->dailySurfaceRunoff = NODATA;
    this->dailyLateralDrainage = NODATA;
    this->dailyIrrigation = NODATA;
    this->dailySoilWaterContent = NODATA;
    this->dailySurfaceWaterContent = NODATA;
    this->dailyEt0 = NODATA;
    this->dailyEvaporation = NODATA;
    this->dailyMaxTranspiration = NODATA;
    this->dailyMaxEvaporation = NODATA;
    this->dailyTranspiration = NODATA;
    this->dailyCropAvailableWater = NODATA;
    this->dailyWaterDeficit = NODATA;
    this->dailyCapillaryRise = NODATA;
    this->dailyWaterTable = NODATA;
}


CriteriaModelOutput::CriteriaModelOutput()
{
    this->initializeDailyOutput();
}


bool CriteriaModel::setSoil(QString soilCode, QString *myError)
{
    // load Soil
    if (! loadSoil(&dbSoil, soilCode, &mySoil, soilTexture, &fittingOptions, myError))
        return false;

    soilLayers.clear();
    soilLayers = soil::getRegularSoilLayers(&mySoil, layerThickness);
    nrLayers = unsigned(soilLayers.size());

    return true;
}


QString getId5Char(QString id)
{
    if (id.length() < 5)
        id = "0" + id;

    return id;
}


bool CriteriaModel::loadMeteo(QString idMeteo, QString idForecast, QString *myError)
{
    QString queryString = "SELECT * FROM meteo_locations WHERE id_meteo='" + idMeteo + "'";
    QSqlQuery query = dbMeteo.exec(queryString);
    query.last();

    if (! query.isValid())
    {
        QString idMeteo5char = getId5Char(idMeteo);
        queryString = "SELECT * FROM meteo_locations WHERE id_meteo='" + idMeteo5char + "'";
        query = dbMeteo.exec(queryString);
        query.last();

        if (! query.isValid())
        {
            if (query.lastError().text() != "")
                *myError = "dbMeteo error: " + query.lastError().text();
            else
                *myError = "Missing meteo location:" + idMeteo;
            return(false);
        }
    }

    QString tableName = query.value("table_name").toString();

    double myLat, myLon;
    if (getValue(query.value(("latitude")), &myLat))
        this->meteoPoint.latitude = myLat;
    else
    {
        *myError = "Missing latitude in idMeteo: " + idMeteo;
        return false;
    }

    if (getValue(query.value(("longitude")), &myLon))
        this->meteoPoint.longitude = myLon;

    queryString = "SELECT * FROM " + tableName + " ORDER BY [date]";
    query = this->dbMeteo.exec(queryString);
    query.last();

    if (! query.isValid())
    {
        if (query.lastError().text() != "")
            *myError = "dbMeteo error: " + query.lastError().text();
        else
            *myError = "Missing meteo table:" + tableName;
        return false;
    }

    query.first();
    QDate firstObsDate = query.value("date").toDate();
    query.last();
    QDate lastObsDate = query.value("date").toDate();

    long nrDays = long(firstObsDate.daysTo(lastObsDate)) + 1;

    // Is Forecast: increase nr of days
    if (this->isShortTermForecast)
        nrDays += this->daysOfForecast;

    // Initialize data
    this->meteoPoint.initializeObsDataD(nrDays, getCrit3DDate(firstObsDate));

    // Read observed data
    if (! readDailyDataCriteria1D(&query, &meteoPoint, myError)) return false;

    // Add Short-Term forecast
    if (this->isShortTermForecast)
    {
        queryString = "SELECT * FROM meteo_locations WHERE id_meteo='" + idForecast + "'";
        query = this->dbForecast.exec(queryString);
        query.last();

        if (! query.isValid())
        {
            QString idForecast5char = getId5Char(idForecast);
            queryString = "SELECT * FROM meteo_locations WHERE id_meteo='" + idForecast5char + "'";
            query = dbForecast.exec(queryString);
            query.last();

            if (! query.isValid())
            {
                if (query.lastError().text() != "")
                    *myError = "dbForecast error: " + query.lastError().text();
                else
                    *myError = "Missing forecast location:" + idForecast;
                return false;
            }
        }
        QString tableNameForecast = query.value("table_name").toString();

        query.clear();
        queryString = "SELECT * FROM " + tableNameForecast + " ORDER BY [date]";
        query = this->dbForecast.exec(queryString);
        query.last();

        // check query
        if (! query.isValid())
        {
            if (query.lastError().text() != "")
                *myError = "dbForecast error: " + query.lastError().text();
            else
                *myError = "Missing forecast table:" + tableName;
            return false;
        }

        // check date
        query.first();
        QDate firstForecastDate = query.value("date").toDate();
        if (firstForecastDate != lastObsDate.addDays(1))
        {
            // previsioni indietro di un giorno: accettato ma tolgo un giorno
            if (firstForecastDate == lastObsDate)
            {
                meteoPoint.nrObsDataDaysD--;
            }
            else
            {
                *myError = "The forecast date doesn't match with observed data.";
                return false;
            }
        }

        // Read forecast data
        if (! readDailyDataCriteria1D(&query, &meteoPoint, myError)) return false;

        // fill temperature (only forecast)
        // estende il dato precedente se mancante
        float previousTmin = NODATA;
        float previousTmax = NODATA;
        long lastObservedIndex = long(firstObsDate.daysTo(lastObsDate));
        for (unsigned long i = lastObservedIndex; i < meteoPoint.nrObsDataDaysD; i++)
        {
            // tmin
            if (int(meteoPoint.obsDataD[i].tMin) != int(NODATA))
                previousTmin = meteoPoint.obsDataD[i].tMin;
            else if (int(previousTmin) != int(NODATA))
                meteoPoint.obsDataD[i].tMin = previousTmin;

            // tmax
            if (int(meteoPoint.obsDataD[i].tMax) != int(NODATA))
                previousTmax = meteoPoint.obsDataD[i].tMax;
            else if (int(previousTmax) != int(NODATA))
                meteoPoint.obsDataD[i].tMax = previousTmax;
        }
    }

    // fill watertable (all data)
    // estende il dato precedente se mancante
    float previousWatertable = NODATA;
    for (unsigned long i = 0; i < meteoPoint.nrObsDataDaysD; i++)
    {
        // watertable
        if (int(meteoPoint.obsDataD[i].waterTable) != int(NODATA))
            previousWatertable = meteoPoint.obsDataD[i].waterTable;
        else if (int(previousWatertable) != int(NODATA))
            meteoPoint.obsDataD[i].waterTable = previousWatertable;
    }

    return true;
}


// alloc memory for annual values of irrigation
void CriteriaModel::initializeSeasonalForecast(const Crit3DDate& firstDate, const Crit3DDate& lastDate)
{
    if (isSeasonalForecast)
    {
        if (seasonalForecasts != nullptr) free(seasonalForecasts);

        nrSeasonalForecasts = lastDate.year - firstDate.year +1;
        seasonalForecasts = new double[unsigned(nrSeasonalForecasts)];
        for (int i = 0; i < nrSeasonalForecasts; i++)
            seasonalForecasts[i] = NODATA;
    }
}


bool CriteriaModel::createOutputTable(QString* myError)
{
    QString queryString = "DROP TABLE '" + this->idCase + "'";
    QSqlQuery myQuery = this->dbOutput.exec(queryString);

    queryString = "CREATE TABLE '" + this->idCase + "'"
            + " ( DATE TEXT, PREC REAL, IRRIGATION REAL, WATER_CONTENT REAL, SURFACE_WC REAL, "
            + " RAW REAL, DEFICIT REAL, DRAINAGE REAL, RUNOFF REAL, ET0 REAL, "
            + " TRANSP_MAX, TRANSP REAL, EVAP_MAX REAL, EVAP REAL, LAI REAL, ROOTDEPTH REAL )";
    myQuery = this->dbOutput.exec(queryString);

    if (myQuery.lastError().isValid())
    {
        *myError = "Error in creating table: " + this->idCase + "\n" + myQuery.lastError().text();
        return false;
    }

    return true;
}


QString getOutputStringNullZero(double value)
{
    if (int(value) != int(NODATA))
        return QString::number(value, 'g', 4);
    else
        return QString::number(0);
}


void CriteriaModel::prepareOutput(Crit3DDate myDate, bool isFirst)
{
    if (isFirst)
    {
        outputString = "INSERT INTO '" + this->idCase + "'"
            + " (DATE, PREC, IRRIGATION, WATER_CONTENT, SURFACE_WC, RAW, DEFICIT, DRAINAGE, RUNOFF, ET0,"
            + " TRANSP_MAX, TRANSP, EVAP_MAX, EVAP, LAI, ROOTDEPTH) "
            + " VALUES ";
    }
    else
    {
        outputString += ",";
    }

    outputString += "('" + QString::fromStdString(myDate.toStdString()) + "'"
            + "," + QString::number(this->output.dailyPrec, 'g', 4)
            + "," + QString::number(this->output.dailyIrrigation, 'g', 4)
            + "," + QString::number(this->output.dailySoilWaterContent, 'g', 5)
            + "," + QString::number(this->output.dailySurfaceWaterContent, 'g', 4)
            + "," + QString::number(this->output.dailyCropAvailableWater, 'g', 4)
            + "," + QString::number(this->output.dailyWaterDeficit, 'g', 4)
            + "," + QString::number(this->output.dailyDrainage, 'g', 4)
            + "," + QString::number(this->output.dailySurfaceRunoff, 'g', 4)
            + "," + QString::number(this->output.dailyEt0, 'g', 3)
            + "," + QString::number(this->output.dailyMaxTranspiration, 'g', 3)
            + "," + QString::number(this->output.dailyTranspiration, 'g', 3)
            + "," + QString::number(this->output.dailyMaxEvaporation, 'g', 3)
            + "," + QString::number(this->output.dailyEvaporation, 'g', 3)
            + "," + getOutputStringNullZero(this->myCrop.LAI)
            + "," + getOutputStringNullZero(this->myCrop.roots.rootDepth)
            + ")";

}


bool CriteriaModel::saveOutput(QString* myError)
{
    QSqlQuery myQuery = dbOutput.exec(outputString);
    outputString.clear();

    if (myQuery.lastError().type() != QSqlError::NoError)
    {
        *myError = "Error in saving output:\n" + myQuery.lastError().text();
        return false;
    }

    return true;
}
