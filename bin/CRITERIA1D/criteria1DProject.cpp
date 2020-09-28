#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QDateTime>
#include <QSettings>

#include "commonConstants.h"
#include "basicMath.h"
#include "cropDbQuery.h"
#include "soilDbTools.h"
#include "criteria1DProject.h"
#include "utilities.h"


Criteria1DProject::Criteria1DProject()
{
    initialize();
}

void Criteria1DProject::initialize()
{
    isProjectLoaded = false;

    path = "";
    projectName = "";
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
        logger.writeInfo("Close Project...");
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
        logger.writeError("Missing settings File.");
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

    logger.setLog(path, projectName);

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
    projectName += projectSettings->value("name","").toString();

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

    criteriaSimulation.firstSimulationDate = projectSettings->value("firstDate",0).toDate();
    if (criteriaSimulation.firstSimulationDate.isValid())
    {
        criteriaSimulation.useAllMeteoData = false;
    }

    // FORECAST (seasonal or short-term)
    projectSettings->endGroup();
    projectSettings->beginGroup("forecast");

    criteriaSimulation.isSeasonalForecast = projectSettings->value("isSeasonalForecast",0).toBool();
    criteriaSimulation.isShortTermForecast = projectSettings->value("isShortTermForecast",0).toBool();
    if (criteriaSimulation.isSeasonalForecast)
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

    logger.writeInfo ("Crop DB: " + dbCropName);
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

    logger.writeInfo ("Soil DB: " + dbSoilName);
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

    logger.writeInfo ("Meteo DB: " + dbMeteoName);
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
        logger.writeInfo ("Forecast DB: " + dbForecastName);
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
        logger.writeInfo ("Output DB: " + dbOutputName);
        criteriaSimulation.dbOutput = QSqlDatabase::addDatabase("QSQLITE", "output");
        criteriaSimulation.dbOutput.setDatabaseName(dbOutputName);

        QString outputDbPath = getFilePath(dbOutputName);
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
    logger.writeInfo ("Units DB: " + dbUnitsName);

    return CRIT3D_OK;
}


bool Criteria1DProject::initializeCsvOutputFile()
{
    if (criteriaSimulation.isSeasonalForecast)
    {
        if (! QDir(outputCsvPath).exists())
        {
            QDir().mkdir(outputCsvPath);
        }

        outputFile.open(outputCsvFileName.toStdString().c_str(), std::ios::out | std::ios::trunc);
        if ( outputFile.fail())
        {
            logger.writeError("open failure: " + QString(strerror(errno)) + '\n');
            return false;
        }
        else
        {
            logger.writeInfo("Output file: " + outputCsvFileName + "\n");
        }

        outputFile << "ID_CASE,CROP,SOIL,METEO,p5,p25,p50,p75,p95\n";
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
        logger.writeError(projectError);
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


int Criteria1DProject::compute()
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
                logger.writeInfo("Unit " + unitList[i].idCase + " " + unitList[i].idCropClass + " ***** missing CROP *****");
                isErrorCrop = true;
                continue;
            }

            // IRRI_RATIO
            double irriRatio = double(getIrriRatioFromClass(&(criteriaSimulation.dbCrop), "crop_class", "id_class",
                                                            unitList[i].idCropClass, &(projectError)));
            if ((criteriaSimulation.isSeasonalForecast || criteriaSimulation.isShortTermForecast) && (int(irriRatio) == int(NODATA)))
            {
                logger.writeInfo("Unit " + unitList[i].idCase + " " + unitList[i].idCropClass + " ***** missing IRRIGATION RATIO *****");
                continue;
            }

            // SOIL
            unitList[i].idSoil = getIdSoilString(&(criteriaSimulation.dbSoil), unitList[i].idSoilNumber, &(projectError));
            if (unitList[i].idSoil == "")
            {
                logger.writeInfo("Unit " + unitList[i].idCase + " Soil nr." + QString::number(unitList[i].idSoilNumber) + " ***** missing SOIL *****");
                isErrorSoil = true;
                continue;
            }

            //LC a regime togliere questa info che rallenta tantissimo
            //logger.writeInfo("Unit " + unitList[i].idCase +" "+ unitList[i].idCrop +" "+ unitList[i].idSoil +" "+ unitList[i].idMeteo);

            if (criteriaSimulation.isSeasonalForecast)
            {
                if (runSeasonalForecast(i, irriRatio))
                    nrUnitsComputed++;
                else
                    isErrorModel = true;
            }
            else
            {
                if (criteriaSimulation.runModel(unitList[i], projectError))
                {
                    nrUnitsComputed++;
                }
                else
                {
                    projectError = "Unit: " + unitList[i].idCase + " - " + projectError;
                    logger.writeError(projectError);
                    isErrorModel = true;
                }
            }
        }

        if (criteriaSimulation.isSeasonalForecast)
        {
            outputFile.close();
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

