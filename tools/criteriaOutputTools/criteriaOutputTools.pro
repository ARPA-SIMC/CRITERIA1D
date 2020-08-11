#-----------------------------------------------------------
#
#   CriteriaOutput
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
        TARGET = debug/CriteriaOutput
    } else {
        TARGET = release/CriteriaOutput
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/CriteriaOutput
    } else {
        TARGET = release/CriteriaOutput
    }
}
win32:{
    TARGET = CriteriaOutput
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += _CRT_SECURE_NO_WARNINGS


SOURCES += \
    main.cpp


INCLUDEPATH +=  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/crop \
                ../../agrolib/criteriaModel ../../agrolib/utilities ../../agrolib/shapeHandler ../../agrolib/criteriaOutput


CONFIG(debug, debug|release) {

    LIBS += -L../../agrolib/criteriaOutput/debug -lcriteriaOutput
    LIBS += -L../../agrolib/shapeHandler/debug -lshapeHandler
    LIBS += -L../../agrolib/utilities/debug -lutilities
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions

} else {

    LIBS += -L../../agrolib/criteriaOutput/release -lcriteriaOutput
    LIBS += -L../../agrolib/shapeHandler/release -lshapeHandler
    LIBS += -L../../agrolib/utilities/release -lutilities
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
} 
