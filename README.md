[![Build Status](https://github.com/arpa-simc/CRITERIA1D/actions/workflows/build-ubuntu-xenial.yml/badge.svg)](https://github.com/ARPA-SIMC/CRITERIA1D/actions/workflows/build-ubuntu-xenial.yml)
[![Build Status](https://simc.arpae.it/moncic-ci/CRITERIA1D/centos8.png)](https://simc.arpae.it/moncic-ci/CRITERIA1D/)
[![Build Status](https://simc.arpae.it/moncic-ci/CRITERIA1D/fedora34.png)](https://simc.arpae.it/moncic-ci/CRITERIA1D/)
[![Build Status](https://copr.fedorainfracloud.org/coprs/simc/stable/package/CRITERIA1D/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/simc/stable/package/CRITERIA1D/)

# CRITERIA1D / GEO
CRITERIA-1D is an agro-hydrological model that simulates one-dimensional water flow in variable saturation soils, crop development, root water extraction and irrigation water needs. Soil water flow can be simulated with two different approaches depending on the user's choice: a physically based numerical model or a layer-based conceptual model. Soil and crop parameters can be defined at different levels of detail. It requires daily agro-meteorological data as input: minimum and maximum air temperature, total precipitation and, if available, water table depth data to estimate capillary rise.

CRITERIA-GEO is a GIS interface for managing geo-referenced model projects. Requires a crop map, a soil map, and a meteorological grid; a computation unit is defined as a different combination of crop, soil and meteo. The output is stored in a SQLite database and can be exported to csv, shapefile or raster data using the [CriteriaOutput](https://github.com/ARPA-SIMC/agrotools) tool. Software is written in C++ using Qt libraries, so cross-platform building is possible (Windows, Linux, Mac OS). 

See [thechnical manual](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/CRITERIA1D_technical_manual.pdf) for more information and [last release](https://github.com/ARPA-SIMC/CRITERIA1D/releases) to download precompiled binaries.

CRITERIA is operational at [Arpae Emilia-Romagna](https://www.arpae.it/it/temi-ambientali/meteo/scopri-di-piu/strumenti-di-modellistica/criteria/criteria-modello-di-bilancio-idrico) and in the [Climate Service for Irrigation Forecasting](https://servizigis.arpae.it/moses/home/index.html). It has been used in several international projects (Demeter, Ensembles, Vintage, Moses, Clara, Highlander, ADA) and it is reported in the [International Soil Modeling Consortium](https://soil-modeling.org/resources-links/model-portal/criteria).

[Criteria-1D API](https://criteria.vaimee.it/) is a project of [VAIMEE](https://vaimee.com/) to integrate the model into the most diverse applications, such as in [ZENTRA cloud](https://ieeexplore.ieee.org/document/9628475).

[BIBLIOGRAPHY](https://www.arpae.it/it/temi-ambientali/meteo/scopri-di-piu/strumenti-di-modellistica/criteria/criteria-bibliografia)

# CRITERIA1D_PRO
CRITERIA1D_PRO is a graphical interface to manage a Criteria-1D project, modify the crop parameters, test the model and the main outputs: leaf area index, evaporation and transpiration, root depth and root density, soil moisture and irrigation water needs. 

![](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/img/cropEditor.png)

# CRITERIA GEO
CRITERIA-GEO is a GIS interface for managing geographical input/output of CRITERIA-1D model: crop map, soil map, meteorological grid and computation units map. A computation unit is defined as a different combination of crop, soil and meteo data.

![](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/img/criteriaGeo.png)

## SOIL_EDITOR
Soil editor is a tool to edit the soil properties. It uses a modified Van Genuchten - Mualem model to estimate the water retention and hydraulic conductivity curves. 

![](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/img/soilEditor.png)

## HEAT1D 
HEAT1D is a graphical interface for testing the soilFluxex3D library in a 1D domain. Users can set fixed or variable atmospheric boundary conditions and soil parameters. Output graph results include soil temperature, soil water content, heat fluxes (diffusive and latent), liquid (isothermal and thermal) and vapor (isothermal and thermal) water fluxes.

![](https://github.com/ARPA-SIMC/CRITERIA1D/blob/master/DOC/img/heat1D.png)

## How to compile the CRITERIA-1D/GEO distribution
Dependencies:
- [Qt libraries](https://www.qt.io/download-qt-installer): Qt 5.x or following is needed (download also QtCharts and Qt 5 Compatibility Module for Qt6.x).

Only for CRITERIA GEO:
- [NetCDF library](https://www.unidata.ucar.edu/downloads/netcdf/): on Windows download and install NetCDF 4.x (32 or 64 bit version, depending on your compiler), then set Netcdf installation path (e.g C:\Program Files\netCDF 4.7.1) in the *NC4_INSTALL_DIR* system variable.

Build:
- LINUX: run *deploy/CRITERIA1D_build_Linux.sh*
- WINDOWS (Visual Studio compiler): run *deploy/CRITERIA1D_build_MSVC_bat*
- WINDOWS (MinGW compiler): *run deploy/CRITERIA1D_build_MinGW_bat*       
- MACOS: TODO
- ALL platforms: please read the information in the first few lines of the script.

The distribution will be created in the directory deploy/CRITERIA1D.


## License
CRITERIA-1D has been developed under contract issued by 
[ARPAE Hydro-Meteo-Climate Service](https://github.com/ARPA-SIMC), Emilia-Romagna, Italy.

The executables (*CRITERIA1D, CRITERIA_GEO, SOIL_EDITOR, HEAT1D*) are released under the GNU GPL license, the libraries (*agrolib*) are released under the GNU LGPL license.

## Contacts
- Fausto Tomei ftomei@arpae.it
- Gabriele Antolini gantolini@arpae.it

