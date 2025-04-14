/*!
    \file mainWindow.cpp

    \abstract Praga Gis mainwindow

    \copyright
    This file is part of CRITERIA-3D distribution.
    CRITERIA-3D has been developed by A.R.P.A.E. Emilia-Romagna.

    CRITERIA-3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    CRITERIA-3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License
    along with CRITERIA-3D.  If not, see <http://www.gnu.org/licenses/>.

    \authors
    Fausto Tomei ftomei@arpae.it
    Gabriele Antolini gantolini@arpae.it
    Laura Costantini laura.costantini0@gmail.com
*/

#include <cmath>

#include "basicMath.h"

#include "formSelection.h"
#include "dialogSelectField.h"
#include "dialogUcmPrevailing.h"
#include "dialogUcmIntersection.h"
#include "dialogShapeAnomaly.h"
#include "dialogOutputMap.h"
#include "dialogDbfTable.h"
#include "commonConstants.h"
#include "shapeUtilities.h"
#include "utilities.h"
#include "formInfo.h"
#include "gis.h"


#ifdef GDAL
    #include "gdalExtensions.h"
    #include "gdalShapeFunctions.h"
#endif

#include "mainWindow.h"
#include "ui_mainWindow.h"


#define MAPBORDER 11
#define INFOHEIGHT 42
#define TOOLSWIDTH 260


CriteriaGeoProject myProject;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("CRITERIA GEO  " + QString(CRITERIA_VERSION));

    isDoubleClick = false;

    // initialize info dialog for shape
    shapeInfoDialog.setWindowTitle("Shape info");
    shapeInfoBrowser.setFixedSize(300,500);
    QVBoxLayout shapeLayout;
    shapeLayout.addWidget(&shapeInfoBrowser);
    shapeInfoDialog.setLayout(&shapeLayout);
    shapeInfoDialog.setWindowFlag(Qt::WindowStaysOnTopHint);

    // Set the MapGraphics Scene and View
    this->mapScene = new MapGraphicsScene(this);
    this->mapView = new MapGraphicsView(mapScene, this->ui->widgetMap);

    // Set tiles source
    this->setTileSource(WebTileSource::OPEN_STREET_MAP);

    // Set start size and position
    this->startCenter = new Position (myProject.gisSettings.startLocation.longitude,
                                     myProject.gisSettings.startLocation.latitude, 0.0);
    this->mapView->setZoomLevel(8);
    this->mapView->centerOn(startCenter->lonLat());
    connect(this->mapView, SIGNAL(zoomLevelChanged(quint8)), this, SLOT(updateMaps()));
    connect(this->mapView, SIGNAL(mouseMoveSignal(QPoint)), this, SLOT(mouseMove(QPoint)));

    connect(ui->checkList, &QListWidget::itemClicked, [=](QListWidgetItem* item){ this->itemClicked(item); });
    ui->checkList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->checkList, &QListWidget::customContextMenuRequested, [=](const QPoint point){ this->itemMenuRequested(point); });

    this->setMouseTracking(true);
}


MainWindow::~MainWindow()
{
    shapeInfoDialog.close();
    shapeInfoDialog.deleteLater();

    if (! rasterObjList.empty())
    {
        for (unsigned int i = 0; i < rasterObjList.size(); i++)
        {
            delete rasterObjList[i];
        }
    }
    rasterObjList.clear();

    if (! shapeObjList.empty())
    {
        for (unsigned int i = 0; i < shapeObjList.size(); i++)
        {
            shapeObjList[i]->getShapePointer()->close();
            delete shapeObjList[i];
        }
    }
    shapeObjList.clear();
    myProject.objectList.clear();

    if (myProject.output.isProjectLoaded)
    {
        closeGeoProject();
    }

    ui->checkList->clear();
    delete mapView;
    delete mapScene;
    delete ui;
}


void MainWindow::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event)

    ui->widgetMap->setGeometry(TOOLSWIDTH, 0, this->width()-TOOLSWIDTH, this->height() - INFOHEIGHT);
    mapView->resize(ui->widgetMap->size());

    ui->checkList->move(MAPBORDER/2, MAPBORDER);
    ui->checkList->resize(TOOLSWIDTH, this->height() - INFOHEIGHT - MAPBORDER*2);
    this->updateMaps();
}


void MainWindow::updateMaps()
{
    if (! rasterObjList.empty())
        for (unsigned int i = 0; i < rasterObjList.size(); i++)
            rasterObjList[i]->updateCenter();

    if (! shapeObjList.empty())
        for (unsigned int i = 0; i < shapeObjList.size(); i++)
            shapeObjList[i]->updateCenter();
}


void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (! this->isDoubleClick)
    {
        if (event->button() == Qt::LeftButton)
        {
            selectShape(event->pos());
        }
    }

    this->isDoubleClick = false;
}


void MainWindow::mouseDoubleClickEvent(QMouseEvent * event)
{
    this->isDoubleClick = true;

    QPoint mapPos = getMapPos(event->pos());
    if (! isInsideMap(mapPos)) return;

    Position newCenter = this->mapView->mapToScene(mapPos);
    this->ui->statusBar->showMessage(QString::number(newCenter.latitude()) + " " + QString::number(newCenter.longitude()));

    if (event->button() == Qt::LeftButton)
        this->mapView->zoomIn();
    else if (event->button() == Qt::RightButton)
        this->mapView->zoomOut();

    this->mapView->centerOn(newCenter.lonLat());
}


void MainWindow::mouseMove(QPoint eventPos)
{
    if (! isInsideMap(eventPos)) return;

    Position pos = this->mapView->mapToScene(eventPos);

    int rasterIndex = getSelectedRasterPos(false);
    QString rasterValueStr = "";
    if (rasterIndex != NODATA)
    {
        GisObject* myObject = myProject.objectList.at(unsigned(rasterIndex));
        gis::Crit3DRasterGrid *myRaster = myObject->getRasterPointer();

        double utmX, utmY;
        gis::getUtmFromLatLon(myProject.gisSettings, pos.latitude(), pos.longitude(), &utmX, &utmY);
        float value = myRaster->getValueFromXY(utmX, utmY);
        if (! isEqual(value, myRaster->header->flag))
        {
            rasterValueStr = QString::number(value);
        }
    }

    QString infoStr = "Lat:" + QString::number(pos.latitude()) + "  Lon:" + QString::number(pos.longitude());
    if (rasterValueStr != "")
    {
        infoStr += " Value: " + rasterValueStr;
    }

    this->ui->statusBar->showMessage(infoStr);
}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    // it doesn't work (control taken by mapGraphics)
}

