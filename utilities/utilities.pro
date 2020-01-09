#-------------------------------------------------
#
#   Utilities library
#   This project is part of CRITERIA-3D distribution
#
#-------------------------------------------------

QT      += core sql
QT      -= gui

TEMPLATE = agrolib
CONFIG += staticlib

CONFIG += debug_and_release
QMAKE_CXXFLAGS += -std=c++11

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/utilities
    } else {
        TARGET = release/utilities
    }
}
win32:{
    TARGET = utilities
}

INCLUDEPATH += ../crit3dDate ../mathFunctions

SOURCES += \
    utilities.cpp

HEADERS += \
    utilities.h

unix {
    target.path = /usr/agrolib
    INSTALLS += target
}
