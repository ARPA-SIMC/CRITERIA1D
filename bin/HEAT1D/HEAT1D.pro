#-----------------------------------------------------
#
#   HEAT1D
#   Test for soilFluxex3D library in a 1D domain
#   This project is part of CRITERIA3D distribution
#
#-----------------------------------------------------

QT       += widgets core charts printsupport

TARGET = HEAT1D
TEMPLATE = app

INCLUDEPATH +=  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis  \
                ../../agrolib/soilFluxes3D/header ../../agrolib/qcustomplot


CONFIG += debug_and_release

CONFIG(debug, debug|release) {

    LIBS += -L../../agrolib/qcustomplot/debug -lqcustomplot
    LIBS += -L../../agrolib/soilFluxes3D/debug -lsoilFluxes3D
    LIBS += -L../../agrolib/gis/debug -lgis
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
}
else
{
    LIBS += -L../../agrolib/qcustomplot/release -lqcustomplot
    LIBS += -L../../agrolib/soilFluxes3D/release -lsoilFluxes3D
    LIBS += -L../../agrolib/gis/release -lgis
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
}


SOURCES += main.cpp\
    callout.cpp \
    colorMapGraph.cpp \
    graphFunctions.cpp \
    linearGraph.cpp \
    mainwindow.cpp \
    heat1D.cpp

HEADERS  += heat1D.h \
    callout.h \
    colorMapGraph.h \
    graphFunctions.h \
    linearGraph.h \
    mainwindow.h

FORMS    += \
    mainwindow.ui


