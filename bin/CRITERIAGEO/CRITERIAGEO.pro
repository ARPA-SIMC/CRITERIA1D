#----------------------------------------------------------------------------
#
#   CRITERIA 1D-GEO
#   interface to set geographical projects for soil water balance 1D
#
#   Dependencies: GDAL
#   See gdal.pri for more info
#   Comment CONFIG += GDAL to compile without GDAL library
#
#   This project is part of CRITERIA-3D distribution
#
#-----------------------------------------------------------------------------


QT    += core gui network widgets sql

TEMPLATE = app
TARGET = CRITERIA_GEO

INCLUDEPATH +=  ../../mapGraphics \
                ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis  \
                ../../agrolib/shapeUtilities ../../agrolib/shapeHandler ../../agrolib/shapeHandler/shapelib \
                ../../agrolib/graphics ../../agrolib/project

CONFIG += debug_and_release


CONFIG(debug, debug|release) {
    LIBS += -L../../mapGraphics/debug -lMapGraphics
    LIBS += -L../../agrolib/shapeUtilities/debug -lshapeUtilities
    LIBS += -L../../agrolib/shapeHandler/debug -lshapeHandler
    LIBS += -L../../agrolib/gis/debug -lgis
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions
} else {
    LIBS += -L../../mapGraphics/release -lMapGraphics
    LIBS += -L../../agrolib/shapeUtilities/release -lshapeUtilities
    LIBS += -L../../agrolib/shapeHandler/release -lshapeHandler
    LIBS += -L../../agrolib/gis/release -lgis
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
}


HEADERS += \
    ../../agrolib/graphics/mapGraphicsRasterObject.h \
    ../../agrolib/graphics/mapGraphicsShapeObject.h \
    ../../agrolib/graphics/colorLegend.h \
    ../../agrolib/project/formInfo.h \
    criteriaGeoProject.h \
    dialogSelectField.h \
    dialogShapeProperties.h \
    dbfTableDialog.h \
    dbfNewColDialog.h \
    dialogUcmIntersection.h \
    dialogUcmPrevailing.h \
    gisObject.h \
    mainWindow.h

SOURCES += \
    ../../agrolib/graphics/mapGraphicsRasterObject.cpp \
    ../../agrolib/graphics/mapGraphicsShapeObject.cpp \
    ../../agrolib/graphics/colorLegend.cpp \
    ../../agrolib/project/formInfo.cpp \
    criteriaGeoProject.cpp \
    dialogSelectField.cpp \
    dialogShapeProperties.cpp \
    dialogUcmIntersection.cpp \
    dialogUcmPrevailing.cpp \
    mainWindow.cpp \
    dbfTableDialog.cpp \
    dbfNewColDialog.cpp \
    gisObject.cpp \
    main.cpp


FORMS += \
    mainWindow.ui


# comment to compile without GDAL library
CONFIG += GDAL

GDAL {
    DEFINES += GDAL
    INCLUDEPATH += ../../agrolib/gdalHandler

    include(../../agrolib/gdal.pri)

    CONFIG(debug, debug|release) {
        LIBS += -L../../agrolib/gdalHandler/debug -lgdalHandler
    } else {
        LIBS += -L../../agrolib/gdalHandler/release -lgdalHandler
    }
}

