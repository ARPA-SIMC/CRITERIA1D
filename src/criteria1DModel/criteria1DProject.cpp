#include <math.h>
#include "commonConstants.h"
#include "criteria1DError.h"
#include "criteria1DProject.h"
#include "basicMath.h"
#include "soilDbTools.h"
#include "cropDbTools.h"
#include "cropDbQuery.h"
#include "criteria1DMeteo.h"
#include "utilities.h"
#include "waterTableDb.h"

#include <QSqlError>
#include <QDate>
#include <QSqlQuery>
#include <QDir>
#include <QSettings>


Crit1DProject::Crit1DProject()
{
    this->initialize();
}


void Crit1DProject::initialize()
{
    isProjectLoaded = false;

    path = "";
    projectName = "";
    outputCsvFileName = "";
    _addDateTimeLogFile = false;

    _computeAllSoilDepth = true;
    _computationSoilDepth = NODATA;

    dbCropName = "";
    dbSoilName = "";
    dbMeteoName = "";
    dbForecastName = "";
    dbOutputName = "";
    dbWaterTableName = "";
    dbComputationUnitsName = "";

    projectError = "";

    compUnitList.clear();

    _isXmlMeteoGrid = false;
    _isSaveState = false;
    _isRestart = false;

    _isYearlyStatistics = false;
    _isMonthlyStatistics = false;
    _isSeasonalForecast = false;
    _isEnsembleForecast = false;
    _isShortTermForecast = false;

    _firstComputationMonth = NODATA;
    _daysOfForecast = NODATA;
    _nrComputationYears = NODATA;
    _nrSetupYears = NODATA;

    _irriSeries.clear();
    _precSeries.clear();

    firstSimulationDate = QDate(1800,1,1);
    lastSimulationDate = QDate(1800,1,1);

    outputString = "";

    // specific outputs
    isClimateOutput = false;
    waterDeficitDepth.clear();
    waterContentDepth.clear();
    degreeOfSaturationDepth.clear();
    waterPotentialDepth.clear();
    availableWaterDepth.clear();
    fractionAvailableWaterDepth.clear();
    factorOfSafetyDepth.clear();
    soilWaterIndexDepth.clear();
    awcDepth.clear();

    texturalClassList.resize(13);
    geotechnicsClassList.resize(19);
}


void Crit1DProject::closeProject()
{
    if (isProjectLoaded)
    {
        logger.writeInfo("Close Project...");
        closeAllDatabase();

        isProjectLoaded = false;
    }
}


bool Crit1DProject::readSettings()
{
    QSettings* projectSettings;
    projectSettings = new QSettings(configFileName, QSettings::IniFormat);

    // PROJECT
    projectSettings->beginGroup("project");

        projectName = projectSettings->value("name", "CRITERIA1D").toString();
        path += projectSettings->value("path", "").toString();

        dbCropName = projectSettings->value("db_crop", "").toString();
        if (dbCropName == "")
        {
            dbCropName = projectSettings->value("crop_db", "").toString();
        }
        if (dbCropName.left(1) == ".")
        {
            dbCropName = QDir::cleanPath(path + dbCropName);
        }

        dbSoilName = projectSettings->value("db_soil", "").toString();
        if (dbSoilName == "")
        {
            dbSoilName = projectSettings->value("soil_db", "").toString();
        }
        if (dbSoilName.left(1) == ".")
        {
            dbSoilName = QDir::cleanPath(path + dbSoilName);
        }

        dbMeteoName = projectSettings->value("db_meteo", "").toString();
        if (dbMeteoName.left(1) == ".")
        {
            dbMeteoName = QDir::cleanPath(path + dbMeteoName);
        }
        if (dbMeteoName.right(3).toUpper() == "XML")
        {
            _isXmlMeteoGrid = true;
        }

        dbForecastName = projectSettings->value("db_forecast", "").toString();
        if (dbForecastName.left(1) == ".")
        {
            dbForecastName = QDir::cleanPath(path + dbForecastName);
        }

        dbWaterTableName = projectSettings->value("db_waterTable", "").toString();
        if (dbWaterTableName.left(1) == ".")
        {
            dbWaterTableName = QDir::cleanPath(path + dbWaterTableName);
        }

        dbComputationUnitsName = projectSettings->value("db_comp_units", "").toString();
        if (dbComputationUnitsName == "")
        {
            // checks the old name
            dbComputationUnitsName = projectSettings->value("db_units", "").toString();
        }
        if (dbComputationUnitsName == "")
        {
            projectError = "Missing information on computational units";
            return false;
        }
        if (dbComputationUnitsName.left(1) == ".")
        {
            dbComputationUnitsName = QDir::cleanPath(path + dbComputationUnitsName);
        }

        // output db
        dbOutputName = projectSettings->value("db_output", "").toString();
        if (dbOutputName.left(1) == ".")
        {
            dbOutputName = QDir::cleanPath(path + dbOutputName);
        }

        // first date
        if (firstSimulationDate == QDate(1800,1,1))
        {
            firstSimulationDate = projectSettings->value("firstDate", 0).toDate();
            if (! firstSimulationDate.isValid())
            {
                firstSimulationDate = projectSettings->value("first_date", 0).toDate();
            }
            if (! firstSimulationDate.isValid())
            {
                firstSimulationDate = QDate(1800,1,1);
            }
        }

        // last date
        if (lastSimulationDate == QDate(1800,1,1))
        {
            lastSimulationDate = projectSettings->value("lastDate",0).toDate();
            if (! lastSimulationDate.isValid())
            {
                 lastSimulationDate = projectSettings->value("last_date",0).toDate();
            }
            if (! lastSimulationDate.isValid())
            {
                lastSimulationDate = QDate(1800,1,1);
            }
        }

        // save and restart
        _isSaveState = projectSettings->value("save_state","").toBool();
        _isRestart = projectSettings->value("restart","").toBool();

    projectSettings->endGroup();

    // SETTINGS
    projectSettings->beginGroup("settings");
        _addDateTimeLogFile = projectSettings->value("add_date_to_log", false).toBool();

        // soil computation depth
        _computeAllSoilDepth = projectSettings->value("compute_all_soil_depth", true).toBool();
        _computationSoilDepth = projectSettings->value("computationDepth", NODATA).toDouble();
    projectSettings->endGroup();

    // FORECAST
    projectSettings->beginGroup("forecast");
        _isYearlyStatistics = projectSettings->value("isYearlyStatistics", 0).toBool();
        _isMonthlyStatistics = projectSettings->value("isMonthlyStatistics", 0).toBool();

        _isSeasonalForecast = projectSettings->value("isSeasonalForecast", 0).toBool();
        _isShortTermForecast = projectSettings->value("isShortTermForecast", 0).toBool();

        // ensemble forecast (typically monthly)
        _isEnsembleForecast = projectSettings->value("isEnsembleForecast", 0).toBool();
        if (! _isEnsembleForecast)
        {
            // also check monthly forecast
            _isEnsembleForecast = projectSettings->value("isMonthlyForecast", 0).toBool();
        }

        if (_isShortTermForecast || _isEnsembleForecast)
        {
            _daysOfForecast = projectSettings->value("daysOfForecast", 0).toInt();
            if (_daysOfForecast == 0)
            {
                projectError = "Missing daysOfForecast";
                return false;
            }
        }

        // first month
        if (_isYearlyStatistics)
        {
            _firstComputationMonth = 1;
        }
        else if (_isMonthlyStatistics || _isSeasonalForecast)
        {
            _firstComputationMonth = projectSettings->value("firstMonth", 0).toInt();
            if (_firstComputationMonth == 0)
            {
                projectError = "Missing firstMonth.";
                return false;
            }
        }

        // setup years
        if (_isSeasonalForecast)
        {
            // default: 1 year
            _nrSetupYears = projectSettings->value("setupYears", 1).toInt();
        }

        // check computation type (only one)
        int nrOfComputationType = 0;
        if (_isShortTermForecast) nrOfComputationType++;
        if (_isEnsembleForecast) nrOfComputationType++;
        if (_isSeasonalForecast) nrOfComputationType++;
        if (_isMonthlyStatistics) nrOfComputationType++;
        if (_isYearlyStatistics) nrOfComputationType++;

        if (nrOfComputationType > 1)
        {
            projectError = "Too many forecast/computation types.";
            return false;
        }

    projectSettings->endGroup();

    projectSettings->beginGroup("csv");
        outputCsvFileName = projectSettings->value("csv_output","").toString();
        if (outputCsvFileName != "")
        {
            if (outputCsvFileName.right(4) == ".csv")
            {
                outputCsvFileName = outputCsvFileName.left(outputCsvFileName.length()-4);
            }

            bool addDate = projectSettings->value("add_date_to_filename","").toBool();
            if (addDate)
            {
                QString dateStr;
                if (lastSimulationDate == QDate(1800,1,1))
                {
                    dateStr = QDate::currentDate().toString("yyyy-MM-dd");
                }
                else
                {
                    dateStr = lastSimulationDate.addDays(1).toString("yyyy-MM-dd");
                }
                outputCsvFileName += "_" + dateStr;
            }
            outputCsvFileName += ".csv";

            if (outputCsvFileName.at(0) == '.')
            {
                outputCsvFileName = path + QDir::cleanPath(outputCsvFileName);
            }
        }

    projectSettings->endGroup();

    // OUTPUT variables (optional)
    QList<QString> depthList;
    projectSettings->beginGroup("output");

        isClimateOutput = projectSettings->value("isClimateOutput", false).toBool();

        depthList = projectSettings->value("waterContent").toStringList();
        if (! setVariableDepth(depthList, waterContentDepth))
        {
            projectError = "Wrong water content depth in " + configFileName;
            return false;
        }

        depthList = projectSettings->value("degreeOfSaturation").toStringList();
        if (! setVariableDepth(depthList, degreeOfSaturationDepth))
        {
            projectError = "Wrong degree of saturation depth in " + configFileName;
            return false;
        }

        depthList = projectSettings->value("waterPotential").toStringList();
        if (! setVariableDepth(depthList, waterPotentialDepth))
        {
            projectError = "Wrong water potential depth in " + configFileName;
            return false;
        }

        depthList = projectSettings->value("waterDeficit").toStringList();
        if (! setVariableDepth(depthList, waterDeficitDepth))
        {
            projectError = "Wrong water deficit depth in " + configFileName;
            return false;
        }

        depthList = projectSettings->value("awc").toStringList();
        if (! setVariableDepth(depthList, awcDepth))
        {
            projectError = "Wrong available water capacity depth in " + configFileName;
            return false;
        }

        depthList = projectSettings->value("availableWater").toStringList();
        if (depthList.size() == 0)
        {
            // alternative field name
            depthList = projectSettings->value("aw").toStringList();
        }
        if (! setVariableDepth(depthList, availableWaterDepth))
        {
            projectError = "Wrong available water depth in " + configFileName;
            return false;
        }

        depthList = projectSettings->value("fractionAvailableWater").toStringList();
        if (depthList.size() == 0)
        {
            // alternative field name
            depthList = projectSettings->value("faw").toStringList();
        }
        if (! setVariableDepth(depthList, fractionAvailableWaterDepth))
        {
            projectError = "Wrong fraction available water depth in " + configFileName;
            return false;
        }

        depthList = projectSettings->value("factorOfSafety").toStringList();
        if (depthList.size() == 0)
        {
            // alternative field name
            depthList = projectSettings->value("fos").toStringList();
        }
        if (! setVariableDepth(depthList, factorOfSafetyDepth))
        {
            projectError = "Wrong slope stability depth in " + configFileName;
            return false;
        }

        depthList = projectSettings->value("soilWaterIndex").toStringList();
        if (! setVariableDepth(depthList, soilWaterIndexDepth))
        {
            projectError = "Wrong Soil Water Index depth in " + configFileName;
            return false;
        }

    projectSettings->endGroup();

    return true;
}


