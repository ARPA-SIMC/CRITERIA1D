#----------------------------------------------------------------------------
#
#   CRITERIA 1D-GEO
#   interface to set geographical projects for soil water balance 1D
#
#   Dependencies: GDAL
#   See gdal.pri for more info
#   Comment CONFIG += GDAL to compile without GDAL library
#
#   This project is part of CRITERIA-1D distribution
#
#-----------------------------------------------------------------------------


QT    += core gui network widgets sql
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

TEMPLATE = app

VERSION = 1.8.7

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/CRITERIA_GEO
    } else {
        TARGET = release/CRITERIA_GEO
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/CRITERIA_GEO
    } else {
        TARGET = release/CRITERIA_GEO
    }
}
win32:{
    TARGET = CRITERIA_GEO
}

INCLUDEPATH +=  ../../mapGraphics \
                ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis ../../agrolib/meteo   \
                ../../agrolib/utilities ../../agrolib/shapeUtilities  \
                ../../agrolib/shapeHandler ../../agrolib/shapeHandler/shapelib  ../../agrolib/netcdfHandler \
                ../../agrolib/criteriaOutput ../../agrolib/graphics ../../agrolib/commonDialogs

CONFIG += debug_and_release

# comment to compile without GDAL library
CONFIG += GDAL

GDAL {
    DEFINES += GDAL
    INCLUDEPATH += ../../agrolib/gdalHandler

    CONFIG(debug, debug|release) {
        LIBS += -L../../agrolib/gdalHandler/debug -lgdalHandler
    } else {
        LIBS += -L../../agrolib/gdalHandler/release -lgdalHandler
    }
    include(../../agrolib/gdal.pri)
}

CONFIG(debug, debug|release) {

    LIBS += -L../../mapGraphics/debug -lMapGraphics
    LIBS += -L../../agrolib/commonDialogs/debug -lcommonDialogs
    LIBS += -L../../agrolib/graphics/debug -lgraphics
    LIBS += -L../../agrolib/criteriaOutput/debug -lcriteriaOutput
    LIBS += -L../../agrolib/netcdfHandler/debug -lnetcdfHandler
    win32:{
        LIBS += -L$$(NC4_INSTALL_DIR)/lib -lnetcdf
    }
    unix:{
        LIBS += -lnetcdf
    }
    macx:{
        LIBS += -L/usr/local/lib/ -lnetcdf
    }
    LIBS += -L../../agrolib/shapeUtilities/debug -lshapeUtilities
    LIBS += -L../../agrolib/shapeHandler/debug -lshapeHandler
    LIBS += -L../../agrolib/utilities/debug -lutilities
    LIBS += -L../../agrolib/meteo/debug -lmeteo
    LIBS += -L../../agrolib/gis/debug -lgis
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions

} else {

    LIBS += -L../../mapGraphics/release -lMapGraphics
    LIBS += -L../../agrolib/commonDialogs/release -lcommonDialogs
   LIBS += -L../../agrolib/graphics/release -lgraphics
    LIBS += -L../../agrolib/criteriaOutput/release -lcriteriaOutput
    LIBS += -L../../agrolib/netcdfHandler/release -lnetcdfHandler
    win32:{
        LIBS += -L$$(NC4_INSTALL_DIR)/lib -lnetcdf
    }
    unix:{
        LIBS += -lnetcdf
    }
    macx:{
        LIBS += -L/usr/local/lib/ -lnetcdf
    }
    LIBS += -L../../agrolib/shapeUtilities/release -lshapeUtilities
    LIBS += -L../../agrolib/shapeHandler/release -lshapeHandler
    LIBS += -L../../agrolib/utilities/release -lutilities
    LIBS += -L../../agrolib/meteo/release -lmeteo
    LIBS += -L../../agrolib/gis/release -lgis
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
}

HEADERS += \
    criteriaGeoProject.h \
    dialogDbfNewCol.h \
    dialogDbfTable.h \
    dialogOutputMap.h \
    dialogSelectField.h \
    dialogShapeAnomaly.h \
    dialogShapeProperties.h \
    dialogUcmIntersection.h \
    dialogUcmPrevailing.h \
    gisObject.h \
    mainWindow.h \
    tabMap.h \
    tabSymbology.h \
    tableDbf.h

SOURCES += \
    criteriaGeoProject.cpp \
    dialogDbfNewCol.cpp \
    dialogDbfTable.cpp \
    dialogOutputMap.cpp \
    dialogSelectField.cpp \
    dialogShapeAnomaly.cpp \
    dialogShapeProperties.cpp \
    dialogUcmIntersection.cpp \
    dialogUcmPrevailing.cpp \
    mainWindow.cpp \
    gisObject.cpp \
    main.cpp \
    tabMap.cpp \
    tabSymbology.cpp \
    tableDbf.cpp


FORMS += \
    mainWindow.ui


RESOURCES += CRITERIAGEO.ico

win32:
{
    RC_ICONS = CRITERIAGEO.ico
}
