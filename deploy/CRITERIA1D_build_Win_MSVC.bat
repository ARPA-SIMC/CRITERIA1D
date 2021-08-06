:: build CRITERIA-1D/GEO
:: run on Qt shell (MSVC version) 
:: move into the deploy directory (cd [local path]\CRITERIA1D\deploy)

:: before the execution call vcvarsall.bat (32 or 64 bit) to complete environment setup
:: example: run C:\"Program Files (x86)"\"Microsoft Visual Studio"\2017\Community\VC\Auxiliary\Build\vcvars64.bat


:: clean all
cd ..\mapGraphics
nmake /S /NOLOGO distclean
cd ..\bin\Makeall_CRITERIA1D
nmake /S /NOLOGO distclean
cd ..\Makeall_CRITERIA1D_PRO
nmake /S /NOLOGO distclean
cd ..\Makeall_SOIL_EDITOR
nmake /S /NOLOGO distclean
cd ..\Makeall_HEAT1D
nmake /S /NOLOGO distclean
cd ..\Makeall_CRITERIAGEO
nmake /S /NOLOGO distclean

:: build CRITERIA-1D
cd ..\Makeall_CRITERIA1D
qmake CONFIG+=release
nmake /S /NOLOGO release

:: build SOIL_EDITOR
cd ..\Makeall_SOIL_EDITOR
qmake CONFIG+=release
nmake /S /NOLOGO release

:: build CRITERIA1D_PRO
cd ..\Makeall_CRITERIA1D_PRO
qmake CONFIG+=release
nmake /S /NOLOGO release

:: build HEAT1D
cd ..\Makeall_HEAT1D
qmake CONFIG+=release
nmake /S /NOLOGO release

:: build mapGraphics
cd ..\..\mapGraphics
qmake CONFIG+=release
nmake /S /NOLOGO release

:: build CRITERIAGEO
cd ..\bin\Makeall_CRITERIAGEO
qmake CONFIG+=release
nmake /S /NOLOGO release

:: copy executables
cd ..\..\deploy
mkdir CRITERIA1D\bin
cd CRITERIA1D\bin
copy ..\..\..\bin\HEAT1D\release\HEAT1D.exe
copy ..\..\..\bin\CRITERIA1D\release\CRITERIA1D.exe
copy ..\..\..\bin\SOIL_EDITOR\release\SOIL_EDITOR.exe
copy ..\..\..\bin\CRITERIA1D_PRO\release\CRITERIA1D_PRO.exe
copy ..\..\..\bin\CRITERIAGEO\release\CRITERIA_GEO.exe


:: deploy executables
windeployqt HEAT1D.exe
windeployqt CRITERIA1D.exe
windeployqt SOIL_EDITOR.exe
windeployqt CRITERIA1D_PRO.exe
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

:: copy settings and template
cd ..\..\
mkdir DATA\SETTINGS
cd DATA\SETTINGS
xcopy /Y ..\..\..\..\DATA\SETTINGS\*.*

cd ..\..\
mkdir DATA\TEMPLATE
cd DATA\TEMPLATE
xcopy /Y ..\..\..\..\DATA\TEMPLATE\*.*

:: copy test project
cd ..\
mkdir PROJECT\test
cd PROJECT\test
xcopy /s /Y ..\..\..\..\..\DATA\PROJECT\test\*.* 


:: return to deploy directory
cd ..\..\..\..\
