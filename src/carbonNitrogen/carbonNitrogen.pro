#----------------------------------------------------
#
#   carbonNitrogen library
#   This project is part of CRITERIA-3D distribution
#
#----------------------------------------------------

QT -= core gui

TARGET = carbonNitrogen
TEMPLATE = lib
CONFIG += staticlib

CONFIG += debug_and_release

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/carbonNitrogen
    } else {
        TARGET = release/carbonNitrogen
    }
}
win32:{
    TARGET = carbonNitrogen
}

INCLUDEPATH += ../../agrolib/mathFunctions ../../agrolib/soil

HEADERS += carbonNitrogen.h
	
SOURCES += carbonNitrogen.cpp

