#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QDateTime>
#include <QSettings>
#include <QDebug>
#include <iostream>

#include "commonConstants.h"
#include "basicMath.h"
#include "utilities.h"
#include "cropDbQuery.h"
#include "soilDbTools.h"
#include "criteria1DProject.h"

using namespace std;

Criteria1DProject::Criteria1DProject()
{
    initialize();
}

void Criteria1DProject::initialize()
{
    isProjectLoaded = false;

    path = "";
    name = "";
    logFileName = "";
    outputCsvFileName = "";
    outputCsvPath = "";

    dbCropName = "";
    dbSoilName = "";
    dbMeteoName = "";
    dbForecastName = "";
    dbOutputName = "";
    dbUnitsName = "";

    projectError = "";
}



void Criteria1DProject::closeProject()
{
    if (isProjectLoaded)
    {
        logInfo("Close Project...");
        closeAllDatabase();
        logFile.close();

        isProjectLoaded = false;
    }
}


int Criteria1DProject::initializeProject(QString settingsFileName)
{
    closeProject();
    initialize();

    if (settingsFileName == "")
    {
        logError("Missing settings File.");
        return ERROR_SETTINGS_MISSING;
    }

    // Configuration file
    QFile myFile(settingsFileName);
    if (myFile.exists())
    {
        configFileName = QDir(myFile.fileName()).canonicalPath();
        configFileName = QDir().cleanPath(configFileName);
        //qDebug("Using config file %s", qPrintable(configFileName));

        QFileInfo fileInfo(configFileName);
        path = fileInfo.path() + "/";
    }
    else
    {
        projectError = "Cannot find settings file: " + settingsFileName;
        return ERROR_SETTINGS_WRONGFILENAME;
    }

    if (!readSettings())
        return ERROR_SETTINGS_MISSINGDATA;

    setLogFile();

    int myError = openAllDatabase();
    if (myError != CRIT3D_OK)
        return myError;

    if (! loadVanGenuchtenParameters(&(criteriaSimulation.dbSoil), criteriaSimulation.soilTexture, &(projectError)))
        return ERROR_SOIL_PARAMETERS;

    if (! loadDriessenParameters(&(criteriaSimulation.dbSoil), criteriaSimulation.soilTexture, &(projectError)))
        return ERROR_SOIL_PARAMETERS;

    isProjectLoaded = true;

    return CRIT3D_OK;
}


bool Criteria1DProject::readSettings()
{
    QSettings* projectSettings;
    projectSettings = new QSettings(configFileName, QSettings::IniFormat);
    projectSettings->beginGroup("project");

    path += projectSettings->value("path","").toString();
    name += projectSettings->value("name","").toString();

    dbCropName = projectSettings->value("db_crop","").toString();
    if (dbCropName.left(1) == ".")
        dbCropName = path + dbCropName;

    dbSoilName = projectSettings->value("db_soil","").toString();
    if (dbSoilName.left(1) == ".")
        dbSoilName = path + dbSoilName;

    dbMeteoName = projectSettings->value("db_meteo","").toString();
    if (dbMeteoName.left(1) == ".")
        dbMeteoName = path + dbMeteoName;
    if (dbMeteoName.right(3) == "xml")
        criteriaSimulation.isXmlGrid = true;

    dbForecastName = projectSettings->value("db_forecast","").toString();
    if (dbForecastName.left(1) == ".")
        dbForecastName = path + dbForecastName;

    // unitList list
    dbUnitsName = projectSettings->value("db_units","").toString();
    if (dbUnitsName.left(1) == ".")
        dbUnitsName = path + dbUnitsName;

    if (dbUnitsName == "")
    {
        projectError = "Missing information on units";
        return false;
    }

    dbOutputName = projectSettings->value("db_output","").toString();
    if (dbOutputName.left(1) == ".")
        dbOutputName = path + dbOutputName;

    // seasonal or short-term forecast
    projectSettings->endGroup();
    projectSettings->beginGroup("forecast");

    criteriaSimulation.isSeasonalForecast = projectSettings->value("isSeasonalForecast",0).toBool();
    criteriaSimulation.isShortTermForecast = projectSettings->value("isShortTermForecast",0).toBool();
    if ((criteriaSimulation.isSeasonalForecast) || (criteriaSimulation.isShortTermForecast))
    {
        outputCsvFileName = projectSettings->value("output").toString();

        if (outputCsvFileName.left(1) == ".")
            outputCsvFileName = path + outputCsvFileName;

        criteriaSimulation.firstSeasonMonth = projectSettings->value("firstMonth",0).toInt();
    }

    if (criteriaSimulation.isShortTermForecast)
    {
        criteriaSimulation.daysOfForecast = projectSettings->value("daysOfForecast",0).toInt();
    }

    criteriaSimulation.firstSimulationDate = projectSettings->value("firstDate",0).toDate();
    if (criteriaSimulation.firstSimulationDate.isValid())
    {
        criteriaSimulation.useAllMeteoData = false;
    }

    projectSettings->endGroup();
    return true;
}