void MainWindow::on_actionMapOpenStreetMap_triggered()
{
    this->setTileSource(WebTileSource::OPEN_STREET_MAP);
}

void MainWindow::on_actionMapStamenTerrain_triggered()
{
    this->setTileSource(WebTileSource::STAMEN_Terrain);
}

void MainWindow::on_actionMapESRISatellite_triggered()
{
    this->setTileSource(WebTileSource::ESRI_WorldImagery);
}

void MainWindow::on_actionMapGoogle_triggered()
{
    this->setTileSource(WebTileSource::GOOGLE_MAP);
}

void MainWindow::on_actionMapGoogleSatellite_triggered()
{
    this->setTileSource(WebTileSource::GOOGLE_Satellite);
}

void MainWindow::on_actionMapGoogleHybridSatellite_triggered()
{
    this->setTileSource(WebTileSource::GOOGLE_Hybrid_Satellite);
}

void MainWindow::on_actionMapGoogleTerrain_triggered()
{
    this->setTileSource(WebTileSource::GOOGLE_Terrain);
}


void MainWindow::setTileSource(WebTileSource::WebTileType tileType)
{
    // deselect all menu
    ui->actionMapOpenStreetMap->setChecked(false);
    ui->actionMapStamenTerrain->setChecked(false);
    ui->actionMapESRISatellite->setChecked(false);
    ui->actionMapGoogle->setChecked(false);
    ui->actionMapGoogleSatellite->setChecked(false);
    ui->actionMapGoogleHybridSatellite->setChecked(false);
    ui->actionMapGoogleTerrain->setChecked(false);

    // select menu
    switch(tileType)
    {
        case WebTileSource::OPEN_STREET_MAP:
            ui->actionMapOpenStreetMap->setChecked(true);
            break;

        case WebTileSource::STAMEN_Terrain:
            ui->actionMapStamenTerrain->setChecked(true);
            break;

        case WebTileSource::ESRI_WorldImagery:
            ui->actionMapESRISatellite->setChecked(true);
            break;

        case WebTileSource::GOOGLE_MAP:
            ui->actionMapGoogle->setChecked(true);
            break;

        case WebTileSource::GOOGLE_Satellite:
            ui->actionMapGoogleSatellite->setChecked(true);
            break;

        case WebTileSource::GOOGLE_Hybrid_Satellite:
            ui->actionMapGoogleHybridSatellite->setChecked(true);
            break;

        case WebTileSource::GOOGLE_Terrain:
             ui->actionMapGoogleTerrain->setChecked(false);
            break;
    }

    // set tiles source
    QSharedPointer<WebTileSource> myTiles(new WebTileSource(tileType), &QObject::deleteLater);
    this->mapView->setTileSource(myTiles);
}


QPoint MainWindow::getMapPos(const QPoint& screenPos)
{
    QPoint mapPos;
    int dx = ui->widgetMap->x();
    int dy = ui->widgetMap->y() + ui->menuBar->height();
    mapPos.setX(screenPos.x() - dx - MAPBORDER);
    mapPos.setY(screenPos.y() - dy - MAPBORDER);
    return mapPos;
}


bool MainWindow::isInsideMap(const QPoint& pos)
{
    if (pos.x() > 0 && pos.y() > 0 &&
        pos.x() < (mapView->width() - MAPBORDER*2) &&
        pos.y() < (mapView->height() - MAPBORDER*2) )
    {
        return true;
    }
    else return false;
}


void MainWindow::addRasterObject(GisObject* myObject)
{
    QListWidgetItem* item = new QListWidgetItem("[RASTER] " + myObject->fileName);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    ui->checkList->addItem(item);

    RasterUtmObject* newRasterObj = new RasterUtmObject(mapView);
    newRasterObj->setOpacity(0.66);
    newRasterObj->initialize(myObject->getRasterPointer(), myObject->gisSettings);
    this->rasterObjList.push_back(newRasterObj);

    this->mapView->scene()->addObject(newRasterObj);
    this->updateMaps();
}


/*
void MainWindow::addNetcdfObject(GisObject* myObject)
{
    QListWidgetItem* item = new QListWidgetItem("[NETCDF] " + myObject->fileName);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    ui->checkList->addItem(item);

    RasterObject* netcdfObj = new RasterObject(this->mapView);
    netcdfObj->setOpacity(0.66);

    NetCDFHandler* netcdfPtr = myObject->getNetcdfHandler();

    if (netcdfPtr->isLatLon || netcdfPtr->isRotatedLatLon)
    {
        netcdfObj->initializeLatLon(netcdfPtr->getRasterPointer(), myObject->gisSettings, netcdfPtr->latLonHeader, true);
    }
    else
    {
        netcdfObj->initializeUTM(netcdfPtr->getRasterPointer(), myObject->gisSettings, true);
    }

    this->rasterObjList.push_back(netcdfObj);

    this->mapView->scene()->addObject(netcdfObj);
    this->updateMaps();
}
*/


bool MainWindow::addShapeObject(GisObject* myObject)
{
    // check zoneNumber
    int zoneNumber = myObject->getShapeHandler()->getUtmZone();
    if ((zoneNumber < 1) || (zoneNumber > 60))
    {
        QMessageBox::critical(nullptr, "ERROR!", "Wrong UTM zone.");
        return false;
    }

    // name
    QString itemName;
    if (myObject->projectName != "")
    {
        itemName = "[PROJECT] " + myObject->projectName;
    }
    else
    {
        itemName = "[SHAPE] " + myObject->fileName;
    }

    // add item
    QListWidgetItem* item = new QListWidgetItem(itemName);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    ui->checkList->addItem(item);

    // add shapeObject
    MapGraphicsShapeObject* newShapeObj = new MapGraphicsShapeObject(this->mapView);
    newShapeObj->initializeUTM(myObject->getShapeHandler());
    newShapeObj->setOpacity(0.8);

    this->shapeObjList.push_back(newShapeObj);
    this->mapView->scene()->addObject(newShapeObj);
    this->updateMaps();

    return true;
}