int Crit1DProject::initializeProject(const QString &settingsFileName)
{
    if (settingsFileName == "")
    {
        logger.writeError("Missing settings File.");
        return ERROR_SETTINGS_MISSING;
    }

    // Settings file
    QFile myFile(settingsFileName);
    if (myFile.exists())
    {
        configFileName = QDir(myFile.fileName()).canonicalPath();
        configFileName = QDir().cleanPath(configFileName);

        QFileInfo fileInfo(configFileName);
        path = fileInfo.path() + "/";
    }
    else
    {
        projectError = "Cannot find settings file: " + settingsFileName;
        return ERROR_SETTINGS_WRONGFILENAME;
    }

    if (! readSettings())
        return ERROR_SETTINGS_MISSINGDATA;

    logger.setLog(path, projectName, _addDateTimeLogFile);

    checkSimulationDates();

    int myError = openAllDatabase();
    if (myError != CRIT1D_OK)
        return myError;

    if (! loadVanGenuchtenParameters(dbSoil, texturalClassList, projectError))
        return ERROR_SOIL_PARAMETERS;

    if (! loadDriessenParameters(dbSoil, texturalClassList, projectError))
        return ERROR_SOIL_PARAMETERS;

    // missing table is not critical
    loadGeotechnicsParameters(dbSoil, geotechnicsClassList, projectError);
    projectError = "";

    // Computational unit list
    if (! readComputationUnitList(dbComputationUnitsName, compUnitList, projectError))
    {
        logger.writeError(projectError);
        return ERROR_READ_UNITS;
    }
    logger.writeInfo("Query result: " + QString::number(compUnitList.size()) + " distinct computational units.");

    isProjectLoaded = true;

    return CRIT1D_OK;
}


void Crit1DProject::checkSimulationDates()
{
    // first date
    QString dateStr = firstSimulationDate.toString("yyyy-MM-dd");
    if (dateStr == "1800-01-01")
    {
        _isRestart = false;
        dateStr = "UNDEFINED";
    }
    logger.writeInfo("First simulation date: " + dateStr);
    QString boolStr = _isRestart ? "TRUE" : "FALSE";
    logger.writeInfo("Restart: " + boolStr);

    // last date
    dateStr = lastSimulationDate.toString("yyyy-MM-dd");
    if (dateStr == "1800-01-01")
    {
        if (_isXmlMeteoGrid)
        {
            lastSimulationDate = QDate::currentDate().addDays(-1);
            dateStr = lastSimulationDate.toString("yyyy-MM-dd");
        }
        else
        {
            _isSaveState = false;
            dateStr = "UNDEFINED";
        }
    }

    logger.writeInfo("Last simulation date: " + dateStr);
    boolStr = _isSaveState? "TRUE" : "FALSE";
    logger.writeInfo("Save state: " + boolStr);

    if (_isSeasonalForecast)
    {
        logger.writeInfo("First forecast month: " + QString::number(_firstComputationMonth));
    }

    if (_isMonthlyStatistics)
    {
        logger.writeInfo("Computation month: " + QString::number(_firstComputationMonth));
    }

    if (_isShortTermForecast || _isEnsembleForecast)
    {
        logger.writeInfo("Nr of forecast days: " + QString::number(_daysOfForecast));
    }
}


bool Crit1DProject::setSoil(const QString &soilCode, QString &errorStr)
{
    if (! loadSoil(dbSoil, soilCode, myCase.mySoil, texturalClassList, geotechnicsClassList, myCase.fittingOptions, errorStr))
        return false;

    // cancel warnings (some soil data are wrong)
    if (errorStr != "")
        errorStr = "";

    // check soil depth (if fixed depth is required)
    if (! _computeAllSoilDepth)
    {
        myCase.mySoil.totalDepth = std::min(myCase.mySoil.totalDepth, _computationSoilDepth);
    }

    std::string errorStdString;
    if (! myCase.initializeSoil(errorStdString))
    {
        errorStr = QString::fromStdString(errorStdString);
        return false;
    }

    return true;
}


