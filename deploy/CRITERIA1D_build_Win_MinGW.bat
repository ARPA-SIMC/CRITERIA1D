:: build CRITERIA-1D/GEO
:: run on Qt shell (MINGW version) 
:: move into the deploy directory (cd [local path]\CRITERIA1D\deploy)

:: GDAL library (for GEO) doesn't work with MinGW compiler 
:: To disable GDAL comment the line CONFIG += GDAL 
:: in these project files:
:: 1) makell_CRITERIAGEO.pro
:: 2) CRITERIAGEO.pro
:: 3) agrolib/criteriaOutput.pro


:: CLEAN all
cd ..\mapGraphics
mingw32-make --silent clean
cd ..\bin\Makeall_CRITERIA1D
mingw32-make --silent distclean
cd ..\Makeall_CROP_EDITOR
mingw32-make --silent distclean
cd ..\Makeall_SOIL_EDITOR
mingw32-make --silent distclean
cd ..\Makeall_HEAT1D
mingw32-make --silent distclean
cd ..\Makeall_CRITERIAGEO
mingw32-make --silent distclean


:: build CRITERIA1D
cd ..\Makeall_CRITERIA1D
qmake -platform win32-g++ CONFIG+=release
mingw32-make --silent release

:: build SOIL_EDITOR
cd ..\Makeall_SOIL_EDITOR
qmake -platform win32-g++ CONFIG+=release
mingw32-make --silent release

:: build CROP_EDITOR
cd ..\Makeall_CROP_EDITOR
qmake -platform win32-g++ CONFIG+=release
mingw32-make --silent release

:: build HEAT1D
cd ..\Makeall_HEAT1D
qmake -platform win32-g++ CONFIG+=release
mingw32-make --silent release

:: mapGraphics
cd ..\..\mapGraphics
qmake -platform win32-g++ CONFIG+=release
mingw32-make --silent release

:: build CRITERIAGEO
cd ..\bin\Makeall_CRITERIAGEO
qmake -platform win32-g++ CONFIG+=release
mingw32-make --silent release


:: copy executables
cd ..\..\DEPLOY
mkdir CRITERIA1D\bin
cd CRITERIA1D\bin
copy ..\..\..\bin\CRITERIA1D\release\CRITERIA1D.exe
copy ..\..\..\bin\SOIL_EDITOR\release\SOIL_EDITOR.exe
copy ..\..\..\bin\CROP_EDITOR\release\CROP_EDITOR.exe
copy ..\..\..\bin\HEAT1D\release\HEAT1D.exe
copy ..\..\..\bin\CRITERIAGEO\release\CRITERIA_GEO.exe


:: deploy
windeployqt HEAT1D.exe
windeployqt CRITERIA1D.exe
windeployqt SOIL_EDITOR.exe
windeployqt CROP_EDITOR.exe
windeployqt CRITERIA_GEO.exe


:: copy doc and img files
cd ..\
mkdir DOC
cd DOC
copy ..\..\..\DOC\CRITERIA1D_technical_manual.pdf
mkdir img
cd img
copy ..\..\..\..\DOC\img\saveButton.png
copy ..\..\..\..\DOC\img\updateButton.png
copy ..\..\..\..\DOC\img\textural_soil.png


:: copy soil data
cd ..\..\
mkdir DATA\SOIL
cd DATA\SOIL
xcopy /Y ..\..\..\..\DATA\SOIL\*.*

cd ..\..\
mkdir DATA\SETTINGS
cd DATA\SETTINGS
xcopy /Y ..\..\..\..\DATA\SETTINGS\*.*
 
:: copy test project
cd ..\..\
mkdir DATA\PROJECT\test
cd DATA\PROJECT\test
xcopy /s /Y ..\..\..\..\..\DATA\PROJECT\test\*.* 

:: return to deploy directory
cd ..\..\..\..\

