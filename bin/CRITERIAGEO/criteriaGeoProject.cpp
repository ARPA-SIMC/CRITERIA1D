/*!
    \file gisProject.cpp

    \abstract Praga-Gis project class

    This file is part of CRITERIA-3D distribution.

    CRITERIA-3D has been developed by A.R.P.A.E. Emilia-Romagna.

    \copyright
    CRITERIA-3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    CRITERIA-3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License
    along with CRITERIA-3D.  If not, see <http://www.gnu.org/licenses/>.

    \authors
    Fausto Tomei ftomei@arpae.it
    Gabriele Antolini gantolini@arpae.it
    Laura Costantini laura.costantini0@gmail.com
*/

#include "commonConstants.h"
#include "criteriaGeoProject.h"
#include "shapeToRaster.h"
#include "unitCropMap.h"
#include "shapeFromCsv.h"
#include "shapeUtilities.h"
#include "formInfo.h"
#include "netcdfHandler.h"

#ifdef GDAL
    #include "gdalRasterFunctions.h"
    #include "gdalShapeFunctions.h"
    #include "gdalShapeIntersection.h"
#endif

#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QDebug>


CriteriaGeoProject::CriteriaGeoProject()
{}


void CriteriaGeoProject::addRaster(gis::Crit3DRasterGrid *myRaster, QString fileNameWithPath, int utmZone)
{
    GisObject* newObject = new(GisObject);
    newObject->setRaster(fileNameWithPath, myRaster, utmZone);
    this->objectList.push_back(newObject);
}


void CriteriaGeoProject::addShapeFile(Crit3DShapeHandler *myShape, QString fileNameWithPath, QString projectName, int utmZone)
{
    GisObject* newObject = new(GisObject);
    newObject->setShapeFile(fileNameWithPath, projectName, myShape, utmZone);
    this->objectList.push_back(newObject);
}


/*
void CriteriaGeoProject::addNetcdf(NetCDFHandler *myNetcdf, QString fileName, int utmZone)
{
    GisObject* newObject = new(GisObject);
    newObject->setNetcdf(fileName, myNetcdf, utmZone);
    this->objectList.push_back(newObject);
}
*/


bool CriteriaGeoProject::loadRaster(const QString &fileNameWithPath, QString &errorStr)
{
    gis::Crit3DRasterGrid* newRaster = new(gis::Crit3DRasterGrid);

    QString fileExtension = fileNameWithPath.right(4);
    if (fileExtension == ".flt" || fileExtension == ".img")
    {
        std::string errorStdStr;
        if (! gis::openRaster(fileNameWithPath.toStdString(), newRaster, gisSettings.utmZone, errorStdStr))
        {
            errorStr = "Wrong raster file: " + QString::fromStdString(errorStdStr);
            return false;
        }
    }
#ifdef GDAL
    else if (! readGdalRaster(fileNameWithPath, newRaster, gisSettings.utmZone, errorStr))
    {
        return false;
    }
#endif

    setDTMScale(newRaster->colorScale);
    addRaster(newRaster, fileNameWithPath, gisSettings.utmZone);

    return true;
}


/*
bool CriteriaGeoProject::loadNetcdf(QString fileNameWithPath)
{
    NetCDFHandler* netCDF = new NetCDFHandler();

    netCDF->initialize(gisSettings.utmZone);

    if (! netCDF->readProperties(fileNameWithPath.toStdString()))
    {
        logError("Wrong netcdf file: " + QString::fromStdString(netCDF->getMetadata()));
        return false;
    }

    addNetcdf(netCDF, fileNameWithPath, gisSettings.utmZone);
    return true;
}
*/


bool CriteriaGeoProject::loadShapefile(QString fileNameWithPath, QString projectName)
{
    Crit3DShapeHandler *myShape = new(Crit3DShapeHandler);
    if (!myShape->open(fileNameWithPath.toStdString()))
    {
        logError("Load shapefile failed.");
        return false;
    }

    addShapeFile(myShape, fileNameWithPath, projectName, myShape->getUtmZone());
    return true;
}