bool Crit1DProject::setMeteoXmlGrid(QString idMeteo, const QString &idForecast, unsigned int memberNr)
{
    // check date
    QDate NODATE = QDate(1800, 1, 1);
    if (! firstSimulationDate.isValid() || firstSimulationDate == NODATE )
    {
        projectError = "Missing first simulation date.";
        return false;
    }
    if (! lastSimulationDate.isValid() || lastSimulationDate == NODATE )
    {
        projectError = "Missing last simulation date.";
        return false;
    }

    // check id
    unsigned row, col;
    bool isMeteoCellFound = false;
    if (observedMeteoGrid->meteoGrid()->findMeteoPointFromId(&row, &col, idMeteo.toStdString()))
    {
        isMeteoCellFound = true;
    }
    else if (idMeteo.at(0) == '0')
    {
        // try with one digit less
        idMeteo = idMeteo.right(idMeteo.size()-1);
        if (observedMeteoGrid->meteoGrid()->findMeteoPointFromId(&row, &col, idMeteo.toStdString()))
        {
            isMeteoCellFound = true;
        }
    }

    if (! isMeteoCellFound)
    {
        projectError = "Missing observed meteo cell: " + idMeteo;
        return false;
    }

    // set firstObsDate: load previous meteo data to compute watertable
    QDate firstObsDate = firstSimulationDate;
    if(myCase.unit.useWaterTableData && myCase.waterTableParameters.isLoaded)
    {
        firstObsDate = firstSimulationDate.addDays(-myCase.waterTableParameters.nrDaysPeriod);
    }

    unsigned nrDays = unsigned(firstObsDate.daysTo(lastSimulationDate)) + 1;

    if (! observedMeteoGrid->gridStructure().isFixedFields())
    {
        if (! observedMeteoGrid->loadGridDailyData(projectError, idMeteo, firstObsDate, lastSimulationDate))
        {
            projectError = "Missing observed data: " + idMeteo;
            return false;
        }
    }
    else
    {
        if (! observedMeteoGrid->loadGridDailyDataFixedFields(projectError, idMeteo, firstObsDate, lastSimulationDate))
        {
            if (projectError == "Missing MeteoPoint id")
            {
                projectError = "Missing observed meteo cell: " + idMeteo;
            }
            else
            {
                projectError = "Missing observed data: " + idMeteo;
            }
            return false;
        }
    }

    if (_isShortTermForecast)
    {
        if (! forecastMeteoGrid->gridStructure().isFixedFields())
        {
            if (! forecastMeteoGrid->loadGridDailyData(projectError, idForecast, lastSimulationDate.addDays(1),
                                                            lastSimulationDate.addDays(_daysOfForecast)))
            {
                if (projectError == "Missing MeteoPoint id")
                {
                    projectError = "Missing forecast meteo cell:" + idForecast;
                }
                else
                {
                    projectError = "Missing forecast data:" + idForecast;
                }
                return false;
            }
        }
        else
        {
            if (! forecastMeteoGrid->loadGridDailyDataFixedFields(projectError, idForecast,
                                              lastSimulationDate.addDays(1), lastSimulationDate.addDays(_daysOfForecast)))
            {
                if (projectError == "Missing MeteoPoint id")
                {
                    projectError = "Missing forecast meteo cell:" + idForecast;
                }
                else
                {
                    projectError = "Missing forecast data:" + idForecast;
                }
                return false;
            }
        }
        nrDays += unsigned(_daysOfForecast);
    }

    if (_isEnsembleForecast)
    {
        if (forecastMeteoGrid->gridStructure().isFixedFields())
        {
            projectError = "DB grid fixed fields: not available";
            return false;
        }
        else
        {
            if (! forecastMeteoGrid->loadGridDailyDataEnsemble(projectError, idForecast, int(memberNr),
                                            lastSimulationDate.addDays(1), lastSimulationDate.addDays(_daysOfForecast)))
            {
                if (projectError == "Missing MeteoPoint id")
                {
                    projectError = "Missing forecast meteo cell:" + idForecast;
                }
                else
                {
                    projectError = "Missing forecast data:" + idForecast;
                }
                return false;
            }
        }
        nrDays += unsigned(_daysOfForecast);
    }

    // set meteoPoint
    myCase.meteoPoint.latitude = observedMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->latitude;
    myCase.meteoPoint.longitude = observedMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->longitude;
    myCase.meteoPoint.initializeObsDataD(nrDays, getCrit3DDate(firstObsDate));

    long lastObsIndex = long(firstObsDate.daysTo(lastSimulationDate)) + 1;
    Crit3DDate currentDate = getCrit3DDate(firstObsDate);
    for (int i = 0; i < lastObsIndex; i++)
    {
        float tmin = observedMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(currentDate, dailyAirTemperatureMin);
        myCase.meteoPoint.setMeteoPointValueD(currentDate, dailyAirTemperatureMin, tmin);

        float tmax = observedMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(currentDate, dailyAirTemperatureMax);
        myCase.meteoPoint.setMeteoPointValueD(currentDate, dailyAirTemperatureMax, tmax);

        float tavg = observedMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(currentDate, dailyAirTemperatureAvg);
        if (isEqual(tavg, NODATA))
        {
            tavg = (tmax + tmin) * 0.5;
        }
        myCase.meteoPoint.setMeteoPointValueD(currentDate, dailyAirTemperatureAvg, tavg);

        float prec = observedMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(currentDate, dailyPrecipitation);
        myCase.meteoPoint.setMeteoPointValueD(currentDate, dailyPrecipitation, prec);
        ++currentDate;
    }

    if (_isShortTermForecast || _isEnsembleForecast)
    {
        for (int i = 1; i <= _daysOfForecast; i++)
        {
            Crit3DDate myDate = getCrit3DDate(lastSimulationDate.addDays(i));
            float tmin = forecastMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(myDate, dailyAirTemperatureMin);
            myCase.meteoPoint.setMeteoPointValueD(myDate, dailyAirTemperatureMin, tmin);

            float tmax = forecastMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(myDate, dailyAirTemperatureMax);
            myCase.meteoPoint.setMeteoPointValueD(myDate, dailyAirTemperatureMax, tmax);

            float tavg = forecastMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(myDate, dailyAirTemperatureAvg);
            if (isEqual(tavg, NODATA))
            {
                tavg = (tmax + tmin) * 0.5;
            }
            myCase.meteoPoint.setMeteoPointValueD(myDate, dailyAirTemperatureAvg, tavg);

            float prec = forecastMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(myDate, dailyPrecipitation);
            myCase.meteoPoint.setMeteoPointValueD(myDate, dailyPrecipitation, prec);
        }
    }

    // set watertable data
    if(myCase.unit.useWaterTableData && myCase.waterTableParameters.isLoaded)
    {
        myCase.fillWaterTableData();
    }

    return true;
}


bool Crit1DProject::setMeteoSqlite(const QString &idMeteo, const QString &idForecast)
{
    QString queryString = "SELECT * FROM point_properties WHERE id_meteo='" + idMeteo + "'";
    QSqlQuery query = dbMeteo.exec(queryString);
    query.last();

    if (! query.isValid() && idMeteo.at(0) == '0')
    {
        // try with one digit less (Pavan case)
        QString idMeteoShort = idMeteo.right(idMeteo.size()-1);
        queryString = "SELECT * FROM point_properties WHERE id_meteo='" + idMeteoShort + "'";
        query = dbMeteo.exec(queryString);
        query.last();
    }

    if (! query.isValid())
    {
        projectError = "Missing ID meteo in point_properties table: " + idMeteo;
        return false;
    }

    QString tableName = query.value("table_name").toString();

    double myLat, myLon;
    if (getValue(query.value(("latitude")), &myLat))
    {
        myCase.meteoPoint.latitude = myLat;
    }
    else
    {
        projectError = "Missing latitude for ID Meteo: " + idMeteo;
        return false;
    }

    if (getValue(query.value(("longitude")), &myLon))
    {
        myCase.meteoPoint.longitude = myLon;
    }
    else
    {
        projectError = "Missing longitude for ID Meteo: " + idMeteo;
        return false;
    }

    queryString = "SELECT * FROM '" + tableName + "' ORDER BY [date]";
    query = this->dbMeteo.exec(queryString);
    query.last();

    if (! query.isValid())
    {
        if (query.lastError().text() != "")
            projectError = "dbMeteo error: " + query.lastError().text();
        else
            projectError = "Missing meteo table:" + tableName;
        return false;
    }

    query.first();
    QDate firstDate = query.value("date").toDate();
    query.last();
    QDate lastDate = query.value("date").toDate();
    unsigned nrDays;
    bool subQuery = false;

    // check dates
    if (firstSimulationDate.toString("yyyy-MM-dd") != "1800-01-01")
    {
        if (firstSimulationDate < firstDate)
        {
            projectError = "Missing meteo data: required first date: " + firstSimulationDate.toString("yyyy-MM-dd");
            return false;
        }
        else
        {
            // data not reduced for watertable computation
            //firstDate = firstSimulationDate;
            subQuery = true;
        }
    }
    if (lastSimulationDate.toString("yyyy-MM-dd") != "1800-01-01")
    {
        if (lastSimulationDate > lastDate)
        {
            projectError = "Missing meteo data: required last date: " + lastSimulationDate.toString("yyyy-MM-dd");
            return false;
        }
        else
        {
            lastDate = lastSimulationDate;
            subQuery = true;
        }
    }

    nrDays = unsigned(firstDate.daysTo(lastDate)) + 1;
    if (subQuery)
    {
        query.clear();
        queryString = "SELECT * FROM '" + tableName + "' WHERE date BETWEEN '"
                    + firstDate.toString("yyyy-MM-dd") + "' AND '" + lastDate.toString("yyyy-MM-dd") + "'"
                    + " ORDER BY date";
        query = this->dbMeteo.exec(queryString);
    }

    // Forecast: increase nr of days
    if (_isShortTermForecast)
        nrDays += unsigned(_daysOfForecast);

    // Initialize data
    myCase.meteoPoint.initializeObsDataD(nrDays, getCrit3DDate(firstDate));

    // Read observed data
    int maxNrDays = NODATA; // all data
    if (! readDailyDataCriteria1D(query, myCase.meteoPoint, maxNrDays, projectError))
    {
        return false;
    }

    // write missing meteo data on log
    if (projectError != "")
    {
        this->logger.writeInfo(projectError);
        projectError = "";
    }

    // fill watertable
    if(myCase.unit.useWaterTableData && myCase.waterTableParameters.isLoaded)
    {
        myCase.fillWaterTableData();
    }

    // Add Short-Term forecast
    if (this->_isShortTermForecast)
    {
        queryString = "SELECT * FROM point_properties WHERE id_meteo='" + idForecast + "'";
        query = dbForecast.exec(queryString);
        query.last();

        if (! query.isValid())
        {
            if (query.lastError().text().isEmpty())
            {
                projectError = "DB: " + dbForecast.databaseName() + "\nMissing point_properties for id meteo:" + idForecast;
            }
            else
            {
                projectError = "dbForecast error: " + query.lastError().text();
            }
            return false;
        }

        QString tableNameForecast = query.value("table_name").toString();

        query.clear();
        queryString = "SELECT * FROM '" + tableNameForecast + "' ORDER BY [date]";
        query = this->dbForecast.exec(queryString);
        query.last();

        // check query
        if (! query.isValid())
        {
            if (query.lastError().text() != "")
                projectError = "dbForecast error: " + query.lastError().text();
            else
                projectError = "Missing forecast table:" + tableName;
            return false;
        }

        // check date
        query.first();
        QDate firstForecastDate = query.value("date").toDate();
        if (firstForecastDate != lastDate.addDays(1))
        {
            // previsioni indietro di un giorno: accettato ma tolgo un giorno
            if (firstForecastDate == lastDate)
            {
                myCase.meteoPoint.nrObsDataDaysD--;
            }
            else
            {
                projectError = "The forecast date doesn't match with observed data.";
                return false;
            }
        }

        // Read forecast data
        maxNrDays = _daysOfForecast;
        if (! readDailyDataCriteria1D(query, myCase.meteoPoint, maxNrDays, projectError))
        {
            return false;
        }

        if (projectError != "")
        {
            this->logger.writeInfo(projectError);
        }

        // fill temperature (only forecast)
        // estende il dato precedente se mancante
        float previousTmin = NODATA;
        float previousTmax = NODATA;
        long lastIndex = long(firstDate.daysTo(lastDate));
        for (unsigned long i = unsigned(lastIndex); i < unsigned(myCase.meteoPoint.nrObsDataDaysD); i++)
        {
            // tmin
            if (int(myCase.meteoPoint.obsDataD[i].tMin) != int(NODATA))
                previousTmin = myCase.meteoPoint.obsDataD[i].tMin;
            else if (int(previousTmin) != int(NODATA))
                myCase.meteoPoint.obsDataD[i].tMin = previousTmin;

            // tmax
            if (int(myCase.meteoPoint.obsDataD[i].tMax) != int(NODATA))
                previousTmax = myCase.meteoPoint.obsDataD[i].tMax;
            else if (int(previousTmax) != int(NODATA))
                myCase.meteoPoint.obsDataD[i].tMax = previousTmax;
        }
    }

    // fill watertable (all data)
    // estende il dato precedente se mancante
    float previousWatertable = NODATA;
    for (unsigned long i = 0; i < unsigned(myCase.meteoPoint.nrObsDataDaysD); i++)
    {
        if (! isEqual(myCase.meteoPoint.obsDataD[i].waterTable, NODATA))
        {
            previousWatertable = myCase.meteoPoint.obsDataD[i].waterTable;
        }
        else if (! isEqual(previousWatertable, NODATA))
        {
            myCase.meteoPoint.obsDataD[i].waterTable = previousWatertable;
        }
    }

    return true;
}


