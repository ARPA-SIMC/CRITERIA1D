#!/bin/bash

set -e 

image=$1

if [[ $image =~ ^centos:8 ]]
then
    echo "qmake $QMAKE"
    # build mapGraphics
    cd mapGraphics
    $QMAKE MapGraphics.pro -spec linux-g++-64 CONFIG+=debug CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make

    cd -

    # build csvToMeteoDb
    cd tools/csvToMeteoDb
    $QMAKE csvToMeteoDb.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make 

    cd -
    
    # build CRITERIAOUTPUT
    cd tools/Makeall_CriteriaOutput
    $QMAKE Makeall_CriteriaOutput.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make

    cd -

    # build CRITERIA1D
    cd bin/Makeall_CRITERIA1D
    $QMAKE Makeall_CRITERIA1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make

    cd -

    # build CROP_EDITOR
    cd bin/Makeall_CROP_EDITOR
    $QMAKE Makeall_CROP_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler 
    make clean
    make

    cd -

    # build SOIL_EDITOR
    cd bin/Makeall_SOIL_EDITOR
    $QMAKE Makeall_SOIL_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make

    cd -

    # build CRITERIAGEO
    cd bin/Makeall_CRITERIAGEO
    $QMAKE Makeall_CRITERIAGEO.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make
    
elif [[ $image =~ ^ubuntu: ]]
then
    # build mapGraphics
    cd mapGraphics
    $QMAKE MapGraphics.pro -spec linux-g++-64 CONFIG+=debug CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make

    cd -

    # build csvToMeteoDb
    cd tools/csvToMeteoDb
    $QMAKE csvToMeteoDb.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make 

    cd -
    
    # build CRITERIAOUTPUT
    cd tools/Makeall_CriteriaOutput
    $QMAKE Makeall_CriteriaOutput.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make

    cd -

    # build CRITERIA1D
    cd bin/Makeall_CRITERIA1D
    $QMAKE Makeall_CRITERIA1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make

    cd -

    # build CROP_EDITOR
    cd bin/Makeall_CROP_EDITOR
    $QMAKE Makeall_CROP_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make

    cd -

    # build SOIL_EDITOR
    cd bin/Makeall_SOIL_EDITOR
    $QMAKE Makeall_SOIL_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make

    cd -

    # build CRITERIAGEO
    cd bin/Makeall_CRITERIAGEO
    $QMAKE Makeall_CRITERIAGEO.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make

    cd -

else
    echo "Unknown image $image"
    exit 1
fi
