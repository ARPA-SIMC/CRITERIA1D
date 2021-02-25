QT -= gui
QT += core sql xml

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/ECMWF_test
    } else {
        TARGET = release/ECMWF_test
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/ECMWF_test
    } else {
        TARGET = release/ECMWF_test
    }
}
win32:{
    TARGET = ECMWF_test
}

INCLUDEPATH +=  ../../../agrolib/crit3dDate ../../../agrolib/mathFunctions ../../../agrolib/gis \
                ../../../agrolib/utilities ../../../agrolib/meteo ../../../agrolib/dbMeteoGrid

CONFIG(debug, debug|release) {

    LIBS += -L../../../agrolib/dbMeteoGrid/debug -ldbMeteoGrid
    LIBS += -L../../../agrolib/meteo/debug -lmeteo
    LIBS += -L../../../agrolib/utilities/debug -lutilities
    LIBS += -L../../../agrolib/gis/debug -lgis
    LIBS += -L../../../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../../../agrolib/mathFunctions/debug -lmathFunctions

} else {

    LIBS += -L../../../agrolib/dbMeteoGrid/release -ldbMeteoGrid
    LIBS += -L../../../agrolib/meteo/release -lmeteo
    LIBS += -L../../../agrolib/utilities/release -lutilities
    LIBS += -L../../../agrolib/gis/release -lgis
    LIBS += -L../../../agrolib/crit3dDate/release -lcrit3dDate
    LIBS += -L../../../agrolib/mathFunctions/release -lmathFunctions
}
