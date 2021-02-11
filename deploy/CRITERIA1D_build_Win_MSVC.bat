:: build CRITERIA1D
:: run on Qt shell (MSVC version)
:: inside deploy directory (cd [local path]\CRITERIA1D\deploy)
:: before the execution call vcvarsall.bat (32 or 64 bit) to complete environment setup
:: example: C:\"Program Files (x86)"\"Microsoft Visual Studio"\2017\Community\VC\Auxiliary\Build\vcvars64.bat


:: clean all
cd ..\mapGraphics
nmake /S /NOLOGO distclean
cd ..\tools\csvToMeteoDb
nmake /S /NOLOGO distclean
cd ..\Makeall_CriteriaOutput
nmake /S /NOLOGO distclean
cd ..\..\bin\Makeall_CRITERIA1D
nmake /S /NOLOGO distclean
cd ..\Makeall_CROP_EDITOR
nmake /S /NOLOGO distclean
cd ..\Makeall_SOIL_EDITOR
nmake /S /NOLOGO distclean
cd ..\Makeall_HEAT1D
nmake /S /NOLOGO distclean
cd ..\Makeall_CRITERIAGEO
nmake /S /NOLOGO distclean

:: mapGraphics
cd ..\..\mapGraphics
qmake CONFIG+=release
nmake /S /NOLOGO release

:: TOOLS
cd ..\tools\csvToMeteoDb
qmake CONFIG+=release
nmake /S /NOLOGO release

cd ..\Makeall_CriteriaOutput
qmake CONFIG+=release
nmake /S /NOLOGO release

:: CRITERIA-1D/GEO
cd ..\..\bin\Makeall_CRITERIA1D
qmake CONFIG+=release
nmake /S /NOLOGO release

cd ..\Makeall_CROP_EDITOR
qmake CONFIG+=release
nmake /S /NOLOGO release

cd ..\Makeall_SOIL_EDITOR
qmake CONFIG+=release
nmake /S /NOLOGO release

cd ..\Makeall_HEAT1D
qmake CONFIG+=release
nmake /S /NOLOGO release

cd ..\Makeall_CRITERIAGEO
qmake CONFIG+=release
nmake /S /NOLOGO release

:: copy executables
cd ..\..\deploy
mkdir CRITERIA1D\bin
cd CRITERIA1D\bin
copy ..\..\..\bin\HEAT1D\release\HEAT1D.exe
copy ..\..\..\bin\CRITERIAGEO\release\CRITERIA_GEO.exe
copy ..\..\..\bin\CRITERIA1D\release\CRITERIA1D.exe
copy ..\..\..\bin\CROP_EDITOR\release\CROP_EDITOR.exe
copy ..\..\..\bin\SOIL_EDITOR\release\SOIL_EDITOR.exe
copy ..\..\..\tools\criteriaOutputTools\release\CriteriaOutput.exe
copy ..\..\..\tools\csvToMeteoDb\release\CsvToMeteoDb.exe

:: deploy executables
windeployqt HEAT1D.exe
windeployqt CRITERIA1D.exe
windeployqt CriteriaOutput.exe
windeployqt CROP_EDITOR.exe
windeployqt SOIL_EDITOR.exe
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

:: copy kiwifruit project
cd ..\
mkdir PROJECT\kiwifruit
cd PROJECT\kiwifruit
xcopy /s /Y ..\..\..\..\..\DATA\PROJECT\kiwifruit\*.* 

:: return to deploy directory
cd ..\..\..\..\
