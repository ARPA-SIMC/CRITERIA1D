TEMPLATE = subdirs

SUBDIRS =   ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis  \
            ../../agrolib/utilities  ../../agrolib/shapeHandler \
            ../../agrolib/shapeUtilities ../../agrolib/criteriaOutput ../criteriaOutputTools

# comment to compile without GDAL library
SUBDIRS += ../../agrolib/gdalHandler

CONFIG += ordered 