// resize and center map on last raster size
void MainWindow::zoomOnLastRaster()
{
    RasterUtmObject* myRaster = this->rasterObjList.back();

    Position center = myRaster->getRasterCenter();
    float size = myRaster->getRasterMaxSize();

    this->mapView->setZoomLevel(quint8(log2(float(ui->widgetMap->width()) / size)));
    this->mapView->centerOn(center.longitude(), center.latitude());

    this->updateMaps();
}


void MainWindow::on_actionLoadRaster_triggered()
{
    #ifdef GDAL
        QStringList rasterFormats = getGdalRasterReadExtension();

        rasterFormats.sort();
        rasterFormats.insert(0, tr("all files (*.*)"));
        rasterFormats.insert(1, tr("ESRI float (*.flt)"));
        rasterFormats.insert(2, tr("ENVI IMG (*.img)"));

        QString fileNameWithPath = QFileDialog::getOpenFileName(this, tr("Open raster file"), "", rasterFormats.join(";;"));
    #else
         QString fileNameWithPath = QFileDialog::getOpenFileName(this, tr("Open raster file"), "",
                                                            tr("ESRI FLT (*.flt);;ENVI IMG (*.img)"));
    #endif

    if (fileNameWithPath == "") return;

    QString errorStr;
    if (! myProject.loadRaster(fileNameWithPath, errorStr))
    {
        myProject.logError(errorStr);
        return;
    }

    this->addRasterObject(myProject.objectList.back());
    this->zoomOnLastRaster();
}


/*
void MainWindow::on_actionLoad_NetCDF_triggered()
{
    QString fileNameWithPath = QFileDialog::getOpenFileName(this, tr("Open NetCDF file"), "", tr("NetCDF files (*.nc)"));

    if (fileNameWithPath == "") return;

    if (! myProject.loadNetcdf(fileNameWithPath))
       return;

    GisObject* myObject = myProject.objectList.back();
    this->addNetcdfObject(myObject);
    this->zoomOnLastRaster();
}
*/


void MainWindow::on_actionLoadShapefile_triggered()
{
    QString fileNameWithPath = QFileDialog::getOpenFileName(this, tr("Open Shapefile"), "", tr("shp files (*.shp)"));
    if (fileNameWithPath == "") return;

    if (! myProject.loadShapefile(fileNameWithPath, ""))
        return;

    GisObject* myObject = myProject.objectList.back();

    this->addShapeObject(myObject);
}


int MainWindow::getRasterIndex(GisObject* myObject)
{
    for (unsigned int i = 0; i < rasterObjList.size(); i++)
    {
        if (myObject->type == gisObjectRaster)
        {
            if (rasterObjList.at(i)->getRasterPointer() == myObject->getRasterPointer())
                return i;
        }
        /*
        else if (myObject->type == gisObjectNetcdf)
        {
            if (rasterObjList.at(i)->getRasterPointer() == myObject->getNetcdfHandler()->getRasterPointer())
                return i;
        }
        */
    }

    return NODATA;
}


void MainWindow::itemClicked(QListWidgetItem* item)
{
    int pos = ui->checkList->row(item);
    GisObject* myObject = myProject.objectList.at(unsigned(pos));

    if (myObject->type == gisObjectRaster || myObject->type == gisObjectNetcdf)
    {
        int i = getRasterIndex(myObject);
        if (i != NODATA)
        {
            myObject->isSelected = item->checkState();
            rasterObjList.at(i)->setVisible(myObject->isSelected);
        }
    }
    else if (myObject->type == gisObjectShape)
    {
        unsigned int i;
        for (i = 0; i < shapeObjList.size(); i++)
        {
            if (shapeObjList.at(i)->getShapePointer() == myObject->getShapeHandler())
                break;
        }

        if (i < shapeObjList.size())
        {
            myObject->isSelected = item->checkState();
            shapeObjList.at(i)->setVisible(myObject->isSelected);
        }
    }
}


void MainWindow::saveRaster(GisObject* myObject)
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save raster Grid"), "", tr("ESRI grid files (*.flt)"));
    if (fileName == "") return;

    std::string errorStr;
    fileName = fileName.left(fileName.length() - 4);
    if (! gis::writeEsriGrid(fileName.toStdString(), myObject->getRasterPointer(), errorStr))
    {
        QMessageBox::information(nullptr, "ERROR!", QString::fromStdString(errorStr));
    }
}

void MainWindow::saveShape(GisObject* myObject)
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Shapefile"), "", tr("shp files (*.shp)"));
    if (fileName == "") return;
    // make a copy of shapefile, keep original file
    copyShapeFile(myObject->getFileNameWithPath(), fileName);
}


void MainWindow::removeRaster(GisObject* myObject)
{
    int i = getRasterIndex(myObject);

    if (i != NODATA)
    {
        // remove from scene
        this->mapView->scene()->removeObject(this->rasterObjList.at(i));
        this->rasterObjList.at(i)->clear();
        this->rasterObjList.erase(this->rasterObjList.begin()+i);
    }
}


RasterUtmObject* MainWindow::getRasterObject(GisObject* myObject)
{
    int i = getRasterIndex(myObject);

    if (i != NODATA)
        return rasterObjList.at(i);
    else
        return nullptr;
}


MapGraphicsShapeObject* MainWindow::getShapeObject(GisObject* myObject)
{
    for (unsigned int i = 0; i < shapeObjList.size(); i++)
    {
        if (shapeObjList.at(i)->getShapePointer() == myObject->getShapeHandler())
            return shapeObjList.at(i);
    }

    return nullptr;
}


