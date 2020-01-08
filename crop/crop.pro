#-----------------------------------------------------
#
#   crop library
#   This project is part of CRITERIA-3D distribution
#
#-----------------------------------------------------

QT -= gui
QT += core sql

TARGET = crop
TEMPLATE = lib
CONFIG += staticlib

CONFIG += debug_and_release

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/crop
    } else {
        TARGET = release/crop
    }
}
win32:{
    TARGET = crop
}

INCLUDEPATH +=  ../../lib/crit3dDate ../../lib/mathFunctions ../../lib/soil ../../lib/utilities

SOURCES += crop.cpp \
    cropDbTools.cpp \
    root.cpp \
    development.cpp \
    biomass.cpp

HEADERS += crop.h \
    biomass.h \
    cropDbTools.h \
    root.h \
    development.h