void Criteria1DProject::closeAllDatabase()
{
    criteriaSimulation.dbCrop.close();
    criteriaSimulation.dbSoil.close();
    criteriaSimulation.dbMeteo.close();
    criteriaSimulation.dbForecast.close();
    criteriaSimulation.dbOutput.close();
}


int Criteria1DProject::openAllDatabase()
{
    closeAllDatabase();

    logInfo ("Crop DB: " + dbCropName);
    if (! QFile(dbCropName).exists())
    {
        projectError = "DB Crop file doesn't exist";
        closeAllDatabase();
        return ERROR_DBPARAMETERS;
    }

    criteriaSimulation.dbCrop = QSqlDatabase::addDatabase("QSQLITE");
    criteriaSimulation.dbCrop.setDatabaseName(dbCropName);
    if (! criteriaSimulation.dbCrop.open())
    {
        projectError = "Open Crop DB failed: " + criteriaSimulation.dbCrop.lastError().text();
        closeAllDatabase();
        return ERROR_DBPARAMETERS;
    }

    logInfo ("Soil DB: " + dbSoilName);
    if (! QFile(dbSoilName).exists())
    {
        projectError = "Soil DB file doesn't exist";
        closeAllDatabase();
        return ERROR_DBSOIL;
    }

    criteriaSimulation.dbSoil = QSqlDatabase::addDatabase("QSQLITE", "soil");
    criteriaSimulation.dbSoil.setDatabaseName(dbSoilName);
    if (! criteriaSimulation.dbSoil.open())
    {
        projectError = "Open soil DB failed: " + criteriaSimulation.dbSoil.lastError().text();
        closeAllDatabase();
        return ERROR_DBSOIL;
    }

    logInfo ("Meteo DB: " + dbMeteoName);
    if (! QFile(dbMeteoName).exists())
    {
        projectError = "Meteo points DB file doesn't exist";
        closeAllDatabase();
        return ERROR_DBMETEO_OBSERVED;
    }

    if (criteriaSimulation.isXmlGrid)
    {
        criteriaSimulation.observedMeteoGrid = new Crit3DMeteoGridDbHandler();
        if (! criteriaSimulation.observedMeteoGrid->parseXMLGrid(dbMeteoName, &projectError))
        {
            return ERROR_XMLGRIDMETEO_OBSERVED;
        }
        if (! criteriaSimulation.observedMeteoGrid->openDatabase(&projectError, "observed"))
        {
            return ERROR_DBMETEO_OBSERVED;
        }
        if (! criteriaSimulation.observedMeteoGrid->loadCellProperties(&projectError))
        {
            return ERROR_PROPERTIES_DBMETEO_OBSERVED;
        }
    }
    else
    {
        criteriaSimulation.dbMeteo = QSqlDatabase::addDatabase("QSQLITE", "meteo");
        criteriaSimulation.dbMeteo.setDatabaseName(dbMeteoName);
        if (! criteriaSimulation.dbMeteo.open())
        {
            projectError = "Open meteo DB failed: " + criteriaSimulation.dbMeteo.lastError().text();
            closeAllDatabase();
            return ERROR_DBMETEO_OBSERVED;
        }
    }

    // meteo forecast
    if (criteriaSimulation.isShortTermForecast)
    {
        logInfo ("Forecast DB: " + dbForecastName);
        if (! QFile(dbForecastName).exists())
        {
            projectError = "DBforecast file doesn't exist";
            closeAllDatabase();
            return ERROR_DBMETEO_FORECAST;
        }

        if (criteriaSimulation.isXmlGrid)
        {
            criteriaSimulation.forecastMeteoGrid = new Crit3DMeteoGridDbHandler();
            if (! criteriaSimulation.forecastMeteoGrid->parseXMLGrid(dbForecastName, &projectError))
            {
                return ERROR_XMLGRIDMETEO_FORECAST;
            }
            if (! criteriaSimulation.forecastMeteoGrid->openDatabase(&projectError, "forecast"))
            {
                return ERROR_DBMETEO_FORECAST;
            }
            if (! criteriaSimulation.forecastMeteoGrid->loadCellProperties(&projectError))
            {
                return ERROR_PROPERTIES_DBMETEO_FORECAST;
            }
        }
        else
        {
            criteriaSimulation.dbForecast = QSqlDatabase::addDatabase("QSQLITE", "forecast");
            criteriaSimulation.dbForecast.setDatabaseName(dbForecastName);
            if (! criteriaSimulation.dbForecast.open())
            {
                projectError = "Open forecast DB failed: " + criteriaSimulation.dbForecast.lastError().text();
                closeAllDatabase();
                return ERROR_DBMETEO_FORECAST;
            }
        }
    }

    // output DB (not used in seasonal forecast)
    if (! criteriaSimulation.isSeasonalForecast)
    {
        QFile::remove(dbOutputName);
        logInfo ("Output DB: " + dbOutputName);
        criteriaSimulation.dbOutput = QSqlDatabase::addDatabase("QSQLITE", "output");
        criteriaSimulation.dbOutput.setDatabaseName(dbOutputName);

        QString outputDbPath = getFilePath(outputCsvFileName);
        if (!QDir(outputDbPath).exists())
             QDir().mkdir(outputDbPath);


        if (! criteriaSimulation.dbOutput.open())
        {
            projectError = "Open output DB failed: " + criteriaSimulation.dbOutput.lastError().text();
            closeAllDatabase();
            return ERROR_DBOUTPUT;
        }
    }

    // db units
    logInfo ("Units DB: " + dbUnitsName);


    return CRIT3D_OK;
}


