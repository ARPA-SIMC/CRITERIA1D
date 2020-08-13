:: build CRITERIA1D
:: run on Qt shell (MSVC version)
:: inside deploy directory (cd [local path]\CRITERIA1D\deploy)
:: before execution remember to call vcvarsall.bat to complete environment setup
:: example: C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat x64


:: build csvToMeteoDb
cd ..\tools\csvToMeteoDb
qmake CONFIG+=release
nmake /S /NOLOGO clean
nmake /S /NOLOGO release

:: build criteriaOutput
cd ..\Makeall_CriteriaOutput
qmake CONFIG+=release
nmake /S /NOLOGO clean
nmake /S /NOLOGO release

:: build CRITERIA-1D
cd ..\..\bin\Makeall_CRITERIA1D
qmake CONFIG+=release
nmake /S /NOLOGO clean
nmake /S /NOLOGO release

:: build CROP_EDITOR
cd ..\Makeall_CROP_EDITOR
qmake CONFIG+=release
nmake /S /NOLOGO clean
nmake /S /NOLOGO release

:: build SOIL_EDITOR
cd ..\Makeall_SOIL_EDITOR
qmake CONFIG+=release
nmake /S /NOLOGO clean
nmake /S /NOLOGO release

:: copy executables
cd ..\..\deploy
mkdir CRITERIA1D\bin
cd CRITERIA1D\bin
copy ..\..\..\bin\CRITERIA1D\release\CRITERIA1D.exe
copy ..\..\..\bin\CROP_EDITOR\release\CROP_EDITOR.exe
copy ..\..\..\bin\SOIL_EDITOR\release\SOIL_EDITOR.exe
copy ..\..\..\tools\criteriaOutputTools\release\CriteriaOutput.exe
copy ..\..\..\tools\csvToMeteoDb\release\CsvToMeteoDb.exe

:: CLEAN distribution
cd ..\..\..\tools\csvToMeteoDb
nmake /S /NOLOGO distclean
cd ..\Makeall_CriteriaOutput
nmake /S /NOLOGO distclean
cd ..\..\bin\Makeall_CRITERIA1D
nmake /S /NOLOGO distclean
cd ..\Makeall_CROP_EDITOR
nmake /S /NOLOGO distclean
cd ..\Makeall_SOIL_EDITOR
nmake /S /NOLOGO distclean

:: deploy executables
cd ..\..\deploy\CRITERIA1D\bin
windeployqt CsvToMeteoDb.exe
windeployqt CriteriaOutput.exe
windeployqt CRITERIA1D.exe
windeployqt CROP_EDITOR.exe
windeployqt SOIL_EDITOR.exe

:: copy img files
cd ..\
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

:: copy kiwifruit project
cd ..\
mkdir PROJECT\kiwifruit
cd PROJECT\kiwifruit
xcopy /s /Y ..\..\..\..\..\DATA\PROJECT\kiwifruit\*.* 

:: return to deploy directory
cd ..\..\..\..\

