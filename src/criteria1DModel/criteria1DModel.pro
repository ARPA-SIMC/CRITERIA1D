#----------------------------------------------------------------
#
#   criteria1DModel library
#
#   Water balance 1D
#   algorithms for soil water infiltration, redistribution,
#   capillary rise, crop water demand and irrigation.
#
#   This library is part of CRITERIA-1D distribution
#
#----------------------------------------------------------------

QT      += sql xml
QT      -= gui

TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++17

CONFIG += debug_and_release
DEFINES += _CRT_SECURE_NO_WARNINGS

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/criteria1DModel
    } else {
        TARGET = release/criteria1DModel
    }
}
win32:{
    TARGET = criteria1DModel
}

INCLUDEPATH +=  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis ../../agrolib/meteo \
                ../../agrolib/dbMeteoGrid ../../agrolib/soil ../../agrolib/crop ../../agrolib/utilities \
                ../../agrolib/waterTable ../../agrolib/soilFluxes3D ../carbonNitrogen

HEADERS += \
    criteria1DCase.h \
    criteria1DError.h \
    criteria1DMeteo.h \
    criteria1DProject.h \
    carbonNitrogenModel.h \
    water1D.h

SOURCES += \
    criteria1DCase.cpp \
    criteria1DMeteo.cpp \
    criteria1DProject.cpp \
    carbonNitrogenModel.cpp \
    water1D.cpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}

