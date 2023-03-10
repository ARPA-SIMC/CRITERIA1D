/*!
    \file gisObject.cpp

    \abstract Praga-Gis object class

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

#include "gisObject.h"
#include "utilities.h"


GisObject::GisObject()
{
    this->initialize();
}


QString GisObject::getFileNameWithPath() const
{
    return fileNameWithPath;
}

void GisObject::initialize()
{
    this->type = gisObjectNone;
    this->fileName = "";
    this->projectName = "";
    this->fileNameWithPath = "";
    this->isSelected = true;
    this->rasterPtr = nullptr;
    this->shapePtr = nullptr;
}


void GisObject::setRaster(QString fileNameWithPath, gis::Crit3DRasterGrid* _rasterPtr, int utmZone)
{
    this->type = gisObjectRaster;
    this->fileNameWithPath = fileNameWithPath;
    this->fileName = getFileName(fileNameWithPath);
    this->isSelected = true;

    this->rasterPtr = _rasterPtr;
    this->gisSettings.utmZone = utmZone;
}


void GisObject::setNetcdf(QString fileNameWithPath, NetCDFHandler* _netcdfPtr, int utmZone)
{
    this->type = gisObjectNetcdf;
    this->fileNameWithPath = fileNameWithPath;
    this->fileName = getFileName(fileNameWithPath);
    this->isSelected = true;

    this->netcdfPtr = _netcdfPtr;
    this->gisSettings.utmZone = utmZone;
}


void GisObject::setShapeFile(QString fileNameWithPath, QString projectName, Crit3DShapeHandler* _shapePtr, int utmZone)
{
    this->type = gisObjectShape;
    this->fileNameWithPath = fileNameWithPath;
    this->fileName = getFileName(fileNameWithPath);
    this->projectName = projectName;
    this->isSelected = true;

    this->shapePtr = _shapePtr;
    this->gisSettings.utmZone = utmZone;
}


gis::Crit3DRasterGrid* GisObject::getRaster()
{
    return this->rasterPtr;
}


Crit3DShapeHandler* GisObject::getShapeHandler()
{
    return this->shapePtr;
}


NetCDFHandler* GisObject::getNetcdfHandler()
{
    return this->netcdfPtr;
}


void GisObject::close()
{

    if (this->type == gisObjectRaster)
    {
        delete rasterPtr;
    }

    if (this->type == gisObjectShape)
    {
        shapePtr->close();
    }

    if (this->type == gisObjectNetcdf)
    {
        netcdfPtr->close();
    }

    this->initialize();
}