bool Crit1DProject::computeUnit(const Crit1DCompUnit& myUnit)
{
    myCase.unit = myUnit;
    return computeCase(0);
}


bool Crit1DProject::computeUnit(unsigned int unitIndex, unsigned int memberNr)
{
    myCase.unit = compUnitList[unitIndex];
    return computeCase(memberNr);
}


// use memberNr = 0 for deterministic run
bool Crit1DProject::computeCase(unsigned int memberNr)
{
    // initialize
    myCase.nrMissingPrec = 0;
    myCase.fittingOptions.useWaterRetentionData = myCase.unit.useWaterRetentionData;

    // the user wants to compute the factor of safety
    myCase.computeFactorOfSafety = (factorOfSafetyDepth.size() > 0);

    if (! loadCropParameters(dbCrop, myCase.unit.idCrop, myCase.crop, projectError))
        return false;

    if (! setSoil(myCase.unit.idSoil, projectError))
        return false;

    // watertable
    if (myCase.unit.useWaterTableData && !dbWaterTableName.isEmpty() && !myCase.unit.idWaterTable.isEmpty())
    {
        projectError = "";
        WaterTableDb wtDataBase = WaterTableDb(dbWaterTableName, projectError);
        if (! projectError.isEmpty())
        {
            return false;
        }
        if (! wtDataBase.readSingleWaterTableParameters(myCase.unit.idWaterTable, myCase.waterTableParameters, projectError))
        {
            return false;
        }
    }

    // meteo data
    if (_isXmlMeteoGrid)
    {
        if (! setMeteoXmlGrid(myCase.unit.idMeteo, myCase.unit.idForecast, memberNr))
            return false;
    }
    else
    {
        if (! setMeteoSqlite(myCase.unit.idMeteo, myCase.unit.idForecast))
            return false;
    }

    // check meteo data
    if (myCase.meteoPoint.nrObsDataDaysD == 0)
    {
        projectError = "Missing meteo data: " + QString::fromStdString(myCase.meteoPoint.name);
        return false;
    }

    if ( !_isMonthlyStatistics && !_isSeasonalForecast && !_isEnsembleForecast )
    {
        if (! createOutputTable(projectError))
            return false;
    }
    // get irri ratio
    if (_isYearlyStatistics || _isMonthlyStatistics || _isSeasonalForecast)
    {
        float irriRatio = getIrriRatioFromCropClass(dbCrop, "crop_class", "id_class",
                                                myCase.unit.idCropClass, projectError);
        if (irriRatio < EPSILON)
        {
            // No irrigation: nothing to do
            return true;
        }
    }

    // set computation period
    unsigned long lastIndex = unsigned(myCase.meteoPoint.nrObsDataDaysD-1);

    Crit3DDate firstDate = myCase.meteoPoint.obsDataD[0].date;
    if (firstSimulationDate.isValid() && firstSimulationDate != QDate(1800,1,1))
    {
        firstDate = getCrit3DDate(firstSimulationDate);
    }
    Crit3DDate lastDate = myCase.meteoPoint.obsDataD[lastIndex].date;

    if (_isYearlyStatistics || _isMonthlyStatistics || _isSeasonalForecast)
    {
        initializeIrrigationStatistics(firstDate, lastDate);
    }
    int indexIrrigationSeries = NODATA;

    // initialize crop
    unsigned nrLayers = unsigned(myCase.soilLayers.size());
    myCase.crop.initialize(myCase.meteoPoint.latitude, nrLayers,
                             myCase.mySoil.totalDepth, getDoyFromDate(firstDate));

    // initialize water content
    if (! myCase.initializeWaterContent(firstDate))
        return false;

    // restart
    bool isFirstDay = true;
    std::string errorString;
    if (_isRestart)
    {
        QString outputDbPath = getFilePath(dbOutput.databaseName());
        QString stateDbName = outputDbPath + "state_" + firstSimulationDate.toString("yyyy_MM_dd") + ".db";
        if (! restoreState(stateDbName, projectError))
        {
            return false;
        }

        float waterTable = myCase.meteoPoint.getMeteoPointValueD(firstDate, dailyWaterTableDepth);
        if (! myCase.crop.restore(firstDate, myCase.meteoPoint.latitude, myCase.soilLayers, double(waterTable), errorString))
        {
            projectError = QString::fromStdString(errorString);
            return false;
        }
    }

    // daily cycle
    for (Crit3DDate myDate = firstDate; myDate <= lastDate; ++myDate)
    {
        if (! myCase.computeDailyModel(myDate, errorString))
        {
            projectError = QString::fromStdString(errorString);
            return false;
        }

        // output
        if (_isYearlyStatistics || _isMonthlyStatistics || _isSeasonalForecast)
        {
            updateIrrigationStatistics(myDate, indexIrrigationSeries);
        }
        if (_isEnsembleForecast)
        {
            updateEnsembleForecastOutput(myDate, memberNr);
        }
        if ( !_isEnsembleForecast && !_isSeasonalForecast && !_isMonthlyStatistics)
        {
            updateOutput(myDate, isFirstDay);
            isFirstDay = false;
        }
    }

    if (_isSaveState)
    {
        if (! saveState(projectError))
            return false;
        logger.writeInfo("Save state:" + dbState.databaseName());
    }

    // SeasonalForecast, EnsembleForecast and MonthlyStatistics do not produce db output (too much useless data)
    if (_isSeasonalForecast || _isEnsembleForecast || _isMonthlyStatistics)
        return true;
    else
        return saveOutput(projectError);
}


