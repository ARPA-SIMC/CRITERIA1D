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
    #ifndef CRITERIAOUTPUTPROJECT_H
        #include "criteriaOutputProject.h"
    #endif

    #include <QString>
    #include <vector>

    class CriteriaGeoProject
    {
    private:

    public:
        gis::Crit3DGisSettings gisSettings;
        std::vector<GisObject*> objectList;
        CriteriaOutputProject output;

        CriteriaGeoProject();

        bool loadRaster(QString fileNameWithPath);
        bool loadShapefile(QString fileNameWithPath, QString projectName);
        bool loadNetcdf(QString fileNameWithPath);

        void addRaster(gis::Crit3DRasterGrid *myRaster, QString fileNameWithPath, int utmZone);
        void addNetcdf(NetCDFHandler *myNetcdf, QString fileNameWithPath, int utmZone);
        void addShapeFile(Crit3DShapeHandler *myShape, QString fileNameWithPath, QString projectName, int utmZone);

        void getRasterFromShape(Crit3DShapeHandler &shape, QString field, QString outputName, double cellSize, bool showInfo);

        bool createShapeFromCsv(int shapeIndex, QString fileCsv, QString fileCsvFormat, QString outputFileName, QString &errorStr);

        bool extractUcmListToDb(Crit3DShapeHandler* shapeHandler, bool showInfo);

        bool addUnitCropMap(Crit3DShapeHandler *crop, Crit3DShapeHandler *soil, Crit3DShapeHandler *meteo,
                            std::string idCrop, std::string idSoil, std::string idMeteo,
                            double cellSize, double threshold, QString ucmFileName, bool isPrevailing, bool showInfo);

        bool createRaster(QString shapeFileName, std::string shapeField, QString resolution, QString outputName, QString &error);

        void logError(QString errorString);

        int createShapeOutput(QDate dateComputation, QString outputName);
    };


#endif // CRITERIA_GEO_PROJECT_H
