#ifndef CRITERIA_GEO_PROJECT_H
#define CRITERIA_GEO_PROJECT_H

    #define CRITERIA_GEO_VERSION "v2.0.2 (2026)"

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
        QString _errorString;
        gis::Crit3DGisSettings _gisSettings;

    public:
        std::vector<GisObject*> objectList;
        CriteriaOutputProject output;

        CriteriaGeoProject();

        gis::Crit3DGisSettings& getGisSettings()
        { return _gisSettings; }

        bool loadRaster(const QString &fileNameWithPath);

        bool loadShapefile(const QString &fileNameWithPath, const QString &projectName);
        //bool loadNetcdf(QString fileNameWithPath);

        void addRaster(gis::Crit3DRasterGrid *myRaster, QString fileNameWithPath, int utmZone);
        void addShapeFile(Crit3DShapeHandler *myShape, QString fileNameWithPath, QString projectName, int utmZone);
        //void addNetcdf(NetCDFHandler *myNetcdf, QString fileNameWithPath, int utmZone);

        bool newRasterFromShape(Crit3DShapeHandler &shapeHandler, const QString &field,
                                const QString &outputName, double cellSize, bool showInfo);

        bool fillRasterFromShape(Crit3DShapeHandler &shapeHandler, gis::Crit3DRasterGrid &refRaster,
                                 const QString &field, const QString &outputName, bool showInfo);

        bool createShapeFromCsv(int shapeIndex, const QString &fileCsv, const QString &fileCsvFormat,
                                const QString &outputFileName);

        bool extractUcmListToDb(Crit3DShapeHandler* shapeHandler, bool showInfo);

        bool assignIdCase(Crit3DShapeHandler* shapeHandler);

        bool computeShapeAnomaly(Crit3DShapeHandler *shape1, Crit3DShapeHandler *shape2,
                                 const std::string &idStr, const std::string &field1,
                                 const std::string &field2, const QString &fileName);

        bool computeUnitCropMap(Crit3DShapeHandler *shapeCrop, Crit3DShapeHandler *shapeSoil, Crit3DShapeHandler *shapeMeteo,
                            const std::string &idCrop, const std::string &idSoil, const std::string &idMeteo,
                            const QString &ucmFileName, double cellSize, double threshold, bool isPrevailing);

        void logError() const;
        void logError(const QString &errorString) const;
        void logWarning(const QString &warningString) const;
        void logInfo(const QString &infoString) const;

        int createShapeOutput(const QDate &dateComputation, const QString &outputName);
    };


#endif // CRITERIA_GEO_PROJECT_H
