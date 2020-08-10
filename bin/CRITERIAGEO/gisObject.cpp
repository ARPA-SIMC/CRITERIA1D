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


GisObject::GisObject()
{
    this->initialize();
}


void GisObject::initialize()
{
    this->type = gisObjectNone;
    this->fileName = "";
    this->fileNameWithPath = "";
    this->isSelected = true;
    this->rasterPtr = nullptr;
    this->shapePtr = nullptr;
}


void GisObject::setRaster(QString fileNameWithPath, gis::Crit3DRasterGrid* rasterPtr, int utmZone)
{
    this->type = gisObjectRaster;
    this->fileNameWithPath = fileNameWithPath;
    this->fileName = getFileName(fileNameWithPath);
    this->isSelected = true;

    this->rasterPtr = rasterPtr;
    this->gisSettings.utmZone = utmZone;
}


void GisObject::setShapeFile(QString fileNameWithPath, Crit3DShapeHandler* shapePtr, int utmZone)
{
    this->type = gisObjectShape;
    this->fileNameWithPath = fileNameWithPath;
    this->fileName = getFileName(fileNameWithPath);
    this->isSelected = true;

    this->shapePtr = shapePtr;
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


void GisObject::close()
{
    if (this->type == gisObjectRaster)
    {
        delete rasterPtr;
    }
    else if (this->type == gisObjectShape)
    {
        shapePtr->close();
    }

    this->initialize();
}


QString getFileName(QString fileNameWithPath)
{
    QString c;
    QString fileName = "";
    for (int i = fileNameWithPath.length()-1; i >= 0; i--)
    {
        c = fileNameWithPath.mid(i,1);
        if ((c != "\\") && (c != "/"))
            fileName = c + fileName;
        else
            return fileName;
    }
    return fileName;
}

