:: build CRITERIA1D
cd ..\bin\Makeall_CRITERIA1D
qmake
mingw32-make release

:: build CROP_EDITOR
cd ..\Makeall_CROP_EDITOR
qmake
mingw32-make release

:: build SOIL_EDITOR
cd ..\Makeall_SOIL_EDITOR
qmake
mingw32-make release

:: build WG
cd ..\Makeall_WG
qmake
mingw32-make release

:: copy executables
cd ..\..\DEPLOY
mkdir CRITERIA1D\bin
cd CRITERIA1D\bin
copy ..\..\..\bin\CRITERIA1D\release\CRITERIA1D.exe
copy ..\..\..\bin\CROP_EDITOR\release\CROP_EDITOR.exe
copy ..\..\..\bin\SOIL_EDITOR\release\SOIL_EDITOR.exe
copy ..\..\..\bin\WG\release\WG.exe

:: deploy
windeployqt CRITERIA1D.exe
windeployqt CROP_EDITOR.exe
windeployqt SOIL_EDITOR.exe
windeployqt WG.exe

:: copy img files
cd ..
mkdir DOC\img
cd DOC\img
copy ..\..\..\..\DOC\img\saveButton.png
copy ..\..\..\..\DOC\img\updateButton.png
copy ..\..\..\..\DOC\img\textural_soil.png

:: copy soil data
cd ..\..\
mkdir DATA\SOIL
cd DATA\SOIL
xcopy /Y ..\..\..\..\DATA\SOIL\*.* 

:: copy WG data
cd ..\..\
mkdir DATA\WG
cd DATA\WG
xcopy /s /Y ..\..\..\..\bin\WG\DATA\*.*

:: copy kiwifruit project
cd ..\..\
mkdir DATA\PROJECT\kiwifruit
cd DATA\PROJECT\kiwifruit
xcopy /s /Y ..\..\..\..\..\DATA\PROJECT\kiwifruit\*.* 

:: return to deploy directory
cd ..\..\..\..\