bool CriteriaGeoProject::newRasterFromShape(Crit3DShapeHandler &shape, const QString &field, const QString &outputName,
                                            double cellSize, bool showInfo)
{
    FormInfo formInfo;
    if (showInfo)
    {
        formInfo.start("Create raster...", 0);
    }

    gis::Crit3DRasterGrid *newRaster = new gis::Crit3DRasterGrid();

    bool isOk = rasterizeShape(shape, *newRaster, field.toStdString(), cellSize);
    if (isOk)
    {
        gis::updateMinMaxRasterGrid(newRaster);
        setDefaultScale(newRaster->colorScale);

        if (showInfo) formInfo.setText("Add raster to map...");

        addRaster(newRaster, outputName, shape.getUtmZone());
    }

    if (showInfo) formInfo.close();

    return isOk;
}


bool CriteriaGeoProject::fillRasterFromShape(Crit3DShapeHandler &shapeHandler, gis::Crit3DRasterGrid &refRaster,
                                             const QString &field, const QString &outputName, bool showInfo)
{
    FormInfo formInfo;
    if (showInfo)
    {
        formInfo.start("Fill raster...", 0);
    }

    gis::Crit3DRasterGrid *newRaster = new gis::Crit3DRasterGrid();

    bool isOk = rasterizeShapeWithRef(refRaster, *newRaster, shapeHandler, field.toStdString());
    if (isOk)
    {
        gis::updateMinMaxRasterGrid(newRaster);
        setDefaultScale(newRaster->colorScale);

        if (showInfo) formInfo.setText("Add raster to map...");

        addRaster(newRaster, outputName, shapeHandler.getUtmZone());
    }

    if (showInfo) formInfo.close();

    return isOk;
}


bool CriteriaGeoProject::computeShapeAnomaly(Crit3DShapeHandler *shape1, Crit3DShapeHandler *shape2,
                             std::string id, std::string field1, std::string field2, QString fileName)
{
    QString errorStr;
    Crit3DShapeHandler *shapeAnomaly = new Crit3DShapeHandler();

    FormInfo formInfo;
    formInfo.start("Create shape...", 0);

    if (computeAnomaly(shapeAnomaly, shape1, shape2, id, field1, field2, fileName, errorStr))
    {
        addShapeFile(shapeAnomaly, fileName, "", shapeAnomaly->getUtmZone());
        formInfo.close();
        return true;
    }
    else
    {
        formInfo.close();
        logError(errorStr);
        return false;
    }
}


bool CriteriaGeoProject::computeUnitCropMap(Crit3DShapeHandler *shapeCrop, Crit3DShapeHandler *shapeSoil, Crit3DShapeHandler *shapeMeteo,
                                std::string idCrop, std::string idSoil, std::string idMeteo,
                                double cellSize, double threshold,
                                QString ucmFileName, bool isPrevailing, bool showInfo)
{
    std::string errorStr;

    if (isPrevailing)
    {
        Crit3DShapeHandler *shapeUCM = new Crit3DShapeHandler();

        if (computeUcmPrevailing(*shapeUCM, *shapeCrop, *shapeSoil, *shapeMeteo, idCrop, idSoil, idMeteo,
                                 cellSize, threshold, ucmFileName, errorStr, showInfo))
        {
            addShapeFile(shapeUCM, QString::fromStdString(shapeUCM->getFilepath()), "", shapeUCM->getUtmZone());
            return true;
        }
        else
        {
            logError(QString::fromStdString(errorStr));
            return false;
        }
    }
    else
    {
        #ifdef GDAL
            /*
            Crit3DShapeHandler *shapeUCM = new Crit3DShapeHandler();

            if (computeUcmIntersection(shapeUCM, shapeCrop, shapeSoil, shapeMeteo, idCrop, idSoil, idMeteo, ucmFileName, &errorStr))
            {
                addShapeFile(shapeUCM, QString::fromStdString(shapeUCM->getFilepath()), "", shapeUCM->getUtmZone());
                return true;
            }
            else
            {
                logError(QString::fromStdString(errorStr));
                return false;
            }*/
            logError("Function is not available.");
            return false;
        #else
            logError("Function is not available (needs GDAL library)");
            return false;
        #endif
    }
}