int Crit1DProject::computeAllUnits()
{
    bool isErrorModel = false;
    bool isErrorSoil = false;
    bool isErrorCrop = false;
    unsigned int nrUnitsComputed = 0;

    if (_isYearlyStatistics || _isMonthlyStatistics || _isSeasonalForecast || _isEnsembleForecast)
    {
        if (! setPercentileOutputCsv())
            return ERROR_DBOUTPUT;
    }
    else
    {
        if (dbOutputName == "")
        {
            logger.writeError("Missing output db");
            return ERROR_DBOUTPUT;
        }

    }

    // create db state
    if (_isSaveState)
    {
        if (! createDbState(projectError))
        {
            logger.writeError(projectError);
            return ERROR_DB_STATE;
        }
    }

    int infoStep = int(compUnitList.size());
    if (compUnitList.size() >= 20)
    {
        infoStep = int(compUnitList.size() / 20);
    }
    logger.writeInfo("COMPUTE...");

    try
    {
        for (unsigned int i = 0; i < compUnitList.size(); i++)
        {
            // is numerical infiltration
            if (compUnitList[i].isNumericalInfiltration)
            {
                logger.writeInfo(compUnitList[i].idCase + " - numerical computation...");
            }

            // check crop class
            compUnitList[i].idCrop = getIdCropFromClass(dbCrop, "crop_class", "id_class",
                                                         compUnitList[i].idCropClass, projectError);
            if (compUnitList[i].idCrop == "")
            {
                compUnitList[i].idCrop = compUnitList[i].idCropClass;
            }

            // check irri ratio
            float irriRatio = getIrriRatioFromCropClass(dbCrop, "crop_class", "id_class",
                                                    compUnitList[i].idCropClass, projectError);
            if (isEqual(irriRatio, NODATA))
            {
                irriRatio = 1;
            }

            // check soil
            if (compUnitList[i].idSoilNumber != NODATA)
            {
                compUnitList[i].idSoil = getIdSoilString(dbSoil, compUnitList[i].idSoilNumber, projectError);
            }
            if (compUnitList[i].idSoil == "")
            {
                logger.writeInfo("Unit " + compUnitList[i].idCase + " Soil nr." + QString::number(compUnitList[i].idSoilNumber) + " ***** missing SOIL *****");
                isErrorSoil = true;
                continue;
            }

            if (_isYearlyStatistics || _isMonthlyStatistics || _isSeasonalForecast)
            {
                if (computeIrrigationStatistics(i, irriRatio))
                    nrUnitsComputed++;
                else
                    isErrorModel = true;
            }
            else
            {
                if (_isEnsembleForecast)
                {
                    if (computeEnsembleForecast(i, irriRatio))
                        nrUnitsComputed++;
                    else
                        isErrorModel = true;
                }
                else
                {
                    if (computeUnit(i, 0))
                    {
                        nrUnitsComputed++;
                    }
                    else
                    {
                        projectError = "Computational Unit: " + compUnitList[i].idCase + "\n" + projectError;
                        logger.writeError(projectError);
                        isErrorModel = true;
                    }
                }
            }

            if ((i+1) % infoStep == 0 && nrUnitsComputed > 20)
            {
                double percentage = (i+1) * 100.0 / compUnitList.size();
                logger.writeInfo("..." + QString::number(round(percentage)) + "%");
            }
        }

        if (_isYearlyStatistics || _isMonthlyStatistics || _isSeasonalForecast || _isEnsembleForecast)
        {
            outputCsvFile.close();
        }

    } catch (std::exception &e)
    {
        qFatal("Error %s ", e.what());

    } catch (...)
    {
        qFatal("Error <unknown>");
        return ERROR_UNKNOWN;
    }

    // error check
    if (nrUnitsComputed == 0)
    {
        if (isErrorModel)
            return ERROR_METEO_OR_MODEL;
        else if (isErrorSoil)
            return ERROR_SOIL_MISSING;
        else if (isErrorCrop)
            return ERROR_CROP_MISSING;
        else
            return ERROR_UNKNOWN;
    }
    else if (nrUnitsComputed < compUnitList.size())
    {
        if (isErrorModel)
            return WARNING_METEO_OR_MODEL;
        else if (isErrorSoil)
            return WARNING_SOIL;
        else if (isErrorCrop)
            return WARNING_CROP;
    }

    return CRIT1D_OK;
}


// update values of ensemble forecast
void Crit1DProject::updateEnsembleForecastOutput(const Crit3DDate &myDate, unsigned int memberNr)
{
    QDate myQdate = getQDate(myDate);

    if (myQdate == lastSimulationDate)
    {
        _irriSeries[memberNr] = 0;
        _precSeries[memberNr] = 0;
    }
    else if (myQdate > lastSimulationDate)
    {
        _irriSeries[memberNr] += float(myCase.output.dailyIrrigation);
        _precSeries[memberNr] += float(myCase.output.dailyPrec);
    }
}


// update values of annual irrigation (month, season or whole year)
void Crit1DProject::updateIrrigationStatistics(const Crit3DDate &myDate, int &currentYearIndex)
{
    if ( !_isYearlyStatistics && !_isMonthlyStatistics && !_isSeasonalForecast )
        return;

    bool isInsideSeason = false;

    if (_isYearlyStatistics)
    {
        isInsideSeason = true;
    }

    if (_isMonthlyStatistics)
    {
        isInsideSeason = (myDate.month == _firstComputationMonth);
    }

    if (_isSeasonalForecast)
    {
        // interannual seasons
        if (_firstComputationMonth < 11)
        {
            if (myDate.month >= _firstComputationMonth && myDate.month <= _firstComputationMonth+2)
                isInsideSeason = true;
        }
        // NDJ or DJF
        else
        {
            int lastMonth = (_firstComputationMonth + 2) % 12;
            if (myDate.month >= _firstComputationMonth || myDate.month <= lastMonth)
                isInsideSeason = true;
        }
    }

    if (isInsideSeason)
    {
        // first date of season: increase year index
        if (myDate.day == 1 && myDate.month == _firstComputationMonth)
        {
            if (currentYearIndex == NODATA)
                currentYearIndex = 0;
            else
                currentYearIndex++;
        }

        // sum of irrigations
        if (currentYearIndex != NODATA)
        {
            if (isEqual(_irriSeries[currentYearIndex], NODATA))
            {
                // first value
                _irriSeries[currentYearIndex] = float(myCase.output.dailyIrrigation);
            }
            else
            {
                _irriSeries[currentYearIndex] += float(myCase.output.dailyIrrigation);
            }
        }
    }
}


bool Crit1DProject::computeEnsembleForecast(unsigned int unitIndex, float irriRatio)
{
    logger.writeInfo(compUnitList[unitIndex].idCase);

    if (!forecastMeteoGrid->gridStructure().isEnsemble())
    {
        projectError = "Forecast grid is not Ensemble.";
        logger.writeError(projectError);
        return false;
    }

    _nrComputationYears = forecastMeteoGrid->gridStructure().nrMembers();
    if (_nrComputationYears < 1)
    {
        projectError = "Missing ensemble members.";
        logger.writeError(projectError);
        return false;
    }

    _irriSeries.resize(unsigned(_nrComputationYears));
    _precSeries.resize(unsigned(_nrComputationYears));
    for (unsigned int memberNr = 1; memberNr < unsigned(_nrComputationYears); memberNr++)
    {
        if (! computeUnit(unitIndex, memberNr))
        {
            logger.writeError(projectError);
            return false;
        }
    }

    // write output
    outputCsvFile << compUnitList[unitIndex].idCase.toStdString();
    outputCsvFile << "," << compUnitList[unitIndex].idCropClass.toStdString();

    // percentiles irrigation
    float percentile = sorting::percentile(_irriSeries, _nrComputationYears, 5, true);
    outputCsvFile << "," << QString::number(double(percentile * irriRatio), 'f', 1).toStdString();
    percentile = sorting::percentile(_irriSeries, _nrComputationYears, 25, false);
    outputCsvFile << "," << QString::number(double(percentile * irriRatio), 'f', 1).toStdString();
    percentile = sorting::percentile(_irriSeries, _nrComputationYears, 50, false);
    outputCsvFile << "," << QString::number(double(percentile * irriRatio), 'f', 1).toStdString();
    percentile = sorting::percentile(_irriSeries, _nrComputationYears, 75, false);
    outputCsvFile << "," << QString::number(double(percentile * irriRatio), 'f', 1).toStdString();
    percentile = sorting::percentile(_irriSeries, _nrComputationYears, 95, false);
    outputCsvFile << "," << QString::number(double(percentile * irriRatio), 'f', 1).toStdString();

    // percentiles prec
    percentile = sorting::percentile(_precSeries, _nrComputationYears, 5, true);
    outputCsvFile << "," << QString::number(double(percentile), 'f', 1).toStdString();
    percentile = sorting::percentile(_precSeries, _nrComputationYears, 25, false);
    outputCsvFile << "," << QString::number(double(percentile), 'f', 1).toStdString();
    percentile = sorting::percentile(_precSeries, _nrComputationYears, 50, false);
    outputCsvFile << "," << QString::number(double(percentile), 'f', 1).toStdString();
    percentile = sorting::percentile(_precSeries, _nrComputationYears, 75, false);
    outputCsvFile << "," << QString::number(double(percentile), 'f', 1).toStdString();
    percentile = sorting::percentile(_precSeries, _nrComputationYears, 95, false);
    outputCsvFile << "," << QString::number(double(percentile), 'f', 1).toStdString() << "\n";

    outputCsvFile.flush();

    return true;
}


