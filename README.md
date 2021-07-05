[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/CRITERIA1D?branch=master&env=DOCKER_IMAGE=centos:8&label=centos8)](https://travis-ci.org/ARPA-SIMC/CRITERIA1D)
[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/CRITERIA1D?branch=master&env=DOCKER_IMAGE=ubuntu:xenial&label=ubuntuxenial)](https://travis-ci.org/ARPA-SIMC/CRITERIA1D)
[![Build Status](https://copr.fedorainfracloud.org/coprs/simc/stable/package/CRITERIA1D/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/simc/stable/package/CRITERIA1D/)

# CRITERIA-1D
CRITERIA-1D is a agro-hydrological model simulating one-dimensional soil water fluxes, crop development and crop water needs. The soil and crop parameters can be defined at different levels of detail. It requires as input daily agro-meteorological data: minimum and maximum air temperature, total precipitation and, if available, data of water table depth to estimate the capillary rise. 

The output is stored in a SQLite database and can be exported to csv, shapefile or raster data using the CriteriaOutput tool. The software is multi-platform (Windows, Linux, Mac OS).

## CRITERIA GEO
CRITERIA-GEO is a GIS interface for managing geographical input/output of CRITERIA-1D model: crop map, soil map, meteorological grid and computation units map. A computation unit is defined as a different combination of crop, soil and meteo data.

![](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/img/criteriaGeo.png)

## CROP_EDITOR
Crop editor is a tool to manage the crop parameters, test the model and show the main output (Leaf Area Index, evaporation and transpiration, root depth and root density, soil moisture and irrigation water needs). 

![](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/img/cropEditor.png)

## SOIL_EDITOR
Soil editor is a tool to edit the soil properties. It uses a modified Van Genuchten - Mualem model to estimate the water retention and hydraulic conductivity curves. 

![](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/img/soilEditor.png)

## HEAT1D 
HEAT1D is a graphical interface for testing the soilFluxex3D library in a 1D domain. Users can set fixed or variable atmospheric boundary conditions and soil parameters. Output graph results include soil temperature, soil water content, heat fluxes (diffusive and latent), liquid (isothermal and thermal) and vapor (isothermal and thermal) water fluxes.

![](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/img/heat1D.png)

## How to compile the CRITERIA-1D/GEO distribution
Dependencies:
- [Qt libraries](https://www.qt.io/download-qt-installer): Qt 5.x or following is needed (download also QtCharts).
- [GDAL](https://gdal.org/): (only for CRITERIA GEO) follow the instructions in agrolib/gdal.pri (for windows: it works only with Visual Studio compiler)

Build:
- Linux: run deploy/CRITERIA1D_build_Linux.sh
- Windows (Visual Studio compiler): run deploy/CRITERIA1D_build_MSVC_bat
- Windows (MinGW compiler): run deploy/CRITERIA1D_build_MinGW_bat       
- ALL: please read the info in the first lines of the script

The distribution will be created in the directory deploy/CRITERIA1D


## License
CRITERIA-1D has been developed under contract issued by 
[ARPAE Hydro-Meteo-Climate Service](https://github.com/ARPA-SIMC), Emilia-Romagna, Italy

The executables (*CRITERIA1D, CRITERIA_GEO, CROP_EDITOR, SOIL_EDITOR, HEAT1D*) are released under the GNU GPL license, the libraries (*agrolib*) are released under the GNU LGPL license.

## Contacts
- Fausto Tomei ftomei@arpae.it
- Gabriele Antolini gantolini@arpae.it

CRITERIA-1D is operational at [Arpae Emilia-Romagna](https://www.arpae.it/it/temi-ambientali/meteo/scopri-di-piu/strumenti-di-modellistica/criteria/criteria-modello-di-bilancio-idrico) and has been used in various international projects (Demeter, Ensembles, Vintage, Moses, Clara, Highlander)

[Criteria API](https://criteria.vaimee.it/) is a project of [VAIMEE](https://vaimee.com/) to provide the power of the model to the most diverse applications.

[CRITERIA-1D on International Soil Modeling Consortium](https://soil-modeling.org/resources-links/model-portal/criteria)

[BIBLIOGRAPHY](https://www.arpae.it/it/temi-ambientali/meteo/scopri-di-piu/strumenti-di-modellistica/criteria/criteria-bibliografia)
