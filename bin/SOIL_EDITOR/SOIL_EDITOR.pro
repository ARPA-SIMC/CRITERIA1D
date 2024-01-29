#-----------------------------------------------------------
#
#   SOIL EDITOR
#   This project is part of CRITERIA1D distribution
#
#-----------------------------------------------------------

QT       += core gui widgets charts sql

TEMPLATE = app

CONFIG += debug_and_release

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/SOIL_EDITOR
    } else {
        TARGET = release/SOIL_EDITOR
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/SOIL_EDITOR
    } else {
        TARGET = release/SOIL_EDITOR
    }
}
win32:{
    TARGET = SOIL_EDITOR
}

INCLUDEPATH +=  ../../agrolib/crit3dDate ../../agrolib/mathFunctions  \
                ../../agrolib/utilities ../../agrolib/soil \
                ../../agrolib/commonChartElements ../../agrolib/soilWidget

CONFIG(debug, debug|release) {
    LIBS += -L../../agrolib/soilWidget/debug -lsoilWidget
    LIBS += -L../../agrolib/commonChartElements/debug -lcommonChartElements
    LIBS += -L../../agrolib/soil/debug -lsoil
    LIBS += -L../../agrolib/utilities/debug -lutilities
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
} else {

    LIBS += -L../../agrolib/soilWidget/release -lsoilWidget
    LIBS += -L../../agrolib/commonChartElements/release -lcommonChartElements
    LIBS += -L../../agrolib/soil/release -lsoil
    LIBS += -L../../agrolib/utilities/release -lutilities
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
}


SOURCES += \
        main.cpp


RESOURCES += soil.ico

win32:
{
    RC_ICONS = soil.ico
}
