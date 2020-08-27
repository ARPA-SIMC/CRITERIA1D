#ifndef CRITERIA_GEO_PROJECT_H
#define CRITERIA_GEO_PROJECT_H

    #ifndef GIS_H
        #include "gis.h"
    #endif
    #ifndef SHAPEHANDLER_H
        #include "shapeHandler.h"
    #endif
    #ifndef GISOBJECT_H
        #include "gisObject.h"
    #endif

    #include <QString>
    #include <vector>

    class CriteriaGeoProject
    {
    private:

    public:
        gis::Crit3DGisSettings gisSettings;
        std::vector<GisObject*> objectList;

        CriteriaGeoProject();

        bool loadRaster(QString fileName);
        bool loadShapefile(QString fileName);
        void addRaster(gis::Crit3DRasterGrid *myRaster, QString fileName, int utmZone);
        void addShapeFile(Crit3DShapeHandler *myShape, QString fileName, int utmZone);

        void getRasterFromShape(Crit3DShapeHandler &shape, QString field, QString outputName, double cellSize, bool showInfo);

        bool createShapeFromCsv(int pos, QString fileCsv, QString fileCsvRef, QString outputFileName);

        bool extractUcmListToDb(Crit3DShapeHandler* shapeHandler, bool showInfo);

        bool addUnitCropMap(Crit3DShapeHandler *crop, Crit3DShapeHandler *soil, Crit3DShapeHandler *meteo,
                            std::string idCrop, std::string idSoil, std::string idMeteo, double cellSize,
                            QString ucmFileName, bool isPrevailing, bool showInfo);

        bool createGeoTIFF(QString shapeFileName, std::string shapeField);

        void logError(std::string errorString);
        void logError(QString errorString);
    };


#endif // CRITERIA_GEO_PROJECT_H
