#!/bin/bash

# specify your Qt directory
#QT_DIR=/opt/Qt/5.12.8/gcc_64/
#QMAKE=$QT_DIR/bin/qmake
QMAKE=/usr/bin/qmake-qt5

# # build mapGraphics
cd ../mapGraphics
$QMAKE MapGraphics.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile all

export LD_LIBRARY_PATH=`pwd`/../mapGraphics/release/:$LD_LIBRARY_PATH

cd -

# build csvToMeteoDb
cd ../tools/csvToMeteoDb
$QMAKE csvToMeteoDb.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build CRITERIAOUTPUT
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

# build CRITERIAGEO
cd ../bin/Makeall_CRITERIAGEO
$QMAKE Makeall_CRITERIAGEO.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build HEAT1D
cd ../bin/Makeall_HEAT1D
$QMAKE Makeall_HEAT1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all
make

cd -

# download linuxdeployqt
#wget -c -nv -O linuxqtdeploy "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
#chmod +x linuxqtdeploy

# build appimage CsvToMeteoDb
cp -r ../tools/csvToMeteoDb/release/CsvToMeteoDb appimage/usr/bin/CsvToMeteoDb
#./linuxqtdeploy --appimage-extract-and-run appimage/usr/share/applications/CsvToMeteoDb.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite

# build appimage CriteriaOutput
cp -r ../tools/criteriaOutputTools/release/CriteriaOutput appimage/usr/bin/CriteriaOutput
#./linuxqtdeploy --appimage-extract-and-run appimage/usr/share/applications/CriteriaOutput.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite

# build appimage CRITERIA1D
cp -r ../bin/CRITERIA1D/release/CRITERIA1D appimage/usr/bin/CRITERIA1D
#./linuxqtdeploy --appimage-extract-and-run appimage/usr/share/applications/CRITERIA1D.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite

# build appimage CROP_EDITOR
cp -r ../bin/CROP_EDITOR/release/CROP_EDITOR appimage/usr/bin/CROP_EDITOR
#./linuxqtdeploy --appimage-extract-and-run appimage/usr/share/applications/CROP_EDITOR.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite

# build appimage SOIL_EDITOR
cp -r ../bin/SOIL_EDITOR/release/SOIL_EDITOR appimage/usr/bin/SOIL_EDITOR
#./linuxqtdeploy --appimage-extract-and-run appimage/usr/share/applications/SOIL_EDITOR.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite

# build appimage CRITERIA_GEO
cp -r ../bin/CRITERIAGEO/release/CRITERIA_GEO appimage/usr/bin/CRITERIA_GEO
#./linuxqtdeploy --appimage-extract-and-run appimage/usr/share/applications/CRITERIA_GEO.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite

# build appimage HEAT1D
cp -r ../bin/HEAT1D/release/HEAT1D appimage/usr/bin/HEAT1D
#./linuxqtdeploy --appimage-extract-and-run appimage/usr/share/applications/HEAT1D.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite

mkdir CRITERIA1D
mkdir CRITERIA1D/bin
mv appimage/usr/bin/* CRITERIA1D/bin/

# copy png
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