bool Crit1DProject::computeIrrigationStatistics(unsigned int index, float irriRatio)
{
    if (! computeUnit(index, 0))
    {
        projectError = "Computational Unit: " + compUnitList[index].idCase + " - " + projectError;
        logger.writeError(projectError);
        return false;
    }

    outputCsvFile << compUnitList[index].idCase.toStdString() << "," << compUnitList[index].idCrop.toStdString() << ",";
    outputCsvFile << compUnitList[index].idSoil.toStdString() << "," << compUnitList[index].idMeteo.toStdString();

    if (irriRatio < EPSILON)
    {
        // No irrigation
        outputCsvFile << ",0,0,0,0,0\n";
    }
    else
    {
        // irrigation percentiles
        float percentile = sorting::percentile(_irriSeries, _nrComputationYears, 5, true);
        outputCsvFile << "," << percentile * irriRatio;
        percentile = sorting::percentile(_irriSeries, _nrComputationYears, 25, false);
        outputCsvFile << "," << percentile * irriRatio;
        percentile = sorting::percentile(_irriSeries, _nrComputationYears, 50, false);
        outputCsvFile << "," << percentile * irriRatio;
        percentile = sorting::percentile(_irriSeries, _nrComputationYears, 75, false);
        outputCsvFile << "," << percentile * irriRatio;
        percentile = sorting::percentile(_irriSeries, _nrComputationYears, 95, false);
        outputCsvFile << "," << percentile * irriRatio << "\n";
    }

    outputCsvFile.flush();
    return true;
}


bool Crit1DProject::setPercentileOutputCsv()
{
    QString outputCsvPath = getFilePath(outputCsvFileName);
    if (! QDir(outputCsvPath).exists())
    {
        if (! QDir().mkdir(outputCsvPath))
        {
            logger.writeError("Error creating directory: " + outputCsvPath);
            return false;
        }
    }

    outputCsvFile.open(outputCsvFileName.toStdString().c_str(), std::ios::out | std::ios::trunc);
    if ( outputCsvFile.fail())
    {
        logger.writeError("Open failure: " + outputCsvFileName + "\n" + QString(strerror(errno)));
        return false;
    }

    logger.writeInfo("Statistics output file (csv): " + outputCsvFileName + "\n");

    if (_isYearlyStatistics || _isMonthlyStatistics || _isSeasonalForecast)
    {
        outputCsvFile << "ID_CASE,CROP,SOIL,METEO,irri_05,irri_25,irri_50,irri_75,irri_95\n";
    }
    else if (_isEnsembleForecast)
    {
        outputCsvFile << "ID_CASE,CROP,irri_05,irri_25,irri_50,irri_75,irri_95,prec_05,prec_25,prec_50,prec_75,prec_95\n";
    }

    return true;
}


// initialize vector for annual values of irrigation
void Crit1DProject::initializeIrrigationStatistics(const Crit3DDate& firstDate, const Crit3DDate& lastDate)
{
    _irriSeries.clear();

    _nrComputationYears = lastDate.year - firstDate.year +1;
    _irriSeries.resize(_nrComputationYears);

    for (int i = 0; i < _nrComputationYears; i++)
    {
        _irriSeries[i] = NODATA;
    }
}


bool Crit1DProject::createDbState(QString &myError)
{
    // create db state
    QString dateStr = lastSimulationDate.addDays(1).toString("yyyy_MM_dd");
    QString outputDbPath = getFilePath(dbOutput.databaseName());
    QString dbStateName = outputDbPath + "state_" + dateStr + ".db";
    if (QFile::exists(dbStateName))
    {
        QFile::remove(dbStateName);
    }
    dbState = QSqlDatabase::addDatabase("QSQLITE", "state");
    dbState.setDatabaseName(dbStateName);

    if (! dbState.open())
    {
        myError = "Open state DB failed: " + dbState.lastError().text();
        return false;
    }

    // create tables
    QString queryString;
    QSqlQuery myQuery;
    queryString = "CREATE TABLE variables ( ID_CASE TEXT, DEGREE_DAYS REAL, DAYS_SINCE_IRR INTEGER )";
    myQuery = dbState.exec(queryString);

    if (myQuery.lastError().isValid())
    {
        myError = "Error in creating variables table \n" + myQuery.lastError().text();
        return false;
    }

    queryString = "CREATE TABLE waterContent ( ID_CASE TEXT, NR_LAYER INTEGER, WC REAL )";
    myQuery = dbState.exec(queryString);

    if (myQuery.lastError().isValid())
    {
        myError = "Error in creating waterContent table \n" + myQuery.lastError().text();
        return false;
    }

    return true;
}


bool Crit1DProject::restoreState(const QString &dbStateName, QString &errorStr)
{
    if (! QFile::exists(dbStateName))
    {
        errorStr = "DB state: " + dbStateName + " does not exist";
        return false;
    }

    QSqlDatabase dbStateToRestore;
    if (QSqlDatabase::contains("stateToRestore"))
        dbStateToRestore = QSqlDatabase::database("stateToRestore");
    else
    {
        dbStateToRestore = QSqlDatabase::addDatabase("QSQLITE", "stateToRestore");
        dbStateToRestore.setDatabaseName(dbStateName);
    }

    if (! dbStateToRestore.open())
    {
        errorStr = "Open state DB failed: " + dbStateToRestore.lastError().text();
        return false;
    }
    QSqlQuery qry(dbStateToRestore);
    qry.prepare( "SELECT * FROM variables WHERE ID_CASE = :id_case");
    qry.bindValue(":id_case", myCase.unit.idCase);

    if( !qry.exec() )
    {
        errorStr = qry.lastError().text();
        return false;
    }
    else
    {
        double degreeDays;
        int daySinceIrr;
        if (qry.first())
        {
            if (! getValue(qry.value("DEGREE_DAYS"), &degreeDays))
            {
                errorStr = "DEGREE_DAYS not found";
                return false;
            }
            myCase.crop.degreeDays = degreeDays;
            if (! getValue(qry.value("DAYS_SINCE_IRR"), &daySinceIrr))
            {
                myCase.crop.daysSinceIrrigation = NODATA;
            }
            else
            {
                myCase.crop.daysSinceIrrigation = daySinceIrr;
            }
        }
        else
        {
            errorStr = "Case not found in the variables table of state DB: " + myCase.unit.idCase;
            return false;
        }
    }
    qry.clear();
    qry.prepare( "SELECT * FROM waterContent WHERE ID_CASE = :id_case");
    qry.bindValue(":id_case", myCase.unit.idCase);

    if( !qry.exec() )
    {
        errorStr = qry.lastError().text();
        return false;
    }
    else
    {
        int nrLayer = -1;
        double wc;
        while (qry.next())
        {
            if (!getValue(qry.value("NR_LAYER"), &nrLayer))
            {
                errorStr = "NR_LAYER not found";
                return false;
            }
            if (!getValue(qry.value("WC"), &wc))
            {
                errorStr = "WC not found";
                return false;
            }
            if (nrLayer < 0 || unsigned(nrLayer) >= myCase.soilLayers.size())
            {
                errorStr = "Invalid NR_LAYER";
                return false;
            }

            // check values
            if (nrLayer > 0)
            {
                wc = std::min(wc, myCase.soilLayers[unsigned(nrLayer)].SAT);
                wc = std::max(wc, myCase.soilLayers[unsigned(nrLayer)].HH);
            }

            myCase.soilLayers[unsigned(nrLayer)].waterContent = wc;
        }

        if (nrLayer == -1)
        {
            errorStr = "waterContent table: idCase not found";
            return false;
        }
    }

    dbStateToRestore.close();
    return true;
}


