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

#ifdef USE_GDAL
    #include "gdalRasterFunctions.h"
    #include "gdalShapeFunctions.h"
#endif

#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QApplication>


CriteriaGeoProject::CriteriaGeoProject()
{
    _errorString = "";
}


void CriteriaGeoProject::addRaster(gis::Crit3DRasterGrid *rasterPtr, QString fileNameWithPath, int utmZone)
{
    GisObject* newObject = new(GisObject);
    newObject->setRaster(fileNameWithPath, rasterPtr, utmZone);
    this->objectList.push_back(newObject);
}


void CriteriaGeoProject::addShapeFile(Crit3DShapeHandler *shapePtr, QString fileNameWithPath, QString projectName, int utmZone)
{
    GisObject* newObject = new(GisObject);
    newObject->setShapeFile(fileNameWithPath, projectName, shapePtr, utmZone);
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


bool CriteriaGeoProject::loadRaster(const QString &fileNameWithPath)
{
    gis::Crit3DRasterGrid* newRaster = new(gis::Crit3DRasterGrid);

    QString fileExtension = fileNameWithPath.right(4);
    if (fileExtension == ".flt" || fileExtension == ".img")
    {
        std::string errorStdStr;
        if (! gis::openRaster(fileNameWithPath.toStdString(), newRaster, _gisSettings.utmZone, errorStdStr))
        {
            _errorString = "Wrong raster file: " + QString::fromStdString(errorStdStr);
            return false;
        }
    }
#ifdef USE_GDAL
    else if (! readGdalRaster(fileNameWithPath, newRaster, _gisSettings.utmZone, _errorString))
    {
        return false;
    }
#endif

    setDTMScale(newRaster->colorScale);
    addRaster(newRaster, fileNameWithPath, _gisSettings.utmZone);

    return true;
}


/*
bool CriteriaGeoProject::loadNetcdf(QString fileNameWithPath)
{
    NetCDFHandler* netCDF = new NetCDFHandler();

    netCDF->initialize(_gisSettings.utmZone);

    if (! netCDF->readProperties(fileNameWithPath.toStdString()))
    {
        logError("Wrong netcdf file: " + QString::fromStdString(netCDF->getMetadata()));
        return false;
    }

    addNetcdf(netCDF, fileNameWithPath, _gisSettings.utmZone);
    return true;
}
*/


bool CriteriaGeoProject::loadShapefile(const QString &fileNameWithPath, const QString &projectName)
{
    Crit3DShapeHandler *myShape = new(Crit3DShapeHandler);
    if (! myShape->open(fileNameWithPath.toStdString(), false))
    {
        _errorString = QString::fromStdString(myShape->errorString);
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
        formInfo.start("Fill raster...", 0);

    gis::Crit3DRasterGrid *newRaster = new gis::Crit3DRasterGrid();

    bool isOk = rasterizeShapeWithRef(refRaster, *newRaster, shapeHandler, field.toStdString());
    if (isOk)
    {
        gis::updateMinMaxRasterGrid(newRaster);
        setDefaultScale(newRaster->colorScale);

        if (showInfo)
            formInfo.setText("Add raster to map...");

        addRaster(newRaster, outputName, shapeHandler.getUtmZone());
    }

    if (showInfo)
        formInfo.close();

    return isOk;
}


bool CriteriaGeoProject::computeShapeAnomaly(Crit3DShapeHandler *shape1, Crit3DShapeHandler *shape2,
                                             const std::string &idStr, const std::string &field1,
                                             const std::string &field2, const QString &fileName)
{
    Crit3DShapeHandler *shapeAnomaly = new Crit3DShapeHandler();

    FormInfo formInfo;
    formInfo.start("Create shape...", 0);

    if (computeAnomaly(shapeAnomaly, shape1, shape2, idStr, field1, field2, fileName, _errorString))
    {
        addShapeFile(shapeAnomaly, fileName, "", shapeAnomaly->getUtmZone());
        formInfo.close();
        return true;
    }
    else
    {
        formInfo.close();
        logError();
        return false;
    }
}


bool CriteriaGeoProject::computeUnitCropMap(Crit3DShapeHandler *shapeCrop, Crit3DShapeHandler *shapeSoil,
                                            Crit3DShapeHandler *shapeMeteo, const std::string &idCrop,
                                            const std::string &idSoil, const std::string &idMeteo,
                                            const QString &ucmFileName, double cellSize, double threshold, bool isPrevailing)
{
    if (isPrevailing)
    {
        Crit3DShapeHandler *shapeUCM = new Crit3DShapeHandler();

        std::string errorStr;
        if (computeUcmPrevailing(*shapeUCM, *shapeCrop, *shapeSoil, *shapeMeteo, idCrop, idSoil, idMeteo,
                                 cellSize, threshold, ucmFileName, errorStr))
        {
            addShapeFile(shapeUCM, QString::fromStdString(shapeUCM->getFilepath()), "", shapeUCM->getUtmZone());
            return true;
        }
        else
        {
            _errorString = QString::fromStdString(errorStr);
            return false;
        }
    }
    else
    {
        #ifdef USE_GDAL
            /*
            Crit3DShapeHandler *shapeUCM = new Crit3DShapeHandler();

            if (computeUcmIntersection(shapeUCM, shapeCrop, shapeSoil, shapeMeteo, idCrop, idSoil, idMeteo, ucmFileName, &_errorString))
            {
                addShapeFile(shapeUCM, QString::fromStdString(shapeUCM->getFilepath()), "", shapeUCM->getUtmZone());
                return true;
            }
            else
            {
                logError(QString::fromStdString(_errorString));
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
    if (! shapeHandler)
    {
        _errorString = "Wrong shapefile.";
        return false;
    }

    int fieldRequired = 0;
    for (int i = 0; i < shapeHandler->getFieldNumbers(); i++)
    {
        QString fieldName = QString::fromStdString(shapeHandler->getFieldName(i)).toUpper();
        if ( fieldName == "ID_CASE" || fieldName == "ID_SOIL" || fieldName == "ID_CROP" || fieldName == "ID_METEO"
            || (fieldName == "HECTARES" || fieldName == "HA") )
        {
            fieldRequired++;
        }
    }
    if (fieldRequired < 5)
    {
        _errorString = "Mandatory fields: ID_CASE, ID_SOIL, ID_CROP, ID_METEO, HECTARES";
        return false;
    }

    QString dbName = QFileDialog::getSaveFileName(nullptr, "Save as", "", "DB files (*.db)");
    if (dbName == "") return false;

    QFile dbFile(dbName);
    if (dbFile.exists())
    {
        if (!dbFile.remove())
        {
            _errorString = "Remove file failed: " + dbName + "\n" + dbFile.errorString();
            return false;
        }
    }

    FormInfo formInfo;
    if (showInfo)
        formInfo.start("Extract computational units in: " + dbName, 0);

    bool result = writeUcmListToDb(*shapeHandler, dbName, _errorString);

    if (showInfo)
        formInfo.close();

    return result;
}


bool CriteriaGeoProject::assignIdCase(Crit3DShapeHandler* shapeHandler)
{
    if (! shapeHandler)
        return false;

    int fieldRequired = 0;
    int fieldNumber = shapeHandler->getFieldNumbers();
    for (int i = 0; i < fieldNumber; ++i)
    {
        QString fieldName = QString::fromStdString(shapeHandler->getFieldName(i)).toUpper();
        if (fieldName == "ID_SOIL" || fieldName == "ID_CROP" || fieldName == "ID_METEO")
        {
            fieldRequired++;
        }
    }
    if (fieldRequired < 3)
    {
        _errorString = "Required fields: ID_SOIL, ID_CROP, ID_METEO";
        return false;
    }

    if (! shapeHandler->existField("ID_CASE"))
    {
        if (! shapeHandler->addField("ID_CASE", FTString, 20, 0))
        {
            _errorString = "Error creating ID_CASE field";
            return false;
        }
    }

    const int idCasePos = shapeHandler->getFieldPos("ID_CASE");
    const int cropPos = shapeHandler->getFieldPos("ID_CROP");
    const int soilPos = shapeHandler->getFieldPos("ID_SOIL");
    const int meteoPos = shapeHandler->getFieldPos("ID_METEO");
    const int nrShapes = shapeHandler->getShapeCount();

    // fill ID_CASE
    for (int shapeNr = 0; shapeNr < nrShapes; shapeNr++)
    {
        std::string cropStr = shapeHandler->readStringAttribute(shapeNr, cropPos);
        if (cropStr == "-9999") cropStr = "";

        std::string soilStr = shapeHandler->readStringAttribute(shapeNr, soilPos);
        if (soilStr == "-9999") soilStr = "";

        std::string meteoStr = shapeHandler->readStringAttribute(shapeNr, meteoPos);
        if (meteoStr == "-9999") meteoStr = "";

        // check ID CASE
        std::string caseStr = "";
        if (!meteoStr.empty() && !soilStr.empty() && !cropStr.empty())
        {
            caseStr =
                "M" + meteoStr +
                "S" + soilStr +
                "C" + cropStr;
        }

        // write ID CASE
        if (! shapeHandler->writeStringAttribute(shapeNr, idCasePos, caseStr.c_str()))
        {
            _errorString = "Error writing ID_CASE field for shape nr. " + QString::number(shapeNr);
            return false;
        }
    }

    return true;
}


bool CriteriaGeoProject::createShapeFromCsv(int shapeIndex, const QString &fileCsv,
                                            const QString &fileCsvFormat, const QString &outputFileName)
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
        _errorString = "Ivalid Computational Units Map - missing ID_CASE";
        return false;
    }

    FormInfo formInfo;
    formInfo.start("Create shapefile...", 0);

    bool isOk = shapeFromCsv(*shapeHandler, fileCsv, fileCsvFormat, outputFileName, _errorString);

    formInfo.close();

    return isOk;
}


int CriteriaGeoProject::createShapeOutput(const QDate &dateComputation, const QString &outputName)
{
    FormInfo formInfo;
    formInfo.start("Create CSV file...", 0);
    qApp->processEvents();

    QString outputCsvFileName = output.path + "tmp/" + outputName +".csv";
    int result = output.createCsvFileFromGUI(dateComputation, outputCsvFileName);
    if (result != CRIT1D_OK)
    {
        formInfo.close();
        return result;
    }

    formInfo.setText("Create output map...");
    qApp->processEvents();
    result = output.createShapeFileFromGUI();
    if (result != CRIT1D_OK)
    {
        formInfo.close();
        return result;
    }

    formInfo.close();

    // clean .csv
    QFile::remove(output.path + "tmp/" + outputName +".csv");

    return result;
}


//--------------------------------------------------------------
// LOG
//--------------------------------------------------------------

void CriteriaGeoProject::logError() const
{
    logError(_errorString);
}

void CriteriaGeoProject::logError(const QString &errorString) const
{
    QMessageBox::critical(nullptr, "ERROR", errorString);
}

void CriteriaGeoProject::logWarning(const QString &warningString) const
{
    QMessageBox::warning(nullptr, "WARNING", warningString);
}

void CriteriaGeoProject::logInfo(const QString &infoString) const
{
    QMessageBox::information(nullptr, "CRITERIA-GEO", infoString);
}

