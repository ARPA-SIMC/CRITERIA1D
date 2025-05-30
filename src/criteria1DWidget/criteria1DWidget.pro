#----------------------------------------------------
#
#   criteria1DWidget library
#   This project is part of CRITERIA-3D distribution
#
#----------------------------------------------------

QT  += widgets sql xml charts printsupport

TEMPLATE = lib
CONFIG += staticlib

CONFIG += debug_and_release

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/criteria1DWidget
    } else {
        TARGET = release/criteria1DWidget
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/criteria1DWidget
    } else {
        TARGET = release/criteria1DWidget
    }
}
win32:{
    TARGET = criteria1DWidget
}

INCLUDEPATH += ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/utilities  \
            ../../agrolib/gis ../../agrolib/meteo ../../agrolib/soil ../../agrolib/dbMeteoGrid  \
            ../../agrolib/crop ../../agrolib/qcustomplot ../../agrolib/commonDialogs \
            ../../agrolib/commonChartElements ../../agrolib/meteoWidget ../../agrolib/soilWidget \
            ../carbonNitrogen ../criteria1DModel


SOURCES += \
    criteria1DWidget.cpp \
    dialogNewCrop.cpp \
    dialogNewProject.cpp \
    tabCarbonNitrogen.cpp \
    tabIrrigation.cpp \
    tabLAI.cpp \
    tabRootDensity.cpp \
    tabRootDepth.cpp \
    tabWaterContent.cpp

HEADERS += \
    criteria1DWidget.h \
    dialogNewCrop.h \
    dialogNewProject.h \
    tabCarbonNitrogen.h \
    tabIrrigation.h \
    tabLAI.h \
    tabRootDensity.h \
    tabRootDepth.h \
    tabWaterContent.h
