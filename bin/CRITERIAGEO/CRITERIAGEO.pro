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

TEMPLATE = app

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
                ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis ../../agrolib/crop   \
                ../../agrolib/utilities ../../agrolib/shapeUtilities  \
                ../../agrolib/shapeHandler ../../agrolib/shapeHandler/shapelib  \
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
    LIBS += -L../../agrolib/criteriaOutput/debug -lcriteriaOutput
    LIBS += -L../../agrolib/shapeUtilities/debug -lshapeUtilities
    LIBS += -L../../agrolib/shapeHandler/debug -lshapeHandler
    LIBS += -L../../agrolib/utilities/debug -lutilities
    LIBS += -L../../agrolib/gis/debug -lgis
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions

} else {

    LIBS += -L../../mapGraphics/release -lMapGraphics
    LIBS += -L../../agrolib/commonDialogs/release -lcommonDialogs
    LIBS += -L../../agrolib/criteriaOutput/release -lcriteriaOutput
    LIBS += -L../../agrolib/shapeUtilities/release -lshapeUtilities
    LIBS += -L../../agrolib/shapeHandler/release -lshapeHandler
    LIBS += -L../../agrolib/utilities/release -lutilities
    LIBS += -L../../agrolib/gis/release -lgis
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
}

HEADERS += \
    ../../agrolib/graphics/mapGraphicsRasterObject.h \
    ../../agrolib/graphics/mapGraphicsShapeObject.h \
    ../../agrolib/graphics/colorLegend.h \
    criteriaGeoProject.h \
    dialogDbfNewCol.h \
    dialogDbfTable.h \
    dialogOutputMap.h \
    dialogSelectField.h \
    dialogShapeProperties.h \
    dialogUcmIntersection.h \
    dialogUcmPrevailing.h \
    gisObject.h \
    mainWindow.h \
    tabMap.h \
    tabSymbology.h \
    tableDbf.h

SOURCES += \
    ../../agrolib/graphics/mapGraphicsRasterObject.cpp \
    ../../agrolib/graphics/mapGraphicsShapeObject.cpp \
    ../../agrolib/graphics/colorLegend.cpp \
    criteriaGeoProject.cpp \
    dialogDbfNewCol.cpp \
    dialogDbfTable.cpp \
    dialogOutputMap.cpp \
    dialogSelectField.cpp \
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