bool Criteria1DProject::initializeCsvOutputFile(QString dateOfForecast)
{
    if ((criteriaSimulation.isSeasonalForecast) || (criteriaSimulation.isShortTermForecast))
    {
        if (!QDir(outputCsvPath).exists())
            QDir().mkdir(outputCsvPath);

        // add date to filename (only for ShortTermForecast)
        if (criteriaSimulation.isShortTermForecast)
        {
            outputCsvFileName = outputCsvFileName.left(outputCsvFileName.length()-4);
            outputCsvFileName += "_" + dateOfForecast + ".csv";
        }

        outputFile.open(outputCsvFileName.toStdString().c_str(), std::ios::out | std::ios::trunc);
        if ( outputFile.fail())
        {
            logError("open failure: " + QString(strerror(errno)) + '\n');
            return false;
        }
        else
        {
            logInfo("Output file: " + outputCsvFileName + "\n");
        }

        if (criteriaSimulation.isSeasonalForecast)
            outputFile << "ID_CASE,CROP,SOIL,METEO,p5,p25,p50,p75,p95\n";

        else if(criteriaSimulation.isShortTermForecast)
            outputFile << "dateForecast,ID_CASE,CROP,SOIL,METEO,deficit,readilyAvailableWater,"
                           "forecast7daysPrec,forecast7daysETc,forecast7daysIRR,previousAllSeasonIRR\n";
    }

    return true;
}


bool Criteria1DProject::runSeasonalForecast(unsigned int index, double irriRatio)
{
    if (irriRatio < EPSILON)
    {
        // No irrigation: nothing to do
        outputFile << unitList[index].idCase.toStdString() << "," << unitList[index].idCrop.toStdString() << ",";
        outputFile << unitList[index].idSoil.toStdString() << "," << unitList[index].idMeteo.toStdString();
        outputFile << ",0,0,0,0,0\n";
        return true;
    }

    if (! criteriaSimulation.runModel(unitList[index], projectError))
    {
        logError();
        return false;
    }

    outputFile << unitList[index].idCase.toStdString() << "," << unitList[index].idCrop.toStdString() << ",";
    outputFile << unitList[index].idSoil.toStdString() << "," << unitList[index].idMeteo.toStdString();
    // percentiles
    double percentile = sorting::percentile(criteriaSimulation.seasonalForecasts, &(criteriaSimulation.nrSeasonalForecasts), 5, true);
    outputFile << "," << percentile * irriRatio;
    percentile = sorting::percentile(criteriaSimulation.seasonalForecasts, &(criteriaSimulation.nrSeasonalForecasts), 25, false);
    outputFile << "," << percentile * irriRatio;
    percentile = sorting::percentile(criteriaSimulation.seasonalForecasts, &(criteriaSimulation.nrSeasonalForecasts), 50, false);
    outputFile << "," << percentile * irriRatio;
    percentile = sorting::percentile(criteriaSimulation.seasonalForecasts, &(criteriaSimulation.nrSeasonalForecasts), 75, false);
    outputFile << "," << percentile * irriRatio;
    percentile = sorting::percentile(criteriaSimulation.seasonalForecasts, &(criteriaSimulation.nrSeasonalForecasts), 95, false);
    outputFile << "," << percentile * irriRatio << "\n";
    outputFile.flush();

    return true;
}


