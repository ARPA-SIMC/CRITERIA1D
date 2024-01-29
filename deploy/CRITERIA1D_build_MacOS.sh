#!/bin/bash

# specify your Qt directory (replace "yourname")
QT_DIR=/Users/yourname/Qt/5.15.2/clang_64/bin
QMAKE=$QT_DIR/qmake
QDEPLOY=$QT_DIR/macdeployqt

# build mapGraphics
cd ../mapGraphics
$QMAKE MapGraphics.pro -spec macx-clang CONFIG+=release CONFIG+=force_debug_info CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile all

export LD_LIBRARY_PATH=`pwd`/../mapGraphics/release/:$LD_LIBRARY_PATH

cd -

# build CRITERIA1D
cd ../bin/Makeall_CRITERIA1D
$QMAKE Makeall_CRITERIA1D.pro -spec macx-clang CONFIG+=release CONFIG+=force_debug_info CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build SOIL_EDITOR
cd ../bin/Makeall_SOIL_EDITOR
$QMAKE Makeall_SOIL_EDITOR.pro -spec macx-clang CONFIG+=release CONFIG+=force_debug_info CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make

cd -

# build CRITERIA1D_PRO
cd ../bin/Makeall_CRITERIA1D_PRO
$QMAKE Makeall_CRITERIA1D_PRO -spec macx-clang CONFIG+=release CONFIG+=force_debug_info CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build CRITERIAGEO
cd ../bin/Makeall_CRITERIAGEO
$QMAKE Makeall_CRITERIAGEO.pro -spec macx-clang CONFIG+=release CONFIG+=force_debug_info CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build HEAT1D
cd ../bin/Makeall_HEAT1D
$QMAKE Makeall_HEAT1D.pro -spec macx-clang CONFIG+=release CONFIG+=force_debug_info CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make

cd -

mkdir CRITERIA1D
mkdir CRITERIA1D/bin

cp -r ../bin/CRITERIA1D/release/CRITERIA1D.app CRITERIA1D/bin/CRITERIA1D.app
cp -r ../bin/SOIL_EDITOR/release/SOIL_EDITOR.app CRITERIA1D/bin/SOIL_EDITOR.app
cp -r ../bin/CRITERIA1D_PRO/release/CRITERIA1D_PRO.app CRITERIA1D/bin/CRITERIA1D_PRO.app
cp -r ../bin/CRITERIAGEO/release/CRITERIA_GEO.app CRITERIA1D/bin/CRITERIA_GEO.app
cp -r ../bin/HEAT1D/release/HEAT1D.app CRITERIA1D/bin/HEAT1D.app

# deploy apps
cd CRITERIA1D/bin
$QDEPLOY CRITERIA1D.app
$QDEPLOY SOIL_EDITOR.app
$QDEPLOY CRITERIA1D_PRO.app
$QDEPLOY CRITERIA_GEO.app
$QDEPLOY HEAT1D.app

cd -

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

# copy settings and template
mkdir CRITERIA1D/DATA/SETTINGS
cp -r ../DATA/SETTINGS/* CRITERIA1D/DATA/SETTINGS
mkdir CRITERIA1D/DATA/TEMPLATE
cp -r ../DATA/TEMPLATE/* CRITERIA1D/DATA/TEMPLATE

# copy TEST project
mkdir CRITERIA1D/DATA/PROJECT
mkdir CRITERIA1D/DATA/PROJECT/test
cp -r ../DATA/PROJECT/test/* CRITERIA1D/DATA/PROJECT/test
