#!/bin/bash

# specify your qmake directory
QT_DIR=/opt/Qt/5.12.8/gcc_64/
QMAKE=$QT_DIR/bin/qmake

# # build mapGraphics
cd ../mapGraphics
$QMAKE MapGraphics.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile all

export LD_LIBRARY_PATH=`pwd`/../mapGraphics/release/:$LD_LIBRARY_PATH

cd -

# build CRITERIA1D
cd ../bin/Makeall_CRITERIA1D
$QMAKE Makeall_CRITERIA1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build CROP_EDITOR
cd ../bin/Makeall_CROP_EDITOR
$QMAKE Makeall_CROP_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build SOIL_EDITOR
cd ../bin/Makeall_SOIL_EDITOR
$QMAKE Makeall_SOIL_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all
make

cd -

# build CRITERIAGEO
cd ../bin/Makeall_CRITERIAGEO
$QMAKE Makeall_CRITERIAGEO.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build HEAT1D
cd ../bin/Makeall_HEAT1D
$QMAKE Makeall_HEAT1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all
make

cd -

# download linuxdeployqt
wget -c -nv -O linuxqtdeploy "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod +x linuxqtdeploy

function make_appimage {

    BIN_PATH=$1
    BIN_NAME=`basename $1`

    rm -rf build
    # make tree and copy executables and images
    cp -rf appimage build
    
    cp $BIN_PATH build/usr/bin/$BIN_NAME
    ./linuxqtdeploy --appimage-extract-and-run build/usr/share/applications/$BIN_NAME.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite
}

# build appimage CRITERIA1D
make_appimage ../bin/CRITERIA1D/release/CRITERIA1D

# build appimage CROP_EDITOR
make_appimage ../bin/CROP_EDITOR/release/CROP_EDITOR

# build appimage SOIL_EDITOR
make_appimage ../bin/SOIL_EDITOR/release/SOIL_EDITOR

# build appimage CRITERIA_GEO
make_appimage ../bin/CRITERIAGEO/release/CRITERIA_GEO

# build appimage HEAT1D
make_appimage ../bin/HEAT1D/release/HEAT1D

mkdir -p CRITERIA1D/bin
mv *.AppImage CRITERIA1D/bin/

# copy png
mkdir -p CRITERIA1D/DOC/img
cp -r ../DOC/CRITERIA1D_technical_manual.pdf CRITERIA1D/DOC/
cp -r ../DOC/img/saveButton.png CRITERIA1D/DOC/img
cp -r ../DOC/img/updateButton.png CRITERIA1D/DOC/img
cp -r ../DOC/img/textural_soil.png CRITERIA1D/DOC/img

# copy soil data
mkdir -p CRITERIA1D/DATA/SOIL
cp -r ../DATA/SOIL/* CRITERIA1D/DATA/SOIL

# copy settings and template
mkdir CRITERIA1D/DATA/SETTINGS
cp -r ../DATA/SETTINGS/* CRITERIA1D/DATA/SETTINGS
mkdir CRITERIA1D/DATA/TEMPLATE
cp -r ../DATA/TEMPLATE/* CRITERIA1D/DATA/TEMPLATE

# copy test project
mkdir -p CRITERIA1D/DATA/PROJECT/test
cp -r ../DATA/PROJECT/test/* CRITERIA1D/DATA/PROJECT/test