void MainWindow::removeShape(GisObject* myObject)
{
    unsigned int i;
    for (i = 0; i < shapeObjList.size(); i++)
    {
        if (shapeObjList.at(i)->getShapePointer() == myObject->getShapeHandler()) break;
    }

    // remove from scene
    this->mapView->scene()->removeObject(this->shapeObjList.at(i));
    this->shapeObjList.at(i)->clear();
    this->shapeObjList.erase(this->shapeObjList.begin()+i);
}


void MainWindow::setShapeStyle(GisObject* myObject, std::string fieldName)
{
    MapGraphicsShapeObject* shapeObject = getShapeObject(myObject);
    if (fieldName == "Edges only")
    {
        shapeObject->setFill(false);
        return;
    }

    DBFFieldType fieldType = myObject->getShapeHandler()->getFieldType(fieldName);

    if (fieldType == FTString)
    {
        shapeObject->setCategories(fieldName);
    }
    else
    {
        shapeObject->setNumericValues(fieldName);
    }

    setDefaultScale(shapeObject->colorScale);
    shapeObject->setFill(true);
}


void MainWindow::setShapeStyle_GUI(GisObject* myObject)
{
    bool isOnlyNumeric = false;
    DialogSelectField shapeFieldDialog(myObject->getShapeHandler(), myObject->fileName, isOnlyNumeric, SHAPESTYLE);
    if (shapeFieldDialog.result() == QDialog::Accepted)
    {
        std::string fieldName = shapeFieldDialog.getFieldSelected().toStdString();
        setShapeStyle(myObject, fieldName);
    }
}


bool MainWindow::exportToNetCDF(GisObject* myObject)
{
    DialogSelectField numericField(myObject->getShapeHandler(), myObject->fileName, true, RASTERIZE);
    if (numericField.result() != QDialog::Accepted)
        return false;

    QString field = numericField.getFieldSelected();
    double cellSize = numericField.getCellSize();
    if (cellSize <= 0)
        cellSize = 100;  // default [m]

    QString outputFileName = QFileDialog::getSaveFileName(this, tr("Save NetCDF as"), "", tr("NetCDF files (*.nc)"));
    if (outputFileName == "")
    {
        QMessageBox::information(nullptr, "Insert output name", "missing NetCDF filename");
        return false;
    }

    FormInfo formInfo;
    formInfo.start("Export to NetCDF...", 0);

    std::string variableName = field.toStdString();     // TODO
    std::string unit = "";                              // TODO
    bool isOK = myProject.output.convertShapeToNetcdf(*(myObject->getShapeHandler()), outputFileName.toStdString(),
                                                      field.toStdString(), variableName, unit, cellSize, NO_DATE, 0);
    formInfo.close();

    if (! isOK)
    {
        myProject.logError(myProject.output.projectError);
        return false;
    }

    return true;
}


bool MainWindow::exportShapeToRaster_gdal(GisObject* myObject)
{
#ifdef GDAL
    DialogSelectField shapeFieldDialog(myObject->getShapeHandler(), myObject->fileName, true, GDALRASTER);

    if (shapeFieldDialog.result() != QDialog::Accepted)
        return false;

    QString fieldName = shapeFieldDialog.getFieldSelected();
    std::string shapeFilePath = (myObject->getShapeHandler())->getFilepath();
    QString shapeFileName = QString::fromStdString(shapeFilePath);

    QString resolutionStr;
    if (shapeFieldDialog.getCellSize() == 0)
    {
        resolutionStr = "200";      // default resolution [m]
    }
    else
    {
        resolutionStr = QString::number(shapeFieldDialog.getCellSize());
    }

    QStringList gdalExt = getGdalRasterWriteExtension();
    QString outputName = QFileDialog::getSaveFileName(this, tr("Save raster as"), "", gdalExt.join(";\n"));
    if (outputName == "")
    {
        QMessageBox::information(nullptr, "Insert output name", "missing raster name");
        return false;
    }

    QString errorStr;
    QString newProj = "";               // keep input shape proj
    QString paletteFileName = "";       // no palette
    bool isPngCopy = false;             // no PNG copy
    QString pngFileName = "";
    QString pngProj = "";

    if (! gdalShapeToRaster(shapeFileName, fieldName, resolutionStr, newProj, outputName,
                           paletteFileName, isPngCopy, pngFileName, pngProj, errorStr))
    {
        myProject.logError("GDAL Error: " + errorStr);
        return false;
    }

    if (! myProject.loadRaster(outputName, errorStr))
    {
        myProject.logError("Error in load raster " + errorStr);
        return false;
    }

    addRasterObject(myProject.objectList.back());
    updateMaps();

    return true;
#else
    Q_UNUSED(myObject)
    QMessageBox::critical(nullptr, "ERROR", "Missing GDAL");
    return false;
#endif
}


void MainWindow::rasterStatisticalSummary(GisObject* myObject)         //Qua dentro mettero la chiamata al gis...
{
    std::string errorStdStr;
    int nrValidCells = NODATA;
    float avgValue = NODATA;

    if (! gis::rasterSummary(myObject->getRasterPointer(), nrValidCells, avgValue, errorStdStr))
    {
        QString errorString = QString::fromStdString(errorStdStr);
        myProject.logError(errorString);
        return;
    }

    if (nrValidCells == 0)
    {
        myProject.logWarning("The raster selected has no valid values.");
        return;
    }

    gis::updateMinMaxRasterGrid(myObject->getRasterPointer());

    float area = nrValidCells * myObject->getRasterPointer()->header->cellSize * myObject->getRasterPointer()->header->cellSize / 1000000;

    QDialog myDialog;
    myDialog.setWindowTitle("Raster statistics");

    QTextBrowser textBrowser;

    textBrowser.setText(QString("Raster name: " + myObject->fileName));
    textBrowser.append(QString("Number of valid cells: " + QString::number(nrValidCells)));
    textBrowser.append(QString("Valid area: " + QString::number(area) + " Km2"));
    textBrowser.append(QString("Average: " + QString::number(avgValue)));
    textBrowser.append(QString("Minimum: " + QString::number(myObject->getRasterPointer()->minimum)));
    textBrowser.append(QString("Maximum: " + QString::number(myObject->getRasterPointer()->maximum)));

    QVBoxLayout mainLayout;
    mainLayout.addWidget(&textBrowser);

    myDialog.setLayout(&mainLayout);
    myDialog.setFixedSize(500,170);
    myDialog.exec();
}


