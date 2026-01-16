#ifndef CRITERIA1DPROJECT_H
#define CRITERIA1DPROJECT_H

    #ifndef LOGGER_H
        #include "logger.h"
    #endif
    #ifndef DBMETEOGRID_H
        #include "dbMeteoGrid.h"
    #endif
    #ifndef CRITERIA1DCASE_H
        #include "criteria1DCase.h"
    #endif
    #ifndef CARBON_NITROGEN_MODEL_H
        #include "carbonNitrogenModel.h"
    #endif

    #include <fstream>

    #define CRITERIA_VERSION "v2.0.0 (2026)"

    class Crit1DProject
    {

    public:
        QSqlDatabase dbCrop;
        QSqlDatabase dbSoil;
        QSqlDatabase dbMeteo;
        Crit3DMeteoGridDbHandler* observedMeteoGrid;

        // dates
        QDate firstSimulationDate;
        QDate lastSimulationDate;

        bool isProjectLoaded;
        QString projectError;
        Logger logger;

        QString path;

        // database
        QString dbCropName;
        QString dbSoilName;
        QString dbOutputName;
        QString dbMeteoName;
        QString dbForecastName;
        QString dbWaterTableName;
        QString dbComputationUnitsName;

        Crit1DCase myCase;
        Crit1DCarbonNitrogenProfile myCarbonNitrogenProfile;

        // soil
        std::vector<soil::Crit3DTextureClass> texturalClassList;
        std::vector<soil::Crit3DGeotechnicsClass> geotechnicsClassList;

        std::vector<Crit1DCompUnit> compUnitList;

        Crit1DProject();

        void initialize();
        int initializeProject(const QString &settingsFileName);

        int computeAllUnits();
        bool computeUnit(const Crit1DCompUnit& myUnit);

        bool setSoil(const QString &soilCode, QString &errorStr);

        void setIsXmlGrid(bool isGrid) { _isXmlMeteoGrid = isGrid; }
        bool isXmlMeteoGrid() { return _isXmlMeteoGrid; }

    private:
        bool _computeAllSoilDepth;
        bool _isXmlMeteoGrid;

        double _computationSoilDepth;                    // [m]

        QString projectName;
        QString configFileName;
        bool _addDateTimeLogFile;

        // save/restart
        bool _isSaveState;
        bool _isRestart;

        // forecast/climate type
        bool _isYearlyStatistics;
        bool _isMonthlyStatistics;
        bool _isSeasonalForecast;
        bool _isEnsembleForecast;
        bool _isShortTermForecast;

        int _firstComputationMonth;
        int _daysOfForecast;
        int _nrComputationYears;
        int _nrSetupYears;

        std::vector<float> _irriSeries;                 // [mm]
        std::vector<float> _precSeries;                 // [mm]

        QString outputString;
        QString outputCsvFileName;
        std::ofstream outputCsvFile;

        // specific output
        bool isClimateOutput;
        std::vector<int> waterContentDepth;             // [cm]
        std::vector<int> degreeOfSaturationDepth;       // [cm]
        std::vector<int> waterPotentialDepth;           // [cm]
        std::vector<int> waterDeficitDepth;             // [cm]
        std::vector<int> awcDepth;                      // [cm]
        std::vector<int> availableWaterDepth;           // [cm]
        std::vector<int> fractionAvailableWaterDepth;   // [cm]
        std::vector<int> factorOfSafetyDepth;           // [cm]
        std::vector<int> soilWaterIndexDepth;           // [cm]

        // DATABASE
        QSqlDatabase dbForecast;
        QSqlDatabase dbOutput;
        QSqlDatabase dbState;

        Crit3DMeteoGridDbHandler* forecastMeteoGrid;

        void closeProject();
        bool readSettings();
        void closeAllDatabase();
        int openAllDatabase();
        void checkSimulationDates();

        bool setMeteoSqlite(const QString &idMeteo, const QString &idForecast);
        bool setMeteoXmlGrid(QString idMeteo, const QString &idForecast, unsigned int memberNr);

        bool setPercentileOutputCsv();

        void initializeIrrigationStatistics(const Crit3DDate &firstDate, const Crit3DDate &lastDate);
        void updateIrrigationStatistics(const Crit3DDate &myDate, int &currentYearIndex);
        bool computeIrrigationStatistics(unsigned int index, float irriRatio);
        bool computeEnsembleForecast(unsigned int unitIndex, float irriRatio);
        void updateEnsembleForecastOutput(const Crit3DDate &myDate, unsigned int memberNr);

        bool computeCase(unsigned int memberNr);
        bool computeUnit(unsigned int unitIndex, unsigned int memberNr);

        bool createOutputTable(QString &errorStr);
        bool createDbState(QString &errorStr);
        bool saveState(QString &errorStr);
        bool restoreState(const QString &dbStateName, QString &errorStr);
        void updateOutput(const Crit3DDate &myDate, bool isFirst);
        bool saveOutput(QString &errorStr);

    };

    QString getOutputStringNullZero(double value);
    bool setVariableDepth(const QList<QString> &depthList, std::vector<int> &variableDepth);


#endif // CRITERIA1DPROJECT_H
