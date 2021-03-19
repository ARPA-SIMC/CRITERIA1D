#!/bin/bash

# specify your Qt directory
QT_DIR=/Users/gabrieleantolini/Qt/5.15.2/clang_64/bin
QMAKE=$QT_DIR/qmake
QDEPLOY=$QT_DIR/macdeployqt

# build mapGraphics
cd ../mapGraphics
$QMAKE MapGraphics.pro -spec macx-clang CONFIG+=release CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile all

export LD_LIBRARY_PATH=`pwd`/../mapGraphics/release/:$LD_LIBRARY_PATH

cd -

# build CRITERIA1D
cd ../bin/Makeall_CRITERIA1D
$QMAKE Makeall_CRITERIA1D.pro -spec macx-clang CONFIG+=release CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build CROP_EDITOR
cd ../bin/Makeall_CROP_EDITOR
$QMAKE Makeall_CROP_EDITOR.pro -spec macx-clang CONFIG+=release CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build SOIL_EDITOR
cd ../bin/Makeall_SOIL_EDITOR
$QMAKE Makeall_SOIL_EDITOR.pro -spec macx-clang CONFIG+=release CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make

cd -

# build CRITERIAGEO
cd ../bin/Makeall_CRITERIAGEO
$QMAKE Makeall_CRITERIAGEO.pro -spec macx-clang CONFIG+=release CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build HEAT1D
cd ../bin/Makeall_HEAT1D
$QMAKE Makeall_HEAT1D.pro -spec macx-clang CONFIG+=release CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make

cd -

mkdir CRITERIA1D
mkdir CRITERIA1D/bin

cp -r ../bin/CRITERIA1D/release/CRITERIA1D CRITERIA1D/bin/CRITERIA1D
cp -r ../bin/CROP_EDITOR/release/CROP_EDITOR CRITERIA1D/bin/CROP_EDITOR
cp -r ../bin/SOIL_EDITOR/release/SOIL_EDITOR CRITERIA1D/bin/SOIL_EDITOR
cp -r ../bin/CRITERIAGEO/release/CRITERIA_GEO CRITERIA1D/bin/CRITERIA_GEO
cp -r ../bin/HEAT1D/release/HEAT1D CRITERIA1D/bin/HEAT1D

# deploy apps
cd CRITERIA1D/bin
$DEPLOY CRITERIA1D.app
$DEPLOY CROP_EDITOR.app
$DEPLOY SOIL_EDITOR.app
$DEPLOY CRITERIA_GEO.app
$DEPLOY HEAT1D.app

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