bool Crit1DProject::saveState(QString &myError)
{
    QString queryString;
    QSqlQuery qry(dbState);

    queryString = "INSERT INTO variables ( ID_CASE, DEGREE_DAYS, DAYS_SINCE_IRR ) VALUES ";
    queryString += "('" + myCase.unit.idCase + "'"
                + "," + QString::number(myCase.crop.degreeDays)
                + "," + QString::number(myCase.crop.daysSinceIrrigation) + ")";
    if( !qry.exec(queryString) )
    {
        myError = "Error in saving variables state:\n" + qry.lastError().text();
        return false;
    }
    qry.clear();

    queryString = "INSERT INTO waterContent ( ID_CASE, NR_LAYER, WC ) VALUES ";
    for (unsigned int i = 0; i<myCase.soilLayers.size(); i++)
    {
        queryString += "('" + myCase.unit.idCase + "'," + QString::number(i) + ","
                    + QString::number(myCase.soilLayers[i].waterContent) + ")";
        if (i < (myCase.soilLayers.size()-1))
            queryString += ",";
    }

    if( !qry.exec(queryString))
    {
        myError = "Error in saving waterContent state:\n" + qry.lastError().text();
        return false;
    }

    qry.clear();
    return true;
}


bool Crit1DProject::createOutputTable(QString &myError)
{
    QString queryString = "DROP TABLE '" + myCase.unit.idCase + "'";
    QSqlQuery myQuery = this->dbOutput.exec(queryString);

    if (isClimateOutput)
    {
        queryString = "CREATE TABLE '" + myCase.unit.idCase + "'"
                      + " ( DATE TEXT, AVAILABLE_WATER REAL,"
                      + " TRANSP_MAX, TRANSP REAL";
    }
    else
    {
        queryString = "CREATE TABLE '" + myCase.unit.idCase + "'"
                  + " ( DATE TEXT, PREC REAL, IRRIGATION REAL, WATER_CONTENT REAL, SURFACE_WC REAL, "
                  + " AVAILABLE_WATER REAL, READILY_AW REAL, FRACTION_AW REAL, "
                  + " RUNOFF REAL, DRAINAGE REAL, LATERAL_DRAINAGE REAL, CAPILLARY_RISE REAL, "
                  + " ET0 REAL, TRANSP_MAX, TRANSP REAL, EVAP_MAX REAL, EVAP REAL, "
                  + " LAI REAL, ROOT_DEPTH REAL, BALANCE REAL";
    }

    // specific depth variables
    for (unsigned int i = 0; i < waterContentDepth.size(); i++)
    {
        QString fieldName = "VWC_" + QString::number(waterContentDepth[i]);
        queryString += ", " + fieldName + " REAL";
    }
    for (unsigned int i = 0; i < degreeOfSaturationDepth.size(); i++)
    {
        QString fieldName = "DEGSAT_" + QString::number(degreeOfSaturationDepth[i]);
        queryString += ", " + fieldName + " REAL";
    }
    for (unsigned int i = 0; i < waterPotentialDepth.size(); i++)
    {
        QString fieldName = "WP_" + QString::number(waterPotentialDepth[i]);
        queryString += ", " + fieldName + " REAL";
    }
    for (unsigned int i = 0; i < waterDeficitDepth.size(); i++)
    {
        QString fieldName = "DEFICIT_" + QString::number(waterDeficitDepth[i]);
        queryString += ", " + fieldName + " REAL";
    }
    for (unsigned int i = 0; i < awcDepth.size(); i++)
    {
        QString fieldName = "AWC_" + QString::number(awcDepth[i]);
        queryString += ", " + fieldName + " REAL";
    }
    for (unsigned int i = 0; i < availableWaterDepth.size(); i++)
    {
        QString fieldName = "AW_" + QString::number(availableWaterDepth[i]);
        queryString += ", " + fieldName + " REAL";
    }
    for (unsigned int i = 0; i < fractionAvailableWaterDepth.size(); i++)
    {
        QString fieldName = "FAW_" + QString::number(fractionAvailableWaterDepth[i]);
        queryString += ", " + fieldName + " REAL";
    }
    for (unsigned int i = 0; i < factorOfSafetyDepth.size(); i++)
    {
        QString fieldName = "FoS_" + QString::number(factorOfSafetyDepth[i]);
        queryString += ", " + fieldName + " REAL";
    }
    for (unsigned int i = 0; i < soilWaterIndexDepth.size(); i++)
    {
        QString fieldName = "SWI_" + QString::number(soilWaterIndexDepth[i]);
        queryString += ", " + fieldName + " REAL";
    }

    // close query
    queryString += ")";
    myQuery = this->dbOutput.exec(queryString);

    if (myQuery.lastError().isValid())
    {
        myError = "Error in creating table: " + myCase.unit.idCase + "\n" + myQuery.lastError().text();
        return false;
    }

    return true;
}


void Crit1DProject::updateOutput(const Crit3DDate &myDate, bool isFirst)
{
    if (isFirst)
    {
        if (isClimateOutput)
        {
            outputString = "INSERT INTO '" + myCase.unit.idCase + "'"
                           + " (DATE, AVAILABLE_WATER,"
                           + " TRANSP_MAX, TRANSP";
        }
        else
        {
            outputString = "INSERT INTO '" + myCase.unit.idCase + "'"
                       + " (DATE, PREC, IRRIGATION, WATER_CONTENT, SURFACE_WC, "
                       + " AVAILABLE_WATER, READILY_AW, FRACTION_AW, "
                       + " RUNOFF, DRAINAGE, LATERAL_DRAINAGE, CAPILLARY_RISE, ET0, "
                       + " TRANSP_MAX, TRANSP, EVAP_MAX, EVAP, LAI, ROOT_DEPTH, BALANCE";
        }

        // specific depth variables
        for (unsigned int i = 0; i < waterContentDepth.size(); i++)
        {
            QString fieldName = "VWC_" + QString::number(waterContentDepth[i]);
            outputString += ", " + fieldName;
        }
        for (unsigned int i = 0; i < degreeOfSaturationDepth.size(); i++)
        {
            QString fieldName = "DEGSAT_" + QString::number(degreeOfSaturationDepth[i]);
            outputString += ", " + fieldName;
        }
        for (unsigned int i = 0; i < waterPotentialDepth.size(); i++)
        {
            QString fieldName = "WP_" + QString::number(waterPotentialDepth[i]);
            outputString += ", " + fieldName;
        }
        for (unsigned int i = 0; i < waterDeficitDepth.size(); i++)
        {
            QString fieldName = "DEFICIT_" + QString::number(waterDeficitDepth[i]);
            outputString += ", " + fieldName;
        }
        for (unsigned int i = 0; i < awcDepth.size(); i++)
        {
            QString fieldName = "AWC_" + QString::number(awcDepth[i]);
            outputString += ", " + fieldName;
        }
        for (unsigned int i = 0; i < availableWaterDepth.size(); i++)
        {
            QString fieldName = "AW_" + QString::number(availableWaterDepth[i]);
            outputString += ", " + fieldName;
        }
        for (unsigned int i = 0; i < fractionAvailableWaterDepth.size(); i++)
        {
            QString fieldName = "FAW_" + QString::number(fractionAvailableWaterDepth[i]);
            outputString += ", " + fieldName;
        }
        for (unsigned int i = 0; i < factorOfSafetyDepth.size(); i++)
        {
            QString fieldName = "FoS_" + QString::number(factorOfSafetyDepth[i]);
            outputString += ", " + fieldName;
        }
        for (unsigned int i = 0; i < soilWaterIndexDepth.size(); i++)
        {
            QString fieldName = "SWI_" + QString::number(soilWaterIndexDepth[i]);
            outputString += ", " + fieldName;
        }

        outputString += ") VALUES ";
    }
    else
    {
        outputString += ",";
    }

    if (isClimateOutput)
    {
        outputString += "('" + QString::fromStdString(myDate.toISOString()) + "'"
                        + "," + QString::number(myCase.output.dailyAvailableWater, 'g', 4)
                        + "," + QString::number(myCase.output.dailyMaxTranspiration, 'g', 3)
                        + "," + QString::number(myCase.output.dailyTranspiration, 'g', 3);
    }
    else
    {
        outputString += "('" + QString::fromStdString(myDate.toISOString()) + "'"
                    + "," + QString::number(myCase.output.dailyPrec)
                    + "," + QString::number(myCase.output.dailyIrrigation)
                    + "," + QString::number(myCase.output.dailySoilWaterContent, 'g', 4)
                    + "," + QString::number(myCase.output.dailySurfaceWaterContent, 'g', 3)
                    + "," + QString::number(myCase.output.dailyAvailableWater, 'g', 4)
                    + "," + QString::number(myCase.output.dailyReadilyAW, 'g', 4)
                    + "," + QString::number(myCase.output.dailyFractionAW, 'g', 4)
                    + "," + QString::number(myCase.output.dailySurfaceRunoff, 'g', 3)
                    + "," + QString::number(myCase.output.dailyDrainage, 'g', 3)
                    + "," + QString::number(myCase.output.dailyLateralDrainage, 'g', 3)
                    + "," + QString::number(myCase.output.dailyCapillaryRise, 'g', 3)
                    + "," + QString::number(myCase.output.dailyEt0, 'g', 3)
                    + "," + QString::number(myCase.output.dailyMaxTranspiration, 'g', 3)
                    + "," + QString::number(myCase.output.dailyTranspiration, 'g', 3)
                    + "," + QString::number(myCase.output.dailyMaxEvaporation, 'g', 3)
                    + "," + QString::number(myCase.output.dailyEvaporation, 'g', 3)
                    + "," + getOutputStringNullZero(myCase.crop.LAI)
                    + "," + getOutputStringNullZero(myCase.crop.roots.rootDepth)
                    + "," + QString::number(myCase.output.dailyBalance, 'g', 3);
    }

    // specific depth variables
    for (unsigned int i = 0; i < waterContentDepth.size(); i++)
    {
        outputString += "," + QString::number(myCase.getVolumetricWaterContent(waterContentDepth[i]), 'g', 4);
    }
    for (unsigned int i = 0; i < degreeOfSaturationDepth.size(); i++)
    {
        outputString += "," + QString::number(myCase.getDegreeOfSaturation(degreeOfSaturationDepth[i]), 'g', 4);
    }
    for (unsigned int i = 0; i < waterPotentialDepth.size(); i++)
    {
        outputString += "," + QString::number(myCase.getWaterPotential(waterPotentialDepth[i]), 'g', 4);
    }
    for (unsigned int i = 0; i < waterDeficitDepth.size(); i++)
    {
        outputString += "," + QString::number(myCase.getWaterDeficitSum(waterDeficitDepth[i]), 'g', 4);
    }
    for (unsigned int i = 0; i < awcDepth.size(); i++)
    {
        outputString += "," + QString::number(myCase.getWaterCapacitySum(awcDepth[i]), 'g', 4);
    }
    for (unsigned int i = 0; i < availableWaterDepth.size(); i++)
    {
        outputString += "," + QString::number(myCase.getAvailableWaterSum(availableWaterDepth[i]), 'g', 4);
    }
    for (unsigned int i = 0; i < fractionAvailableWaterDepth.size(); i++)
    {
        outputString += "," + QString::number(myCase.getAvailableWaterFraction(fractionAvailableWaterDepth[i]), 'g', 4);
    }
    for (unsigned int i = 0; i < factorOfSafetyDepth.size(); i++)
    {
        outputString += "," + QString::number(myCase.getSlopeStability(factorOfSafetyDepth[i]), 'g', 4);
    }
    for (unsigned int i = 0; i < soilWaterIndexDepth.size(); i++)
    {
        outputString += "," + QString::number(myCase.getSoilWaterIndex(soilWaterIndexDepth[i]), 'g', 4);
    }

    outputString += ")";
}


