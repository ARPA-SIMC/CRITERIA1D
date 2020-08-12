#ifndef CRITERIA1DPROJECT
#define CRITERIA1DPROJECT

    #ifndef CRITERIA1DSIMULATION_H
        #include "criteria1DSimulation.h"
    #endif

    #include <fstream>

    #define ERROR_SETTINGS_MISSING -1
    #define ERROR_SETTINGS_WRONGFILENAME -2
    #define ERROR_SETTINGS_MISSINGDATA -3
    #define ERROR_SETTINGS_PROXY -4
    #define ERROR_DBPARAMETERS -5
    #define ERROR_DBSOIL -6
    #define ERROR_DBMETEO_OBSERVED -7
    #define ERROR_DBMETEO_FORECAST -8
    #define ERROR_DBUNITS -9
    #define ERROR_DBOUTPUT -10
    #define ERROR_WRONGDATE -11
    #define ERROR_XMLGRIDMETEO_OBSERVED -12
    #define ERROR_XMLGRIDMETEO_FORECAST -13
    #define ERROR_PROPERTIES_DBMETEO_OBSERVED -14
    #define ERROR_PROPERTIES_DBMETEO_FORECAST -15

    #define ERROR_READ_UNITS -15

    #define ERROR_SOIL_PARAMETERS -20
    #define ERROR_SOIL_MISSING -21
    #define WARNING_SOIL 20

    #define ERROR_CROP_MISSING -30
    #define WARNING_CROP 30

    #define ERROR_METEO_OR_MODEL -40
    #define WARNING_METEO_OR_MODEL 40

    #define ERROR_UNKNOWN -99


    class Criteria1DProject
    {
        public:
            QString path;
            QString name;
            QString configFileName;

            QString dbCropName;
            QString dbSoilName;
            QString dbOutputName;
            QString dbUnitsName;

            QString dbMeteoName;
            QString dbForecastName;

            QString logFileName;
            std::ofstream logFile;
            std::ofstream outputFile;

            bool isProjectLoaded;

            QString outputCsvFileName;
            QString outputCsvPath;

            QString projectError;

            Crit1DSimulation criteriaSimulation;

            std::vector<Crit1DUnit> unitList;

            Criteria1DProject();

            void initialize();
            int initializeProject(QString myFileName);
            bool initializeCsvOutputFile();

            bool readSettings();
            int openAllDatabase();

            void closeProject();
            void closeAllDatabase();

            int compute();
            bool runSeasonalForecast(unsigned int index, double irriRatio);

            bool setLogFile();
            void logInfo(QString logStr);
            void logError();
            void logError(QString myErrorStr);
    };

#endif // CRITERIA1DPROJECT

