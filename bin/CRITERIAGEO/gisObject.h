#ifndef GISOBJECT_H
#define GISOBJECT_H

    #ifndef GIS_H
        #include "gis.h"
    #endif

    #ifndef SHAPEHANDLER_H
        #include "shapeHandler.h"
    #endif
    #ifndef NETCDFHANDLER_H
        #include "netcdfHandler.h"
    #endif

    #include <QString>

    enum gisObjectType{gisObjectNone, gisObjectRaster, gisObjectShape, gisObjectNetcdf};

    class GisObject
    {
    private:
        QString fileNameWithPath;

        gis::Crit3DRasterGrid* rasterPtr;
        Crit3DShapeHandler* shapePtr;
        NetCDFHandler* netcdfPtr;

        void initialize();

    public:
        gis::Crit3DGisSettings gisSettings;
        gisObjectType type;
        QString fileName;
        QString projectName;
        bool isSelected;

        GisObject();

        gis::Crit3DRasterGrid* getRaster();
        Crit3DShapeHandler* getShapeHandler();
        NetCDFHandler* getNetcdfHandler();

        void setRaster(QString filename, gis::Crit3DRasterGrid* rasterPtr, int utmZone);
        void setNetcdf(QString fileNameWithPath, NetCDFHandler* _netcdfPtr, int utmZone);
        void setShapeFile(QString filename, QString projectName, Crit3DShapeHandler* shapePtr, int utmZone);

        void close();

        QString getFileNameWithPath() const;
    };


#endif // GISOBJECT_H