bool Criteria1DProject::runShortTermForecast(QString dateForecastStr, unsigned int index, double irriRatio)
{
    if (! criteriaSimulation.runModel(unitList[index], projectError))
    {
        logError();
        return false;
    }

    std::string idCaseStr = unitList[index].idCase.toStdString();

    // last Observed day: day before
    Crit3DDate lastObservedDate = Crit3DDate(dateForecastStr.toStdString()).addDays(-1);
    int lastIndex = criteriaSimulation.myCase.meteoPoint.nrObsDataDaysD - criteriaSimulation.daysOfForecast - 1;
    Crit3DDate firstDate = criteriaSimulation.myCase.meteoPoint.obsDataD[0].date;
    Crit3DDate lastDate = criteriaSimulation.myCase.meteoPoint.obsDataD[lastIndex].date;
    if (lastObservedDate < firstDate || lastObservedDate > lastDate)
    {
        logError(" wrong date.");
        return false;
    }

    std::string lastObservedDateStr = lastObservedDate.toStdString();
    std::string lastDateOfForecast = lastObservedDate.addDays(criteriaSimulation.daysOfForecast).toStdString();

    // first date for annual irrigation
    Crit3DDate firstDateAllSeason;
    if (criteriaSimulation.firstSeasonMonth <= lastObservedDate.month)
    {
        firstDateAllSeason = Crit3DDate(1, criteriaSimulation.firstSeasonMonth, lastObservedDate.year);
    }
    else
    {
        firstDateAllSeason = Crit3DDate(1, criteriaSimulation.firstSeasonMonth, lastObservedDate.year - 1);
    }

    std::string firstDateAllSeasonStr = firstDateAllSeason.toStdString();

    double prec = NODATA;
    double maxTranspiration = NODATA;
    double forecastIrrigation = NODATA;
    double previousIrrigation = NODATA;
    double readilyAvailWater = NODATA;
    double deficit = NODATA;

    std::string mySQLstr = "SELECT SUM(PREC) AS prec,"
                        " SUM(TRANSP_MAX) AS maxTransp, SUM(IRRIGATION) AS irr"
                        " FROM '" + idCaseStr + "'"
                        " WHERE DATE > '" + lastObservedDateStr + "'"
                        " AND DATE <= '" + lastDateOfForecast + "'";

    QString mySQL = QString::fromStdString(mySQLstr);

    QSqlQuery myQuery = criteriaSimulation.dbOutput.exec(mySQL);

    if (myQuery.lastError().type() != QSqlError::NoError)
    {
        logError(mySQL + "\n" + myQuery.lastError().text());
    }
    else
    {
        myQuery.last();
        prec = myQuery.value("prec").toDouble();
        maxTranspiration = myQuery.value("maxTransp").toDouble();
        forecastIrrigation = myQuery.value("irr").toDouble();
    }

    mySQLstr = "SELECT RAW, DEFICIT FROM '"
            + idCaseStr + "'"
            " WHERE DATE = '" + lastObservedDateStr + "'";

    mySQL = QString::fromStdString(mySQLstr);

    myQuery = criteriaSimulation.dbOutput.exec(mySQL);

    if (myQuery.lastError().type() != QSqlError::NoError)
        logError(mySQL + "\n" + myQuery.lastError().text());
    else
    {
        myQuery.last();
        readilyAvailWater = myQuery.value("RAW").toDouble();
        deficit = myQuery.value("DEFICIT").toDouble();
    }

    mySQLstr = "SELECT SUM(IRRIGATION) AS previousIrrigation FROM '"
            + idCaseStr + "'"
            " WHERE DATE <= '" + lastObservedDateStr + "'"
            " AND DATE >= '" + firstDateAllSeasonStr + "'";

    mySQL = QString::fromStdString(mySQLstr);

    myQuery = criteriaSimulation.dbOutput.exec(mySQL);

    if (myQuery.lastError().type() != QSqlError::NoError)
        logError(mySQL + "\n" + myQuery.lastError().text());
    else
    {
        myQuery.last();
        previousIrrigation = myQuery.value("previousIrrigation").toDouble();
    }

    outputFile << dateForecastStr.toStdString();
    outputFile << "," << unitList[index].idCase.toStdString();
    outputFile << "," << unitList[index].idCrop.toStdString();
    outputFile << "," << unitList[index].idSoil.toStdString();
    outputFile << "," << unitList[index].idMeteo.toStdString();
    outputFile << "," << QString::number(deficit,'f',2).toStdString();
    outputFile << "," << QString::number(readilyAvailWater,'f',1).toStdString();
    outputFile << "," << QString::number(prec,'f',1).toStdString();
    outputFile << "," << QString::number(maxTranspiration,'f',1).toStdString();
    outputFile << "," << forecastIrrigation * irriRatio;
    outputFile << "," << previousIrrigation * irriRatio << "\n";
    outputFile.flush();

    return true;
}


