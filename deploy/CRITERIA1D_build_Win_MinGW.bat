:: build CRITERIA-1D
:: run on Qt shell (MINGW version) 
:: inside deploy directory (cd [local path]\CRITERIA1D\deploy)


:: build csvToMeteoDb
cd ..\tools\csvToMeteoDb
qmake csvToMeteoDb.pro CONFIG+=release
mingw32-make clean
mingw32-make release

:: build criteriaOutput
cd ..\Makeall_CriteriaOutput
qmake Makeall_CriteriaOutput.pro CONFIG+=release
mingw32-make clean
mingw32-make release

:: build CRITERIA1D
cd ..\..\bin\Makeall_CRITERIA1D
qmake Makeall_CRITERIA1D.pro CONFIG+=release
mingw32-make clean
mingw32-make release

:: build SOIL_EDITOR
cd ..\Makeall_SOIL_EDITOR
qmake Makeall_SOIL_EDITOR.pro CONFIG+=release
mingw32-make release

:: build CROP_EDITOR
cd ..\Makeall_CROP_EDITOR
qmake Makeall_CROP_EDITOR.pro CONFIG+=release
mingw32-make release


:: copy executables
cd ..\..\DEPLOY
mkdir CRITERIA1D\bin
cd CRITERIA1D\bin
copy ..\..\..\tools\csvToMeteoDb\release\CsvToMeteoDb.exe
copy ..\..\..\tools\criteriaOutputTools\release\CriteriaOutput.exe
copy ..\..\..\bin\CRITERIA1D\release\CRITERIA1D.exe
copy ..\..\..\bin\SOIL_EDITOR\release\SOIL_EDITOR.exe
copy ..\..\..\bin\CROP_EDITOR\release\CROP_EDITOR.exe


:: deploy
windeployqt CsvToMeteoDb.exe
windeployqt CriteriaOutput.exe
windeployqt CRITERIA1D.exe
windeployqt SOIL_EDITOR.exe
windeployqt CROP_EDITOR.exe


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

:: copy kiwifruit project
cd ..\..\
mkdir DATA\PROJECT\kiwifruit
cd DATA\PROJECT\kiwifruit
xcopy /s /Y ..\..\..\..\..\DATA\PROJECT\kiwifruit\*.* 

:: return to deploy directory
cd ..\..\..\..\


