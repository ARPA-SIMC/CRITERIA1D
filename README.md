# CRITERIA-1D/GEO
CRITERIA-1D (console application) is a soil water balance model simulating one-dimensional water fluxes, crop development and crop water needs. The soil and crop parameters can be defined at different level of detail. It requires as input daily meteorological data (temperature and precipitation) and daily data of watertable depth to estimate the capillary rise.

CRITERIA-GEO is a GIS interface for managing geographical input/output of the model: crop map, soil map, meteorological grid and computation units map. A computation unit is defined as a different combination of crop, soil and meteo data. 

CROP_EDITOR and SOIL_EDITOR are tools to manage and test the crop and soil parameters of the model.

## CROP_EDITOR
Crop editor is a tool to manage the crop parameters, test the model and show the main output (Leaf Area Index, evaporation and transpiration, root depth and root density, soil moisture and irrigation water needs). 

![](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/img/cropEditor.png)

## SOIL_EDITOR
Soil editor is a tool to edit the soil properties. It uses a modified Van Genuchten - Mualem model to estimate the water retention and hydraulic conductivity curves. 

![](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/img/soilEditor.png)

## HEAT1D 
#### bin/Makeall_HEAT1D
HEAT1D is a graphical interface for testing the soilFluxex3D library in a 1D domain. Users can set fixed or variable atmospheric boundary conditions and soil parameters. Output graph results include soil temperature, soil water content, heat fluxes (diffusive and latent), liquid (isothermal and thermal) and vapor (isothermal and thermal) water fluxes.  
Graphs are produced using the Qwt plot library (http://qwt.sourceforge.net) which should be compiled before usage.

## How to compile the CRITERIA-1D distribution
Dependencies:
- [Qt libraries](https://www.qt.io/download-qt-installer): Qt 5.x or following is needed (download also QtCharts).

Build:
- Linux: run deploy/CRITERIA1D_build_Linux.sh
- Windows (Visual Studio compiler): run deploy/CRITERIA1D_build_MSVC_bat
- Windows (MinGW compiler): run deploy/CRITERIA1D_build_MinGW_bat

The distribution will be created in deploy/CRITERIA1D


## License
CRITERIA-1D has been developed under contract issued by 
[ARPAE Hydro-Meteo-Climate Service](https://github.com/ARPA-SIMC), Emilia-Romagna, Italy

The executables (*CRITERIA1D, CRITERIAGEO, TEST_EDITOR, SOIL_EDITOR, HEAT1D*) are released under the GNU GPL license, libreries (*agrolib*) are released under the GNU LGPL license.