int Criteria1DProject::compute(QString dateOfForecast)
{
    bool isErrorModel = false;
    bool isErrorSoil = false;
    bool isErrorCrop = false;
    unsigned int nrUnitsComputed = 0;

    try
    {
        for (unsigned int i = 0; i < unitList.size(); i++)
        {
            // CROP
            unitList[i].idCrop = getCropFromClass(&(criteriaSimulation.dbCrop), "crop_class", "id_class",
                                                         unitList[i].idCropClass, &(projectError)).toUpper();
            if (unitList[i].idCrop == "")
            {
                logInfo("Unit " + unitList[i].idCase + " " + unitList[i].idCropClass + " ***** missing CROP *****");
                isErrorCrop = true;
                continue;
            }

            // IRRI_RATIO
            double irriRatio = double(getIrriRatioFromClass(&(criteriaSimulation.dbCrop), "crop_class", "id_class",
                                                            unitList[i].idCropClass, &(projectError)));
            if ((criteriaSimulation.isSeasonalForecast || criteriaSimulation.isShortTermForecast) && (int(irriRatio) == int(NODATA)))
            {
                logInfo("Unit " + unitList[i].idCase + " " + unitList[i].idCropClass + " ***** missing IRRIGATION RATIO *****");
                continue;
            }

            // SOIL
            unitList[i].idSoil = getIdSoilString(&(criteriaSimulation.dbSoil), unitList[i].idSoilNumber, &(projectError));
            if (unitList[i].idSoil == "")
            {
                logInfo("Unit " + unitList[i].idCase + " Soil nr." + QString::number(unitList[i].idSoilNumber) + " ***** missing SOIL *****");
                isErrorSoil = true;
                continue;
            }

            //LC a regime togliere questa info che rallenta tantissimo
            //logInfo("Unit " + unitList[i].idCase +" "+ unitList[i].idCrop +" "+ unitList[i].idSoil +" "+ unitList[i].idMeteo);

            if (criteriaSimulation.isSeasonalForecast)
            {
                if (runSeasonalForecast(i, irriRatio))
                    nrUnitsComputed++;
                else
                    isErrorModel = true;

                continue;
            }

            if(criteriaSimulation.isShortTermForecast)
            {
                if (runShortTermForecast(dateOfForecast, i, irriRatio))
                    nrUnitsComputed++;
                else
                    isErrorModel = true;

                continue;
            }

            if (criteriaSimulation.runModel(unitList[i], projectError))
                nrUnitsComputed++;
            else
            {
                projectError = "Unit: " + unitList[i].idCase + " - " + projectError;
                logError();
                isErrorModel = true;
            }
        }

        outputFile.close();

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
    else if (nrUnitsComputed < unitList.size())
    {
        if (isErrorModel)
            return WARNING_METEO_OR_MODEL;
        else if (isErrorSoil)
            return WARNING_SOIL;
        else if (isErrorCrop)
            return WARNING_CROP;
    }

    return CRIT3D_OK;
}


//-------------------
//
//   LOG functions
//
//-------------------

bool Criteria1DProject::setLogFile()
{
    if (!QDir(path + "log").exists())
         QDir().mkdir(path + "log");

    QString myDate = QDateTime().currentDateTime().toString("yyyy-MM-dd hh.mm");
    QString fileName = name + "_" + myDate + ".txt";

    logFileName = path + "log/" + fileName;
    std::cout << "SWB PROCESSOR - log file created:\n" << logFileName.toStdString() << std::endl;

    logFile.open(logFileName.toStdString().c_str());
    return (logFile.is_open());
}


void Criteria1DProject::logInfo(QString logStr)
{
    if (logFile.is_open())
        logFile << logStr.toStdString() << std::endl;

    std::cout << logStr.toStdString() << std::endl;
}


void Criteria1DProject::logError()
{
    if (logFile.is_open())
        logFile << "----ERROR!----\n" << projectError.toStdString() << std::endl;

    std::cout << "----ERROR!----\n" << projectError.toStdString() << std::endl << std::endl;
}


void Criteria1DProject::logError(QString myErrorStr)
{
    projectError = myErrorStr;
    logError();
}

