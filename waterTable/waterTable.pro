#-------------------------------------------------
#
#   WaterTable library import data, computation, draw
#
#-------------------------------------------------

QT      += core
QT      -= gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += debug_and_release
CONFIG += c++11 c++14 c++17

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/waterTable
    } else {
        TARGET = release/waterTable
    }
}
win32:{
    TARGET = waterTable
}

INCLUDEPATH += ../mathFunctions ../meteo ../gis ../dbMeteoPoints ../dbMeteoGrid

SOURCES += importRegistry.cpp \
    well.cpp

HEADERS += importRegistry.h \
    well.h

