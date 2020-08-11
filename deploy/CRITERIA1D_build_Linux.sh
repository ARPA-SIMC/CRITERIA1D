#!/bin/bash

# specify your Qt directory
QT_DIR=/opt/Qt/5.12.8/gcc_64/
QMAKE=$QT_DIR/bin/qmake

# build CRITERIA1D
cd ../bin/Makeall_CRITERIA1D
$QMAKE Makeall_CRITERIA1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile qmake_all 
make 

cd -

# build CROP_EDITOR
cd ../bin/Makeall_CROP_EDITOR
$QMAKE Makeall_CROP_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile qmake_all 
make 

cd -

# build SOIL_EDITOR
cd ../bin/Makeall_SOIL_EDITOR
$QMAKE Makeall_SOIL_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile qmake_all 
make

cd -

# build WG
cd ../bin/Makeall_WG
$QMAKE Makeall_WG.pro -spec linux-g++-64 CONFIG+=release CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile qmake_all 
make

cd -

function make_appimage {

    rm -rf appimage
    # make tree and copy executables and images
    mkdir appimage
    # copy img files
    cp ../DOC/img/saveButton.png appimage
    cp ../DOC/img/updateButton.png appimage
    cp ../DOC/img/textural_soil.png appimage
    cp ../DOC/img/CRITERIA3D_icon.png appimage/$1.png
    mkdir appimage/usr
    mkdir appimage/usr/bin
    echo 'Place executable here' > appimage/usr/bin/README
    mkdir appimage/usr/share
    mkdir appimage/usr/share/applications
    cp ../bin/$1/release/$1 appimage/usr/bin/$1
    cp $1.desktop appimage/usr/share/applications/

    ./linuxqtdeploy appimage/usr/share/applications/$1.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite

}

# download linuxdeployqt
wget -c -nv -O linuxqtdeploy "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod +x linuxqtdeploy

make_appimage CRITERIA1D
make_appimage CROP_EDITOR
make_appimage SOIL_EDITOR
make_appimage WG

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

# copy WG data
mkdir CRITERIA1D/DATA/WG
cp -r ../bin/WG/DATA/* CRITERIA1D/DATA/WG

# copy kiwifruit project
mkdir CRITERIA1D/DATA/PROJECT
mkdir CRITERIA1D/DATA/PROJECT/kiwifruit
cp -r ../DATA/PROJECT/kiwifruit/* CRITERIA1D/DATA/PROJECT/kiwifruit