bool Crit1DProject::saveOutput(QString &errorStr)
{
    QSqlQuery myQuery = dbOutput.exec(outputString);
    outputString.clear();

    if (myQuery.lastError().type() != QSqlError::NoError)
    {
        errorStr = "Error in saveOutput:\n" + myQuery.lastError().text();
        return false;
    }

    return true;
}


void Crit1DProject::closeAllDatabase()
{
    dbCrop.close();
    dbSoil.close();
    dbMeteo.close();
    dbForecast.close();
    dbOutput.close();
}


int Crit1DProject::openAllDatabase()
{
    closeAllDatabase();

    logger.writeInfo ("Crop DB: " + dbCropName);
    if (! QFile(dbCropName).exists())
    {
        projectError = "DB Crop file doesn't exist";
        closeAllDatabase();
        return ERROR_DBPARAMETERS;
    }

    dbCrop = QSqlDatabase::addDatabase("QSQLITE");
    dbCrop.setDatabaseName(dbCropName);
    if (! dbCrop.open())
    {
        projectError = "Open Crop DB failed: " + dbCrop.lastError().text();
        closeAllDatabase();
        return ERROR_DBPARAMETERS;
    }

    logger.writeInfo ("Soil DB: " + dbSoilName);
    if (! QFile(dbSoilName).exists())
    {
        projectError = "Soil DB file doesn't exist";
        closeAllDatabase();
        return ERROR_DBSOIL;
    }

    dbSoil = QSqlDatabase::addDatabase("QSQLITE", "soil");
    dbSoil.setDatabaseName(dbSoilName);
    if (! dbSoil.open())
    {
        projectError = "Open soil DB failed: " + dbSoil.lastError().text();
        closeAllDatabase();
        return ERROR_DBSOIL;
    }

    logger.writeInfo ("Meteo DB: " + dbMeteoName);
    if (! QFile(dbMeteoName).exists())
    {
        projectError = "Meteo points DB file doesn't exist";
        closeAllDatabase();
        return ERROR_DBMETEO_OBSERVED;
    }

    if (_isXmlMeteoGrid)
    {
        observedMeteoGrid = new Crit3DMeteoGridDbHandler();
        if (! observedMeteoGrid->parseXMLGrid(dbMeteoName, projectError))
        {
            return ERROR_XMLGRIDMETEO_OBSERVED;
        }
        if (! observedMeteoGrid->openDatabase(projectError, "observed"))
        {
            return ERROR_DBMETEO_OBSERVED;
        }
        if (! observedMeteoGrid->loadCellProperties(projectError))
        {
            return ERROR_PROPERTIES_DBMETEO_OBSERVED;
        }
    }
    else
    {
        dbMeteo = QSqlDatabase::addDatabase("QSQLITE", "meteo");
        dbMeteo.setDatabaseName(dbMeteoName);
        if (! dbMeteo.open())
        {
            projectError = "Open meteo DB failed: " + dbMeteo.lastError().text();
            closeAllDatabase();
            return ERROR_DBMETEO_OBSERVED;
        }
    }

    if (! dbWaterTableName.isEmpty())
    {
        logger.writeInfo ("Water Table DB: " + dbWaterTableName);
    }

    // meteo forecast
    if (_isShortTermForecast || _isEnsembleForecast)
    {
        logger.writeInfo ("Forecast DB: " + dbForecastName);
        if (! QFile(dbForecastName).exists())
        {
            projectError = "DBforecast file doesn't exist";
            closeAllDatabase();
            return ERROR_DBMETEO_FORECAST;
        }

        if (_isXmlMeteoGrid)
        {
            forecastMeteoGrid = new Crit3DMeteoGridDbHandler();
            if (! forecastMeteoGrid->parseXMLGrid(dbForecastName, projectError))
            {
                return ERROR_XMLGRIDMETEO_FORECAST;
            }
            if (! forecastMeteoGrid->openDatabase(projectError, "forecast"))
            {
                return ERROR_DBMETEO_FORECAST;
            }
            if (! forecastMeteoGrid->loadCellProperties(projectError))
            {
                return ERROR_PROPERTIES_DBMETEO_FORECAST;
            }
        }
        else
        {
            dbForecast = QSqlDatabase::addDatabase("QSQLITE", "forecast");
            dbForecast.setDatabaseName(dbForecastName);
            if (! dbForecast.open())
            {
                projectError = "Open forecast DB failed: " + dbForecast.lastError().text();
                closeAllDatabase();
                return ERROR_DBMETEO_FORECAST;
            }
        }
    }

    logger.writeInfo ("Computational units DB: " + dbComputationUnitsName);

    // output DB (not used in seasonal/monthly forecast)
    if ( !_isMonthlyStatistics && !_isSeasonalForecast && !_isEnsembleForecast)
    {
        if (dbOutputName == "")
        {
            logger.writeError("Missing output DB");
                return ERROR_DBOUTPUT;
        }
        QFile::remove(dbOutputName);
        logger.writeInfo ("Output DB: " + dbOutputName);
        dbOutput = QSqlDatabase::addDatabase("QSQLITE", "output");
        dbOutput.setDatabaseName(dbOutputName);

        QString outputDbPath = getFilePath(dbOutputName);
        if (!QDir(outputDbPath).exists())
             QDir().mkdir(outputDbPath);

        if (! dbOutput.open())
        {
            projectError = "Open output DB failed: " + dbOutput.lastError().text();
            closeAllDatabase();
            return ERROR_DBOUTPUT;
        }
    }

    return CRIT1D_OK;
}



QString getOutputStringNullZero(double value)
{
    if (int(value) != int(NODATA))
        return QString::number(value, 'g', 3);
    else
        return QString::number(0);
}


// variableDepth [cm] (integer numbers)
bool setVariableDepth(const QList<QString>& depthList, std::vector<int>& variableDepth)
{
    int nrDepth = depthList.size();
    if (nrDepth > 0)
    {
        variableDepth.resize(unsigned(nrDepth));
        for (int i = 0; i < nrDepth; i++)
        {
            variableDepth[unsigned(i)] = depthList[i].toInt();
            if (variableDepth[unsigned(i)] <= 0)
                return false;
        }
    }

    return true;
}
