#-----------------------------------------------------------
#
#   csvToMeteoDb
#   This project is part of CRITERIA1D distribution
#
#-----------------------------------------------------------

QT += core sql
QT -= gui

CONFIG += console
TEMPLATE = app

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/csvToMeteoDb
    } else {
        TARGET = release/csvToMeteoDb
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/csvToMeteoDb
    } else {
        TARGET = release/csvToMeteoDb
    }
}
win32:{
    TARGET = csvToMeteoDb
}


SOURCES += main.cpp
