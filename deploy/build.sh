#!/bin/bash

set -ex

image=$1

if [[ $image =~ ^centos:8 ]]
then
    echo "qmake $QMAKE"
    # build mapGraphics
    cd mapGraphics
    $QMAKE MapGraphics.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make

    cd -

    # build CRITERIA1D
    cd bin/Makeall_CRITERIA1D
    $QMAKE Makeall_CRITERIA1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make

    cd -

    # build CROP_EDITOR
    cd bin/Makeall_CROP_EDITOR
    $QMAKE Makeall_CROP_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler 
    make clean
    make

    cd -

    # build SOIL_EDITOR
    cd bin/Makeall_SOIL_EDITOR
    $QMAKE Makeall_SOIL_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make

    cd -

    # build CRITERIAGEO
    cd bin/Makeall_CRITERIAGEO
    $QMAKE Makeall_CRITERIAGEO.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make
    
    
elif [[ $image =~ ^ubuntu: ]]
then
    # build mapGraphics
    cd mapGraphics
    $QMAKE MapGraphics.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make

    cd -

    # build CRITERIA1D
    cd bin/Makeall_CRITERIA1D
    $QMAKE Makeall_CRITERIA1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make

    cd -

    # build CROP_EDITOR
    cd bin/Makeall_CROP_EDITOR
    $QMAKE Makeall_CROP_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make

    cd -

    # build SOIL_EDITOR
    cd bin/Makeall_SOIL_EDITOR
    $QMAKE Makeall_SOIL_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make

    cd -

    # build CRITERIAGEO
    cd bin/Makeall_CRITERIAGEO
    $QMAKE Makeall_CRITERIAGEO.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make

    cd -
    
    # build HEAT1D
    cd bin/Makeall_HEAT1D
    $QMAKE Makeall_HEAT1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make
    
    cd -
    
    # download linuxdeployqt
    wget -c -nv -O linuxqtdeploy "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
    chmod +x linuxqtdeploy
    
    # build appimage CRITERIA1D
    cp -rf deploy/appimage deploy/tmpbuild
    cp bin/CRITERIA1D/release/CRITERIA1D deploy/tmpbuild/usr/bin/CRITERIA1D
    ./linuxqtdeploy --appimage-extract-and-run deploy/tmpbuild/usr/share/applications/CRITERIA1D.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite
    cp deploy/tmpbuild/usr/bin/* deploy
    rm -rf deploy/tmpbuild/
    
    # build appimage CROP_EDITOR
    cp -rf deploy/appimage deploy/tmpbuild
    cp bin/CROP_EDITOR/release/CROP_EDITOR deploy/tmpbuild/usr/bin/CROP_EDITOR
    ./linuxqtdeploy --appimage-extract-and-run deploy/tmpbuild/usr/share/applications/CROP_EDITOR.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite
    cp deploy/tmpbuild/usr/bin/* deploy
    rm -rf deploy/tmpbuild/
    
    # build appimage SOIL_EDITOR
    cp -rf deploy/appimage deploy/tmpbuild
    cp bin/SOIL_EDITOR/release/SOIL_EDITOR deploy/tmpbuild/usr/bin/SOIL_EDITOR
    ./linuxqtdeploy --appimage-extract-and-run deploy/tmpbuild/usr/share/applications/SOIL_EDITOR.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite
    cp deploy/tmpbuild/usr/bin/* deploy
    rm -rf deploy/tmpbuild/
    
    # build appimage CRITERIA_GEO
    cp -rf deploy/appimage deploy/tmpbuild
    cp bin/CRITERIAGEO/release/CRITERIA_GEO deploy/tmpbuild/usr/bin/CRITERIA_GEO
    LD_LIBRARY_PATH=`pwd`/mapGraphics/release ./linuxqtdeploy --appimage-extract-and-run deploy/tmpbuild/usr/share/applications/CRITERIA_GEO.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite
    cp deploy/tmpbuild/usr/bin/* deploy
    rm -rf deploy/tmpbuild/
    
    # build appimage HEAT1D
    cp -rf deploy/appimage deploy/tmpbuild
    cp bin/HEAT1D/release/HEAT1D deploy/tmpbuild/usr/bin/HEAT1D
    ./linuxqtdeploy --appimage-extract-and-run deploy/tmpbuild/usr/share/applications/HEAT1D.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite
    cp deploy/tmpbuild/usr/bin/* deploy
    rm -rf deploy/tmpbuild/

else
    echo "Unknown image $image"
    exit 1
fi
