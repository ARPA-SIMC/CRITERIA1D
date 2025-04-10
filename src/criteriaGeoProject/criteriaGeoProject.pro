#----------------------------------------------------------------------------
#
#   CRITERIA GEO Project library
#
#   This project is part of CRITERIA-1D distribution
#
#-----------------------------------------------------------------------------

QT    += core gui network widgets sql
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat


TEMPLATE = lib
CONFIG += staticlib


DEFINES += _CRT_SECURE_NO_WARNINGS

CONFIG += debug_and_release


unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/criteriaGeoProject
    } else {
        TARGET = release/criteriaGeoProject
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/criteriaGeoProject
    } else {
        TARGET = release/criteriaGeoProject
    }
}
win32:{
    TARGET = criteriaGeoProject
}


INCLUDEPATH +=  ../../mapGraphics \
                ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis ../../agrolib/meteo   \
                ../../agrolib/utilities ../../agrolib/shapeUtilities  \
                ../../agrolib/shapeHandler ../../agrolib/shapeHandler/shapelib  ../../agrolib/netcdfHandler \
                ../../agrolib/criteriaOutput ../../agrolib/graphics ../../agrolib/commonDialogs


# comment to compile without GDAL library
CONFIG += GDAL

GDAL {
    DEFINES += GDAL
    INCLUDEPATH += ../../agrolib/gdalHandler
    include(../../agrolib/gdal.pri)
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
    gisObject.cpp \
    tabMap.cpp \
    tabSymbology.cpp \
    tableDbf.cpp

