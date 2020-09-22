#!/bin/bash

# specify your Qt directory
QT_DIR=/opt/Qt/5.12.8/gcc_64/
QMAKE=$QT_DIR/bin/qmake

# build csvToMeteoDb
cd ../tools/csvToMeteoDb
$QMAKE csvToMeteoDb.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all 
make 

cd -

build CRITERIAOUTPUT
cd ../tools/Makeall_CriteriaOutput
$QMAKE Makeall_CriteriaOutput.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all 
make 

cd -

# build CRITERIA1D
cd ../bin/Makeall_CRITERIA1D
$QMAKE Makeall_CRITERIA1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all 
make 

cd -

# build CROP_EDITOR
cd ../bin/Makeall_CROP_EDITOR
$QMAKE Makeall_CROP_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all 
make 

cd -

# build SOIL_EDITOR
cd ../bin/Makeall_SOIL_EDITOR
$QMAKE Makeall_SOIL_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all 
make

cd -

function make_appimage {

    BIN_NAME=$1
    BIN_DIR=$2

    rm -rf appimage
    # make tree and copy executables and images
    mkdir appimage
    # copy img files
    cp ../DOC/img/saveButton.png appimage
    cp ../DOC/img/updateButton.png appimage
    cp ../DOC/img/textural_soil.png appimage
    cp ../DOC/img/CRITERIA1D_icon.png appimage/$BIN_NAME.png
    mkdir appimage/usr
    mkdir appimage/usr/bin
    echo 'Place executable here' > appimage/usr/bin/README
    mkdir appimage/usr/share
    mkdir appimage/usr/share/applications
    cp $BIN_DIR/$BIN_NAME appimage/usr/bin/$BIN_NAME
    cp linuxDeploy/$BIN_NAME.desktop appimage/usr/share/applications/

    cp /home/xenial/CRITERIA3D/DEPLOY/appimage/CRITERIA1D.png appimage/$BIN_NAME.png
    
    ./linuxqtdeploy appimage/usr/share/applications/$BIN_NAME.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite

}

# download linuxdeployqt
wget -c -nv -O linuxqtdeploy "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"

chmod +x linuxqtdeploy

make_appimage CsvToMeteoDb ../tools/csvToMeteoDb/release/
make_appimage CriteriaOutput ../tools/criteriaOutputTools/release/
make_appimage CRITERIA1D ../bin/CRITERIA1D/release/
make_appimage CROP_EDITOR ../bin/CROP_EDITOR/release/
make_appimage SOIL_EDITOR ../bin/SOIL_EDITOR/release/


mkdir CRITERIA1D
mkdir CRITERIA1D/bin
mv *.AppImage* CRITERIA1D/bin

# copy img files
mkdir CRITERIA1D/DOC
mkdir CRITERIA1D/DOC/img
cp -r ../DOC/img/saveButton.png CRITERIA1D/DOC/img
cp -r ../DOC/img/updateButton.png CRITERIA1D/DOC/img
cp -r ../DOC/img/textural_soil.png CRITERIA1D/DOC/img

# copy soil data
mkdir CRITERIA1D/DATA
mkdir CRITERIA1D/DATA/SOIL
cp -r ../DATA/SOIL/* CRITERIA1D/DATA/SOIL

# copy kiwifruit project
mkdir CRITERIA1D/DATA/PROJECT
mkdir CRITERIA1D/DATA/PROJECT/kiwifruit
cp -r ../DATA/PROJECT/kiwifruit/* CRITERIA1D/DATA/PROJECT/kiwifruit
