TEMPLATE = subdirs

SUBDIRS =   ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/soilFluxes3D  \
            ../../agrolib/gis ../../agrolib/meteo ../../agrolib/soil ../../agrolib/crop \
            ../../agrolib/utilities ../../agrolib/dbMeteoGrid  \
            ../../agrolib/commonChartElements ../../agrolib/qcustomplot ../../agrolib/commonDialogs  \
            ../../agrolib/meteoWidget ../../agrolib/soilWidget  \
            ../../src/carbonNitrogen ../../src/criteria1DModel  ../../src/criteria1DWidget  \
            ../CRITERIA1D_PRO

CONFIG += ordered