bool CriteriaGeoProject::extractUcmListToDb(Crit3DShapeHandler* shapeHandler, bool showInfo)
{
    QString errorStr;

    int fieldRequired = 0;
    for (int i = 0; i < shapeHandler->getFieldNumbers(); i++)
    {
        QString fieldName = QString::fromStdString(shapeHandler->getFieldName(i)).toUpper();
        if ( fieldName == "ID_CASE" || fieldName == "ID_SOIL" || fieldName == "ID_CROP" || fieldName == "ID_METEO"
            || (fieldName == "HECTARES" || fieldName == "HA") )
        {
            fieldRequired = fieldRequired + 1;
        }
    }
    if (fieldRequired < 5)
    {
        errorStr = "Required fields: ID_CASE, ID_SOIL, ID_CROP, ID_METEO, HECTARES";
        logError(errorStr);
        return false;
    }

    QString dbName = QFileDialog::getSaveFileName(nullptr, "Save as", "", "DB files (*.db)");
    if (dbName == "") return false;

    QFile dbFile(dbName);
    if (dbFile.exists())
    {
        if (!dbFile.remove())
        {
            logError("Remove file failed: " + dbName + "\n" + dbFile.errorString());
            return false;
        }
    }

    FormInfo formInfo;
    if (showInfo) formInfo.start("Extract computational units in: " + dbName, 0);

    bool result = writeUcmListToDb(*shapeHandler, dbName, errorStr);

    if (showInfo)
        formInfo.close();

    if (! result)
        logError(errorStr);

    return result;
}


bool CriteriaGeoProject::createShapeFromCsv(int shapeIndex, QString fileCsv, QString fileCsvFormat,
                                            QString outputFileName, QString &errorStr)
{
    Crit3DShapeHandler* shapeHandler = (objectList.at(unsigned(shapeIndex)))->getShapeHandler();

    bool found = false;
    for (int i = 0; i < shapeHandler->getFieldNumbers(); i++)
    {
        if (shapeHandler->getFieldName(i) == "ID_CASE")
        {
            found = true;
        }
    }
    if (!found)
    {
        errorStr = "Ivalid Computational Units Map - missing ID_CASE";
        return false;
    }

    FormInfo formInfo;
    formInfo.start("Create shapefile...", 0);

    bool isOk = shapeFromCsv(*shapeHandler, fileCsv, fileCsvFormat, outputFileName, errorStr);

    formInfo.close();

    return isOk;
}


int CriteriaGeoProject::createShapeOutput(QDate dateComputation, QString outputName)
{
    FormInfo formInfo;

    QString outputCsvFileName = output.path + "tmp/" + outputName +".csv";
    int result;
    if (! QFile(outputCsvFileName).exists())
    {
        formInfo.start("Create CSV file...", 0);
        result = output.createCsvFileFromGUI(dateComputation, outputCsvFileName);
        if (result != CRIT1D_OK)
        {
            return result;
        }
        formInfo.close();
    }

    formInfo.start("Create output map...", 0);
    result = output.createShapeFileFromGUI();

    formInfo.close();

    if (result != CRIT1D_OK)
        logError("ERROR CODE " + QString::number(result));

    // clean .csv
    QFile::remove(output.path + "tmp/" + outputName +".csv");

    return result;
}


//--------------------------------------------------------------
// LOG
//--------------------------------------------------------------

void CriteriaGeoProject::logError(QString errorString)
{
    QMessageBox::critical(nullptr, "ERROR!", errorString);
}

void CriteriaGeoProject::logWarning(QString errorString)
{
    QMessageBox::warning(nullptr, "WARNING", errorString);
}

