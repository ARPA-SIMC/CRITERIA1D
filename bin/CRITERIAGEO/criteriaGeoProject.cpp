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


void CriteriaGeoProject::addNetcdf(NetCDFHandler *myNetcdf, QString fileNameWithPath, int utmZone)
{
    GisObject* newObject = new(GisObject);
    newObject->setNetcdf(fileNameWithPath, myNetcdf, utmZone);
    this->objectList.push_back(newObject);
}


void CriteriaGeoProject::addShapeFile(Crit3DShapeHandler *myShape, QString fileNameWithPath, QString projectName, int utmZone)
{
    GisObject* newObject = new(GisObject);
    newObject->setShapeFile(fileNameWithPath, projectName, myShape, utmZone);
    this->objectList.push_back(newObject);
}


bool CriteriaGeoProject::loadRaster(QString fileNameWithPath)
{
    gis::Crit3DRasterGrid* myRaster = new(gis::Crit3DRasterGrid);
    int utmZone = this->gisSettings.utmZone;

#ifdef GDAL
    QString errorStr;
    if (! readGdalRaster(fileNameWithPath, myRaster, utmZone, errorStr))
    {
        logError(errorStr);
        return false;
    }
#else
     std::string errorStr;
     if (!gis::openRaster(fileNameWithPath.toStdString(), myRaster, errorStr))
     {
         logError("Wrong raster file: " + QString::fromStdString(errorStr));
         return false;
     }
#endif

    setDefaultDEMScale(myRaster->colorScale);
    addRaster(myRaster, fileNameWithPath, utmZone);
    return true;
}


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


void CriteriaGeoProject::getRasterFromShape(Crit3DShapeHandler &shape, QString field, QString outputName, double cellSize, bool showInfo)
{
    gis::Crit3DRasterGrid *newRaster = new gis::Crit3DRasterGrid();

    FormInfo formInfo;
    if (showInfo)
    {
        formInfo.start("Create raster...", 0);
    }

    if (rasterizeShape(shape, *newRaster, field.toStdString(), cellSize))
    {
        gis::updateMinMaxRasterGrid(newRaster);
        setTemperatureScale(newRaster->colorScale);

        if (showInfo) formInfo.setText("Add raster to map...");

        addRaster(newRaster, outputName, shape.getUtmZone());
    }
    else
    {
        logError("Error in rasterize shape.");
    }

    if (showInfo) formInfo.close();

}


bool CriteriaGeoProject::addUnitCropMap(Crit3DShapeHandler *crop, Crit3DShapeHandler *soil, Crit3DShapeHandler *meteo,
                                std::string idCrop, std::string idSoil, std::string idMeteo,
                                double cellSize, double threshold,
                                QString ucmFileName, bool isPrevailing, bool showInfo)
{
    std::string errorStr;

    Crit3DShapeHandler *ucm = new(Crit3DShapeHandler);

    if (isPrevailing)
    {
        if (computeUcmPrevailing(*ucm, *crop, *soil, *meteo, idCrop, idSoil, idMeteo,
                                 cellSize, threshold, ucmFileName, errorStr, showInfo))
        {
            addShapeFile(ucm, QString::fromStdString(ucm->getFilepath()), "", ucm->getUtmZone());
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
        if (computeUcmIntersection(ucm, crop, soil, meteo, idCrop, idSoil, idMeteo, ucmFileName, &errorStr))
        {
            addShapeFile(ucm, QString::fromStdString(ucm->getFilepath()), "", ucm->getUtmZone());
            return true;
        }
        else
        {
            logError(QString::fromStdString(errorStr));
            return false;
        }
        */
        logError("TO DO function");
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
        if (shapeHandler->getFieldName(i) == "ID_CASE" || shapeHandler->getFieldName(i) == "ID_SOIL"
            || shapeHandler->getFieldName(i) == "ID_CROP" || shapeHandler->getFieldName(i) == "ID_METEO"
            || shapeHandler->getFieldName(i) == "hectares" || shapeHandler->getFieldName(i) == "Hectares")
        {
            fieldRequired = fieldRequired + 1;
        }
    }
    if (fieldRequired < 5)
    {
        errorStr = "Required fields: ID_CASE, ID_SOIL, ID_CROP, ID_METEO, hectares";
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


#ifdef GDAL
bool CriteriaGeoProject::createRaster(QString shapeFileName, std::string shapeField, QString resolution, QString outputName, QString &error)
{
    QString proj = ""; //keep input proj
    if (shapeToRaster(shapeFileName, shapeField, resolution, proj, outputName, error))
    {
        return loadRaster(outputName);
    }
    return false;
}
#endif


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

