#-----------------------------------------------------------
#
#   CsvToMeteoDb
#   This project is part of CRITERIA1D distribution
#
#-----------------------------------------------------------

QT += core sql
QT -= gui

CONFIG += console
TEMPLATE = app

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/CsvToMeteoDb
    } else {
        TARGET = release/CsvToMeteoDb
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/CsvToMeteoDb
    } else {
        TARGET = release/CsvToMeteoDb
    }
}
win32:{
    TARGET = CsvToMeteoDb
}


SOURCES += main.cpp
