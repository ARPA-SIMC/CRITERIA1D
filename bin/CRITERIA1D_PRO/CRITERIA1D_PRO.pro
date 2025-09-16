#-----------------------------------------------------------
#
#   CRITERIA1D PRO
#   This project is part of CRITERIA3D distribution
#
#-----------------------------------------------------------

QT       += widgets charts sql xml printsupport

TEMPLATE = app
VERSION = 1.9.0
QMAKE_TARGET_COPYRIGHT = "\\251 2025 ARPAE ER - Climate Observatory"

CONFIG += debug_and_release

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/CRITERIA1D_PRO
    } else {
        TARGET = release/CRITERIA1D_PRO
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/CRITERIA1D_PRO
    } else {
        TARGET = release/CRITERIA1D_PRO
    }
}
win32:{
    TARGET = CRITERIA1D_PRO
}

INCLUDEPATH +=  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis  \
                ../../agrolib/meteo ../../agrolib/dbMeteoGrid ../../agrolib/utilities ../../agrolib/waterTable \
                ../../agrolib/soil ../../agrolib/crop ../../agrolib/qcustomplot ../../agrolib/commonDialogs \
                ../../agrolib/commonChartElements ../../agrolib/meteoWidget ../../agrolib/soilWidget \
                ../../src/carbonNitrogen  ../../src/criteria1DModel ../../src/criteria1DWidget

CONFIG(debug, debug|release) {
    LIBS += -L../../src/criteria1DWidget/debug -lcriteria1DWidget
    LIBS += -L../../src/criteria1DModel/debug -lcriteria1DModel
    LIBS += -L../../agrolib/soilWidget/debug -lsoilWidget
    LIBS += -L../../agrolib/meteoWidget/debug -lmeteoWidget
    LIBS += -L../../agrolib/commonDialogs/debug -lcommonDialogs
    LIBS += -L../../agrolib/waterTable/debug -lwaterTable
    LIBS += -L../../agrolib/commonChartElements/debug -lcommonChartElements
    LIBS += -L../../agrolib/qcustomplot/debug -lqcustomplot
    LIBS += -L../../agrolib/dbMeteoGrid/debug -ldbMeteoGrid
    LIBS += -L../../agrolib/crop/debug -lcrop
    LIBS += -L../../agrolib/utilities/debug -lutilities
    LIBS += -L../../src/carbonNitrogen/debug -lcarbonNitrogen
    LIBS += -L../../agrolib/soil/debug -lsoil
    LIBS += -L../../agrolib/meteo/debug -lmeteo
    LIBS += -L../../agrolib/gis/debug -lgis
    LIBS += -L../../agrolib/soilFluxes3D/debug -lsoilFluxes3D
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
} else {

    LIBS += -L../../src/criteria1DWidget/release -lcriteria1DWidget
    LIBS += -L../../src/criteria1DModel/release -lcriteria1DModel
    LIBS += -L../../agrolib/soilWidget/release -lsoilWidget
    LIBS += -L../../agrolib/meteoWidget/release -lmeteoWidget
    LIBS += -L../../agrolib/commonDialogs/release -lcommonDialogs
    LIBS += -L../../agrolib/waterTable/release -lwaterTable
    LIBS += -L../../agrolib/commonChartElements/release -lcommonChartElements
    LIBS += -L../../agrolib/qcustomplot/release -lqcustomplot
    LIBS += -L../../agrolib/dbMeteoGrid/release -ldbMeteoGrid
    LIBS += -L../../agrolib/crop/release -lcrop
    LIBS += -L../../agrolib/utilities/release -lutilities
    LIBS += -L../../src/carbonNitrogen/release -lcarbonNitrogen
    LIBS += -L../../agrolib/soil/release -lsoil
    LIBS += -L../../agrolib/meteo/release -lmeteo
    LIBS += -L../../agrolib/gis/release -lgis
    LIBS += -L../../agrolib/soilFluxes3D/release -lsoilFluxes3D
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
}

SOURCES += \
        main.cpp


RESOURCES += criteria1D.ico

win32:
{
    RC_ICONS = criteria1D.ico
}

