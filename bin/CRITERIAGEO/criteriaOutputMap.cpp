#include "criteriaOutputMap.h"
#include <QSettings>
#include <QFile>
#include <QDir>

CriteriaOutputMap::CriteriaOutputMap()
{

}

void CriteriaOutputMap::initialize()
{
    isProjectLoaded = false;

    path = "";
    projectName = "";
    dbUnitsName = "";
    dbDataName = "";
    dbDataHistoricalName = "";
    dbCropName = "";
    ucmFileName = "";
    fieldListFileName = "";

    dbUnitsName = "";
    dbDataName = "";
    dbCropName = "";
    dbDataHistoricalName = "";

    outputFile = "";

    nrUnits = 0;
}


void CriteriaOutputMap::closeProject()
{
    if (isProjectLoaded)
    {
        unitList.clear();
        dbData.close();
        dbCrop.close();
        dbDataHistorical.close();

        isProjectLoaded = false;
    }
}

bool CriteriaOutputMap::initializeProject(QString settingsFileName)
{
    closeProject();
    initialize();

    if (settingsFileName == "")
    {
        error = "Missing settings File.";
        return false;
    }

    // Configuration file
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
        error = "Cannot find settings file: " + settingsFileName;
        return false;
    }

    if (!readSettings())
    {
        error = "Read settings: " + error;
        return false;
    }

    isProjectLoaded = true;
    return true;
}

bool CriteriaOutputMap::readSettings()
{
    QSettings* projectSettings;
    projectSettings = new QSettings(configFileName, QSettings::IniFormat);
    projectSettings->beginGroup("project");

    projectName = projectSettings->value("name","").toString();

    // unit list
    dbUnitsName = projectSettings->value("db_units","").toString();
    if (dbUnitsName.left(1) == ".")
    {
        dbUnitsName = path + QDir::cleanPath(dbUnitsName);
    }
    if (dbUnitsName == "")
    {
        error = "Missing information on units";
        return false;
    }

    dbDataName = projectSettings->value("db_data","").toString();
    if (dbDataName.left(1) == ".")
    {
        dbDataName = path + QDir::cleanPath(dbDataName);
    }

    dbCropName = projectSettings->value("db_crop","").toString();
    if (dbCropName.left(1) == ".")
    {
        dbCropName = path + QDir::cleanPath(dbCropName);
    }

    dbDataHistoricalName = projectSettings->value("db_data_historical","").toString();
    if (dbDataHistoricalName.left(1) == ".")
    {
        dbDataHistoricalName = path + QDir::cleanPath(dbDataHistoricalName);
    }
    projectSettings->endGroup();

    projectSettings->beginGroup("shapefile");
    // UCM
    ucmFileName = projectSettings->value("UCM","").toString();
    if (ucmFileName.left(1) == ".")
    {
        ucmFileName = path + QDir::cleanPath(ucmFileName);
    }

    // Field listgetFileNamegetFileName
    fieldListFileName = projectSettings->value("field_list", "").toString();
    if (fieldListFileName.left(1) == ".")
    {
        fieldListFileName = path + QDir::cleanPath(fieldListFileName);
    }
    projectSettings->endGroup();
    return true;
}