void MainWindow::itemMenuRequested(const QPoint point)
{
    QPoint itemPoint = ui->checkList->mapToGlobal(point);
    QListWidgetItem* item = ui->checkList->itemAt(point);
    int pos = ui->checkList->row(item);
    GisObject* myObject = myProject.objectList.at(unsigned(pos));

    QMenu submenu;
    RasterUtmObject* myRasterObject = getRasterObject(myObject);
    MapGraphicsShapeObject* myShapeObject = getShapeObject(myObject);

    if (myObject->type == gisObjectShape)
    {
        if (myShapeObject != nullptr)
        {
            if (! myObject->projectName.isEmpty())
            {
                submenu.addAction("Close Project");
                submenu.addSeparator();
            }
            else
            {
                submenu.addAction("Remove");
                submenu.addSeparator();
                submenu.addAction("Save as");
                submenu.addSeparator();
            }

            submenu.addAction("Show data");
            submenu.addAction("Attribute table");
            submenu.addSeparator();
            submenu.addAction("Set style");
            submenu.addAction("Set grayscale");
            submenu.addAction("Set default scale");
            submenu.addAction("Reverse color scale");
            submenu.addSeparator();
            submenu.addAction("Export to raster (gdal)");
            submenu.addAction("Export to NetCDF");
        }
    }
    if (myObject->type == gisObjectRaster)
    {
        if (myRasterObject != nullptr)
        {
            submenu.addAction("Remove");
            submenu.addSeparator();
            submenu.addAction("Save as");
            submenu.addSeparator();
            submenu.addAction("Set grayscale");
            submenu.addAction("Set default scale");
            submenu.addAction("Set dtm scale");
            submenu.addAction("Reverse color scale");
            submenu.addSeparator();
            submenu.addAction("Statistical summary");
            submenu.addSeparator();

            if (myRasterObject->opacity() < 1)
                submenu.addAction("Set opaque");
            else
                submenu.addAction("Set transparent");
        }
    }
    if (myObject->type == gisObjectNetcdf)
    {
        submenu.addAction("Remove");
        submenu.addSeparator();
    }

    QAction* rightClickItem = submenu.exec(itemPoint);

    if (rightClickItem)
    {
        if (rightClickItem->text() == "Remove" )
        {
            if (myObject->type == gisObjectRaster || myObject->type == gisObjectNetcdf)
            {
                this->removeRaster(myObject);
            }
            else if (myObject->type == gisObjectShape)
            {
                this->removeShape(myObject);
            }
            myObject->close();
            myProject.objectList.erase(myProject.objectList.begin()+pos);

            ui->checkList->takeItem(ui->checkList->indexAt(point).row());
        }
        else if (rightClickItem->text() == "Close Project" )
        {
            on_actionClose_Project_triggered();
        }
        else if (rightClickItem->text().contains("Show data"))
        {
            DialogShapeProperties showData(myObject->getShapeHandler(), myObject->fileName);
        }
        else if (rightClickItem->text().contains("Attribute table"))
        {
            DialogDbfTable Table(myObject->getShapeHandler(), myObject->fileName);
        }
        else if (rightClickItem->text().contains("Set style"))
        {
            setShapeStyle_GUI(myObject);
        }
        else if (rightClickItem->text().contains("Export to raster (gdal)"))
        {
            exportShapeToRaster_gdal(myObject);
        }
        else if (rightClickItem->text().contains("Save as"))
        {
            if (myObject->type == gisObjectRaster)
            {
                this->saveRaster(myObject);
            }
            else if (myObject->type == gisObjectShape)
            {
                this->saveShape(myObject);
            }
        }
        else if (rightClickItem->text().contains("Export to NetCDF"))
        {
            if (myObject->type == gisObjectRaster)
            {
                // TODO
            }
            else if (myObject->type == gisObjectShape)
            {
                this->exportToNetCDF(myObject);
            }
        }
        else if (rightClickItem->text().contains("Set grayscale"))
        {
            if (myObject->type == gisObjectRaster)
            {
                setGrayScale(myObject->getRasterPointer()->colorScale);
                emit myRasterObject->redrawRequested();
            }
            if (myObject->type == gisObjectShape)
            {
                setGrayScale(myShapeObject->colorScale);
                emit myShapeObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Set default scale"))
        {
            if (myObject->type == gisObjectRaster)
            {
                setDefaultScale(myObject->getRasterPointer()->colorScale);
                emit myRasterObject->redrawRequested();
            }
            if (myObject->type == gisObjectShape)
            {
                setDefaultScale(myShapeObject->colorScale);
                emit myShapeObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Set dtm scale"))
        {
            if (myObject->type == gisObjectRaster)
            {
                setDTMScale(myObject->getRasterPointer()->colorScale);
                emit myRasterObject->redrawRequested();
            }
            if (myObject->type == gisObjectShape)
            {
                setDTMScale(myShapeObject->colorScale);
                emit myShapeObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Reverse color scale"))
        {
            if (myObject->type == gisObjectRaster)
            {
                reverseColorScale(myObject->getRasterPointer()->colorScale);
                emit myRasterObject->redrawRequested();
            }
            else if (myObject->type == gisObjectShape)
            {
                reverseColorScale(myShapeObject->colorScale);
                emit myShapeObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Statistical summary"))
        {
            if (myObject->type == gisObjectRaster && myRasterObject != nullptr)
            {
                this->rasterStatisticalSummary(myObject);
            }
        }
        else if (rightClickItem->text().contains("Set opaque"))
        {
            if (myObject->type == gisObjectRaster)
            {
                myRasterObject->setOpacity(1.0);
                emit myRasterObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Set transparent"))
        {
            if (myObject->type == gisObjectRaster)
            {
                // TODO choose value
                myRasterObject->setOpacity(0.66);
                emit myRasterObject->redrawRequested();
            }
        }
    }
}


bool MainWindow::selectShape(QPoint screenPos)
{
    // check if there is an item selected
    QListWidgetItem* itemSelected = ui->checkList->currentItem();
    if (itemSelected == nullptr)
    {
        return false;
    }

    // check if the selected element is a shape
    int row = ui->checkList->row(itemSelected);
    GisObject* myGisObject = myProject.objectList.at(unsigned(row));
    if (myGisObject->type != gisObjectShape)
    {
        return false;
    }

    // check shape object
    MapGraphicsShapeObject *myShapeObject = getShapeObject(myGisObject);
    if (myShapeObject == nullptr)
    {
        return false;
    }

    // check position
    QPoint mapPos = getMapPos(screenPos);
    if (! isInsideMap(mapPos))
    {
        return false;
    }

    Crit3DShapeHandler* shapeHandler = myGisObject->getShapeHandler();
    Position geoPos = mapView->mapToScene(mapPos);

    double x, y;
    gis::latLonToUtmForceZone(myProject.gisSettings.utmZone, geoPos.latitude(), geoPos.longitude(), &x, &y);
    int index = shapeHandler->getShapeIndexfromPoint(x, y);

    // update only if the index is changed
    if (index != myShapeObject->getSelected())
    {
        myShapeObject->setSelected(index);
        emit myShapeObject->redrawRequested();

        // update shape info dialog
        shapeInfoBrowser.setText(QString::fromStdString(shapeHandler->getAttributesList(index)));
        shapeInfoDialog.show();
    }

    return true;
}


void MainWindow::on_actionCompute_Ucm_prevailing_triggered()
{
    if (shapeObjList.empty())
    {
        QMessageBox::information(nullptr, "No shape loaded", "Load crop, soil and meteo shape before.");
        return;
    }

    // create shapehandler list
    std::vector<Crit3DShapeHandler*> shapeList;
    for (unsigned int i = 0; i < shapeObjList.size(); i++)
    {
        shapeList.push_back(shapeObjList.at(i)->getShapePointer());
    }

    DialogUcmPrevailing ucmDialog(shapeList);
    if (ucmDialog.result() == QDialog::Rejected) return;

    QString ucmFileName = QFileDialog::getSaveFileName(this, tr("Save Shapefile"), "", tr("shp files (*.shp)"));
    if (ucmFileName == "") return;

    bool isPrevailing = true;
    double threshold = 0.5;         // TODO aggiungere a ucmDialog

    if (myProject.computeUnitCropMap(ucmDialog.getCrop(), ucmDialog.getSoil(), ucmDialog.getMeteo(),
                                 ucmDialog.getIdCrop().toStdString(), ucmDialog.getIdSoil().toStdString(),
                                 ucmDialog.getIdMeteo().toStdString(), ucmDialog.getCellSize(), threshold,
                                 ucmFileName, isPrevailing))
    {
        addShapeObject(myProject.objectList.back());
    }
}


void MainWindow::on_actionCompute_Ucm_intersection_triggered()
{
    if (shapeObjList.empty())
    {
        QMessageBox::information(nullptr, "No shape loaded", "Load crop, soil and meteo shape before.");
        return;
    }

    // create shapehandler list
    std::vector<Crit3DShapeHandler*> shapeList;
    for (unsigned int i = 0; i < shapeObjList.size(); i++)
    {
        shapeList.push_back(shapeObjList.at(i)->getShapePointer());
    }

    DialogUcmIntersection ucmDialog(shapeList);
    if (ucmDialog.result() == QDialog::Rejected) return;

    QString ucmFileName = QFileDialog::getSaveFileName(this, tr("Save Shapefile"), "", tr("shp files (*.shp)"));
    if (ucmFileName == "") return;

    bool isPrevailing = false;
    double threshold = 0.5;

    if (myProject.computeUnitCropMap(ucmDialog.getCrop(), ucmDialog.getSoil(), ucmDialog.getMeteo(),
                                 ucmDialog.getIdCrop().toStdString(), ucmDialog.getIdSoil().toStdString(),
                                 ucmDialog.getIdMeteo().toStdString(), NODATA, threshold,
                                 ucmFileName, isPrevailing))
    {
        addShapeObject(myProject.objectList.back());
    }

}


void MainWindow::on_actionExtract_Unit_Crop_Map_list_triggered()
{
    QListWidgetItem * itemSelected = ui->checkList->currentItem();
    if (itemSelected == nullptr || !itemSelected->text().contains("SHAPE"))
    {
        QMessageBox::information(nullptr, "No shape selected", "Select a Computational Units Map.");
        return;
    }
    else
    {
        int pos = ui->checkList->row(itemSelected);
        GisObject* myObject = myProject.objectList.at(unsigned(pos));
        Crit3DShapeHandler* shapeHandler = myObject->getShapeHandler();

        myProject.extractUcmListToDb(shapeHandler, true);
     }
}


void MainWindow::on_actionCreate_Shape_file_from_Csv_triggered()
{
    int shapeIndex = getSelectedShapePos();
    if (shapeIndex == NODATA)
        return;

    QString fileCsv = QFileDialog::getOpenFileName(this, tr("Open CSV data file"), "", tr("CSV files (*.csv)"));

    if (fileCsv == "")
    {
        QMessageBox::information(nullptr, "missing CSV data", "Select CSV data to read.");
        return;
    }

    QString fileCsvFormat = QFileDialog::getOpenFileName(this, tr("Open output format"), "", tr("CSV files (*.csv)"));

    if (fileCsvFormat == "")
    {
        QMessageBox::information(nullptr, "missing CSV file", "Select CSV output format.");
        return;
    }

    QString outputFileName = QFileDialog::getSaveFileName(this, tr("Save Shapefile as"), "", tr("shp files (*.shp)"));

    if (outputFileName == "")
    {
        QMessageBox::information(nullptr, "Insert output name", "missing shapefile name");
        return;
    }

    QString errorStr;
    if (! myProject.createShapeFromCsv(shapeIndex, fileCsv, fileCsvFormat, outputFileName, errorStr))
    {
        myProject.logError(errorStr);
    }
}


void MainWindow::on_actionLoadProject_triggered()
{
    // a project is already opened
    if (myProject.output.isProjectLoaded)
    {
        QMessageBox::StandardButton confirm;
        QString msg = "Are you sure you want to close "+myProject.output.projectName+" ?";
        confirm = QMessageBox::question(nullptr, "Warning", msg, QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
        if (confirm == QMessageBox::Yes)
        {
            closeGeoProject();
        }
        else return;
    }

    QString projFileName = QFileDialog::getOpenFileName(this, tr("Open GEO project"), "", tr("Settings files (*.ini)"));

    if (projFileName == "") return;

    // set current dateTime, then GUI overwrite this information
    int myResult = myProject.output.initializeProject(projFileName, "", QDateTime::currentDateTime().date(), false);
    if (myResult != CRIT1D_OK)
    {
        myProject.logError(myProject.output.projectError);
        return;
    }

    if (myProject.output.ucmFileName == "")
    {
        myProject.logError("Missing Unit Crop Map (shapefile)");
        myProject.output.isProjectLoaded = false;
        return;
    }

    QString projectName = getFileName(projFileName);
    projectName = projectName.left(projectName.length() -4);
    if (! myProject.loadShapefile(myProject.output.ucmFileName, projectName))
    {
        myProject.output.isProjectLoaded = false;
        return;
    }

    GisObject* myObject = myProject.objectList.back();
    this->addShapeObject(myObject);

    QDir().mkdir(myProject.output.path + "tmp");

    // enable output map action
    ui->actionOutput_Map->setEnabled(true);
}


void MainWindow::closeGeoProject()
{
    if (!myProject.output.isProjectLoaded)
    {
        return;
    }
    for (unsigned int i = 0; i < myProject.objectList.size(); i++)
    {
        if (myProject.objectList[i]->getFileNameWithPath() == myProject.output.ucmFileName)
        {
            GisObject* myObject = myProject.objectList.at(unsigned(i));
            this->removeShape(myObject);
            myObject->close();
            myProject.objectList.erase(myProject.objectList.begin()+i);
            for (int j = 0; j < ui->checkList->count(); j++)
            {
                if (ui->checkList->item(i)->text().contains("[PROJECT]"))
                {
                    ui->checkList->takeItem(i);
                }
            }
        }
    }

    // remove tmp dir
    QDir tmpDir(myProject.output.path + "tmp");
    tmpDir.removeRecursively();
    myProject.output.closeProject();
}


void MainWindow::on_actionClose_Project_triggered()
{
    if ( !myProject.output.isProjectLoaded )
        return;

    QMessageBox::StandardButton confirm;
    QString msg = "This operation will close the project: " + myProject.output.projectName + "\nAre you sure?";
    confirm = QMessageBox::question(nullptr, "Warning", msg, QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if (confirm == QMessageBox::No)
    {
        return;
    }

    closeGeoProject();

    // disable output map action
    ui->actionOutput_Map->setEnabled(false);

    return;
}


void MainWindow::on_actionOutput_Map_triggered()
{
    if (! myProject.output.getAllDbVariable())
    {
        myProject.logError("Error in load db data variables:\n" + myProject.output.projectError);
        return;
    }
    else
    {
        // add DTX
        myProject.output.outputVariable.varNameList << "DT30" << "DT90" << "DT180" ;
    }
    QDate firstDate;
    QDate lastDate;
    if (! myProject.output.getDbDataDates(firstDate, lastDate))
    {
        myProject.logError("Ivalid dates: " + myProject.output.projectError);
        return;
    }

    DialogOutputMap outputMap(myProject.output.outputVariable.varNameList, firstDate, lastDate);
    if (outputMap.result() != QDialog::Accepted)
    {
        return;
    }
    else
    {
        // fill myProject.output.outputVariable
        QDate dateComputation;
        myProject.output.outputVariable.varNameList.clear();
        myProject.output.outputVariable.varNameList << outputMap.getTabMapVariable();
        if (outputMap.getTabMapElab() == "daily value")
        {
            myProject.output.outputVariable.computationList << ""; // computation is empty
            dateComputation = outputMap.getTabMapDate();
            myProject.output.outputVariable.nrDays << "0";
        }
        else
        {
            myProject.output.outputVariable.computationList << outputMap.getTabMapElab();
            dateComputation = outputMap.getTabMapStartDate();
            myProject.output.outputVariable.nrDays << QString::number(outputMap.getTabMapStartDate().daysTo(outputMap.getTabMapEndDate()));
        }
        myProject.output.outputVariable.referenceDay << 0;
        if (outputMap.isTabMapClimateComputation())
        {
            myProject.output.outputVariable.climateComputation << outputMap.getTabMapClimateComputation();
            myProject.output.outputVariable.param1 << outputMap.getTabMapThreshold();
            myProject.output.outputVariable.param2 << outputMap.getTabMapTimeWindow();
        }
        else
        {
            // climate computation is empty
            myProject.output.outputVariable.climateComputation << "";
        }

        QString fieldName = outputMap.getTabMapVariable();;
        myProject.output.outputVariable.outputVarNameList << fieldName;

        // create CSV and shapeOutput
        QString outputName = outputMap.getTabMapOutputName();
        int result = myProject.createShapeOutput(dateComputation, outputName);
        if (result != CRIT1D_OK)
        {
            myProject.logError("Error in createShapeOutput\n" + myProject.output.projectError);
            return;
        }

        // add shape to GUI
        if (! myProject.loadShapefile(myProject.output.path + "tmp/" + outputName +".shp", ""))
            return;

        GisObject* myObject = myProject.objectList.back();
        this->addShapeObject(myObject);
        setShapeStyle(myObject, "outputVar");
    }
}


void MainWindow::on_actionCompute_anomaly_triggered()
{
    if (shapeObjList.empty() || shapeObjList.size() < 2)
    {
        QMessageBox::information(nullptr, "No shape loaded", "Load shapefiles before.");
        return;
    }

    // create shapehandler list
    std::vector<Crit3DShapeHandler*> shapeList;
    for (unsigned int i = 0; i < shapeObjList.size(); i++)
    {
        shapeList.push_back(shapeObjList.at(i)->getShapePointer());
    }

    DialogShapeAnomaly anomalyDialog(shapeList);
    if (anomalyDialog.result() == QDialog::Rejected) return;

    QString anomalyFileName = QFileDialog::getSaveFileName(this, tr("Save anomaly Shapefile"), "", tr("shp files (*.shp)"));
    if (anomalyFileName == "") return;

    if (myProject.computeShapeAnomaly(anomalyDialog.getClimateShape(), anomalyDialog.getForecastShape(),
                                      anomalyDialog.getIdClimate().toStdString(),
                                      anomalyDialog.getFieldClimate().toStdString(), anomalyDialog.getFieldForecast().toStdString(),
                                      anomalyFileName))
    {
        addShapeObject(myProject.objectList.back());
    }
}


int MainWindow::getSelectedRasterPos(bool isInfo)
{
    if (rasterObjList.empty())
    {
        if (isInfo)
            QMessageBox::information(nullptr, "No raster loaded", "Load a raster before.");
        return NODATA;
    }

    QListWidgetItem * itemSelected = ui->checkList->currentItem();

    if (itemSelected == nullptr || ! itemSelected->text().contains("RASTER"))
    {
        if (isInfo)
            QMessageBox::information(nullptr, "No raster selected", "Select a raster before.");
        return NODATA;
    }

    return ui->checkList->row(itemSelected);
}


int MainWindow::getSelectedShapePos()
{
    if (shapeObjList.empty())
    {
        QMessageBox::information(nullptr, "No shape loaded", "Load a shapefile before.");
        return NODATA;
    }

    QListWidgetItem * itemSelected = ui->checkList->currentItem();

    if (itemSelected == nullptr || ! itemSelected->text().contains("SHAPE"))
    {
        QMessageBox::information(nullptr, "No shape selected", "Select a shapefile before.");
        return NODATA;
    }

    return ui->checkList->row(itemSelected);
}


void MainWindow::on_actionRasterize_all_shape_triggered()
{
    int pos = getSelectedShapePos();
    if (pos == NODATA) return;

    GisObject* myObject = myProject.objectList.at(unsigned(pos));

    DialogSelectField numericField(myObject->getShapeHandler(), myObject->fileName, true, RASTERIZE);
    if (numericField.result() != QDialog::Accepted)
        return;

    double resolution = numericField.getCellSize();
    if (resolution <= 0)
    {
        resolution = 100;  // default [m]
    }

    bool showInfo = true;
    if ( myProject.newRasterFromShape(*(myObject->getShapeHandler()), numericField.getFieldSelected(),
                                     numericField.getOutputName(), resolution, showInfo) )
    {
        addRasterObject(myProject.objectList.back());
        updateMaps();
    }
    else
    {
        myProject.logError("Error in rasterize.");
    }
}


gis::Crit3DRasterGrid* MainWindow::selectRaster(const QString &title, QString &rasterFileName, bool &isOk)
{
    isOk = false;

    // raster list
    QList<QString> rasterList;
    for (int i = 0; i < myProject.objectList.size(); i++)
    {
        if (myProject.objectList[i]->type == gisObjectRaster)
            rasterList.append(myProject.objectList[i]->fileName);
    }
    if (rasterList.size() == 0)
    {
        myProject.logWarning("No raster loaded.");
        return nullptr;
    }

    // select raster
    FormSelection rasterSelection(rasterList, title);
    if (rasterSelection.result() == QDialog::Rejected)
        return nullptr;

    rasterFileName = rasterSelection.getSelection();
    for (int i = 0; i < myProject.objectList.size(); i++)
    {
        if (myProject.objectList[i]->fileName == rasterFileName)
        {
            isOk = true;
            return myProject.objectList[i]->getRasterPointer();
        }
    }

    return nullptr;
}


void MainWindow::on_actionRasterize_with_base_triggered()
{
    // select shapefile
    int pos = getSelectedShapePos();
    if (pos == NODATA) return;
    GisObject* shapeObject = myProject.objectList.at(unsigned(pos));
    Crit3DShapeHandler* shapeHandler = shapeObject->getShapeHandler();

    // select raster
    QString rasterFileName;
    bool isOk;
    gis::Crit3DRasterGrid *refRaster = selectRaster("Select reference raster", rasterFileName, isOk);
    if (! isOk)
        return;

    // select shape field
    bool isOnlyNumeric = true;
    DialogSelectField numericField(shapeObject->getShapeHandler(), shapeObject->fileName, isOnlyNumeric, RASTERIZE_WITHBASE);

    if (numericField.result() != QDialog::Accepted)
        return;

    bool showInfo = true;
    if ( myProject.fillRasterFromShape(*shapeHandler, *refRaster, numericField.getFieldSelected(),
                                     numericField.getOutputName(), showInfo) )
    {
        addRasterObject(myProject.objectList.back());
        updateMaps();
    }
    else
    {
        myProject.logError("Error in rasterize.");
    }

}


void MainWindow::on_actionClipRaster_with_shape_triggered()
{
    myProject.logWarning("This feature is not yet available.");
}


void MainWindow::on_actionClipRaster_with_raster_triggered()
{
    // select raster
    QString refRasterFileName;
    bool isOk;
    gis::Crit3DRasterGrid *refRaster = selectRaster("Reference raster", refRasterFileName, isOk);
    if (! isOk) return;

    QString maskRasterFileName;
    gis::Crit3DRasterGrid *maskRaster = selectRaster("Mask raster", maskRasterFileName, isOk);
    if (! isOk) return;

    gis::Crit3DRasterGrid* outputRaster = new gis::Crit3DRasterGrid();
    if (! gis::clipRasterWithRaster(refRaster, maskRaster, outputRaster))
    {
        myProject.logError("Error in clipping.");
        return;
    }

    setDefaultScale(outputRaster->colorScale);
    myProject.addRaster(outputRaster, refRasterFileName + "_clip", myProject.gisSettings.utmZone);

    addRasterObject(myProject.objectList.back());
    updateMaps();
}

