# CRITERIA-1D/GEO
CRITERIA-1D is a one-dimensional model simulating the soil water balance and the crop development (console application).

CRITERIA-GEO is a GIS interface for managing CRITERIA-1D geographical input/output: crop map, soil map, meteorological grid and simulation units.

CROP_EDITOR and SOIL_EDITOR are tools to manage crop and soil parameters of the model.

## How to compile
Dependencies:
- [Qt libraries](https://www.qt.io/download-qt-installer): Qt 5.x or following is needed (download also QtCharts).
- [MapGraphics library](https://github.com/raptorswing/MapGraphics) (modified version): build with Qt Creator the project *mapGraphics/MapGraphics.pro*

Build:
- deploy [..]

## CROP_EDITOR
#### bin/Makeall_CROP_EDITOR
crop editor is a tool to [..]

![](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/img/cropEditor.png)

## SOIL_EDITOR
#### bin/Makeall_SOIL_EDITOR
Soil editor is a tool to show/edit soil data, including water retention and hydrologic conductivity curves, using modified Van Genuchten - Mualem model.

![](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/img/soilEditor.png)


## HEAT1D 
#### bin/Makeall_HEAT1D
HEAT1D is a graphical interface for testing the soilFluxex3D library in a 1D domain. Users can set fixed or variable atmospheric boundary conditions and soil parameters. Output graph results include soil temperature, soil water content, heat fluxes (diffusive and latent), liquid (isothermal and thermal) and vapor (isothermal and thermal) water fluxes.  
Graphs are produced using the Qwt plot library (http://qwt.sourceforge.net) which should be compiled before usage.



## License
CRITERIA-1D has been developed under contract issued by 
ARPAE Hydro-Meteo-Climate Service, Emilia-Romagna, Italy: http://www.arpae.it/sim/

The executables (*CRITERIA1D, CRITERIAGEO, TEST_EDITOR, SOIL_EDITOR*) are released under the GNU GPL license, libreries (*agrolib*) are released under the GNU LGPL license.