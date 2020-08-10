#-----------------------------------------------------------
#
#   criteriaOutput
#   post-processing of CRITERIA-1D output
#   to csv or shapefile
#
#   This project is part of CRITERIA-1D distribution
#
#-----------------------------------------------------------

QT -= gui
QT += core sql

CONFIG += console
TEMPLATE = app

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/criteriaOutput
    } else {
        TARGET = release/criteriaOutput
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/criteriaOutput
    } else {
        TARGET = release/criteriaOutput
    }
}
win32:{
    TARGET = criteriaOutput
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += _CRT_SECURE_NO_WARNINGS


SOURCES += \
    ../../agrolib/criteriaModel/criteria1DUnit.cpp \
    ../../agrolib/crop/cropDbQuery.cpp \
    criteriaOutputProject.cpp \
    criteriaOutputVariable.cpp \
    main.cpp \
    shapeFromCsv.cpp

HEADERS += \
    criteriaOutputProject.h \
    criteriaOutputVariable.h \
    shapeFromCsv.h


INCLUDEPATH +=  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/crop \
                ../../agrolib/criteriaModel ../../agrolib/utilities ../../agrolib/shapeHandler


CONFIG(debug, debug|release) {

    LIBS += -L../../agrolib/shapeHandler/debug -lshapeHandler
    LIBS += -L../../agrolib/utilities/debug -lutilities
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions

} else {

    LIBS += -L../../agrolib/shapeHandler/release -lshapeHandler
    LIBS += -L../../agrolib/utilities/release -lutilities
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
} 
