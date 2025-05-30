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

        bool loadRaster(const QString &fileNameWithPath, QString &errorStr);
        bool loadShapefile(QString fileNameWithPath, QString projectName);
        //bool loadNetcdf(QString fileNameWithPath);

        void addRaster(gis::Crit3DRasterGrid *myRaster, QString fileNameWithPath, int utmZone);
        void addShapeFile(Crit3DShapeHandler *myShape, QString fileNameWithPath, QString projectName, int utmZone);
        //void addNetcdf(NetCDFHandler *myNetcdf, QString fileNameWithPath, int utmZone);

        bool newRasterFromShape(Crit3DShapeHandler &shapeHandler, const QString &field,
                                const QString &outputName, double cellSize, bool showInfo);

        bool fillRasterFromShape(Crit3DShapeHandler &shapeHandler, gis::Crit3DRasterGrid &refRaster,
                                 const QString &field, const QString &outputName, bool showInfo);

        bool createShapeFromCsv(int shapeIndex, QString fileCsv, QString fileCsvFormat, QString outputFileName, QString &errorStr);

        bool extractUcmListToDb(Crit3DShapeHandler* shapeHandler, bool showInfo);

        bool computeShapeAnomaly(Crit3DShapeHandler *shape1, Crit3DShapeHandler *shape2,
                            std::string id, std::string field1, std::string field2, QString fileName);

        bool computeUnitCropMap(Crit3DShapeHandler *shapeCrop, Crit3DShapeHandler *shapeSoil, Crit3DShapeHandler *shapeMeteo,
                            std::string idCrop, std::string idSoil, std::string idMeteo,
                            double cellSize, double threshold, QString ucmFileName, bool isPrevailing);

        void logError(QString errorString);
        void logWarning(QString errorString);

        int createShapeOutput(QDate dateComputation, QString outputName);
    };


#endif // CRITERIA_GEO_PROJECT_H
