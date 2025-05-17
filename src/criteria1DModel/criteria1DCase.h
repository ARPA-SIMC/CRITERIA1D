#ifndef CRITERIA1DCASE_H
#define CRITERIA1DCASE_H

    #ifndef SOIL_H
        #include "soil.h"
    #endif
    #ifndef CARBON_H
        #include "carbonNitrogen.h"
    #endif
    #ifndef CROP_H
        #include "crop.h"
    #endif
    #ifndef METEOPOINT_H
        #include "meteoPoint.h"
    #endif
    #ifndef COMPUTATIONUNITSDB_H
        #include "computationUnitsDb.h"
    #endif
    #ifndef WATERTABLEDB_H
        #include "waterTableDb.h"
    #endif

    #include <QString>
    #include <vector>

    /*!
    * \brief daily output of Criteria1D
    * \note all variables are in [mm] (except dailyWaterTable [m])
    */
    class Crit1DOutput
    {
    public:
        double dailyPrec;
        double dailySurfaceRunoff;
        double dailySoilWaterContent;
        double dailySurfaceWaterContent;
        double dailyLateralDrainage;
        double dailyDrainage;
        double dailyIrrigation;
        double dailyEt0;
        double dailyMaxEvaporation;
        double dailyEvaporation;
        double dailyMaxTranspiration;
        double dailyTranspiration;
        double dailyAvailableWater;
        double dailyFractionAW;
        double dailyReadilyAW;
        double dailyCapillaryRise;
        double dailyWaterTable;                 // [m]
        double dailyBalance;

        Crit1DOutput();
        void initialize();
    };


    class Crit1DCase
    {
    public:
        Crit1DCompUnit unit;
        bool computeFactorOfSafety;
        int nrMissingPrec;

        // SOIL
        soil::Crit3DSoil mySoil;
        std::vector<soil::Crit1DLayer> soilLayers;
        std::vector<Crit3DCarbonNitrogenLayer> carbonNitrogenLayers;
        soil::Crit3DFittingOptions fittingOptions;

        std::vector<double> prevWaterContent;           // [mm]

        // CROP
        Crit3DCrop crop;

        // WATERTABLE
        WaterTableParameters waterTableParameters;

        // WHEATER
        Crit3DMeteoPoint meteoPoint;

        // OUTPUT
        Crit1DOutput output;

        Crit1DCase();

        bool initializeSoil(std::string &error);
        bool initializeWaterContent(const Crit3DDate &myDate);
        bool fillWaterTableData();

        bool computeDailyModel(const Crit3DDate &myDate, std::string &error);

        double getVolumetricWaterContent(double computationDepth);
        double getDegreeOfSaturation(double computationDepth);
        double getWaterPotential(double computationDepth);
        double getAvailableWaterFraction(double computationDepth);
        double getSlopeStability(double computationDepth);

        double getWaterDeficitSum(double computationDepth);
        double getWaterCapacitySum(double computationDepth);
        double getAvailableWaterSum(double computationDepth);
        double getSoilWaterIndex(double computationDepth);

        double getPloughedSoilDepth() { return _ploughedSoilDepth; }

    private:
        double _minLayerThickness;       // [m]
        double _geometricFactor;         // [-]

        double _ploughedSoilDepth;       // [m]

        double _lx, _ly;                 // [m]
        double _area;                    // [m2]

        bool initializeNumericalFluxes(std::string &error);
        bool computeNumericalFluxes(const Crit3DDate &myDate, std::string &error);

        bool computeWaterFluxes(const Crit3DDate &myDate, std::string &error);

        double checkIrrigationDemand(int doy, double currentPrec, double precForecast, double maxTranspiration);

        void storeWaterContent();
        void restoreWaterContent();
        double getTotalWaterContent();

    };



#endif // CRITERIA1DCASE_H
