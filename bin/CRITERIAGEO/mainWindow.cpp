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
#include "formText.h"
#include "gis.h"
#include "zonalStatistic.h"


#ifdef USE_GDAL
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
    setWindowTitle("CRITERIA GEO  " + QString(CRITERIA_GEO_VERSION));

    isDoubleClick = false;

    // initialize info dialog for shape
    shapeInfoDialog.setWindowTitle("Shape info");
    shapeInfoBrowser.setFixedSize(300, 300);
    QVBoxLayout shapeLayout;
    shapeLayout.addWidget(&shapeInfoBrowser);
    shapeInfoDialog.setLayout(&shapeLayout);
    shapeInfoDialog.setWindowFlag(Qt::WindowStaysOnTopHint);

    // Set the MapGraphics Scene and View
    mapScene = new MapGraphicsScene(this);
    mapView = new MapGraphicsView(mapScene, ui->widgetMap);

    // Set rubber band
    rubberBand = new RubberBand(QRubberBand::Rectangle, mapView);

    // Set tiles source
    setTileSource(WebTileSource::GOOGLE_Terrain);

    // Set start size and position
    startCenter = new Position (myProject.getGisSettings().startLocation.longitude,
                                     myProject.getGisSettings().startLocation.latitude, 0.0);
    mapView->setZoomLevel(8);
    mapView->centerOn(startCenter->lonLat());
    connect(this->mapView, SIGNAL(zoomLevelChanged(quint8)), this, SLOT(updateMaps()));
    connect(this->mapView, SIGNAL(mouseMoveSignal(QPoint)), this, SLOT(mouseMove(QPoint)));

    connect(ui->checkList, &QListWidget::itemClicked, [=](QListWidgetItem* item){ this->itemClicked(item); });
    ui->checkList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->checkList, &QListWidget::customContextMenuRequested, [=](const QPoint point){ this->itemMenuRequested(point); });

    setMouseTracking(true);
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

    updateMaps();
}


void MainWindow::updateMaps()
{
    for (auto* object : shapeObjList)
    {
        if (object != nullptr)
            object->updateCenter();
    }

    for (auto* object : rasterObjList)
    {
        if (object != nullptr)
            object->updateCenter();
    }
}


void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (isDoubleClick)
    {
        isDoubleClick = false;
        return;
    }

    QPoint eventPos = event->pos();

    if (event->button() == Qt::LeftButton)
    {
        if (selectShape(eventPos))
            updateMaps();
    }
    else if (event->button() == Qt::RightButton)
    {
        bool isContextMenu = true;

        if (rubberBand->isVisible())
        {
            getRubberBandRect(eventPos, rubberBandRect);
            rubberBand->hide();
            isContextMenu = (rubberBandRect.height() < 2);
        }

        if (isContextMenu)
            contextMenuRequested(event->pos());
    }
}


void MainWindow::mouseDoubleClickEvent(QMouseEvent * event)
{
    QPoint mapPos = getMapPos(event->pos());
    if (! isInsideMap(mapPos))
        return;

    isDoubleClick = true;

    Position newCenter = mapView->mapToScene(mapPos);
    ui->statusBar->showMessage("Set center: " + QString::number(newCenter.latitude()) + " " + QString::number(newCenter.longitude()));

    if (event->button() == Qt::LeftButton)
        mapView->zoomIn();
    else if (event->button() == Qt::RightButton)
        mapView->zoomOut();

    mapView->centerOn(newCenter.lonLat());
}


void MainWindow::mouseMove(QPoint eventPos)
{
    if (! isInsideMap(eventPos))
        return;

    // rubber band
    if (rubberBand != nullptr && rubberBand->isVisible())
    {
        QPoint widgetPos = eventPos + QPoint(MAPBORDER, MAPBORDER);
        rubberBand->setGeometry(QRect(rubberBand->getOrigin(), widgetPos).normalized());
        return;
    }

    Position geoPos = mapView->mapToScene(eventPos);

    int rasterIndex = getSelectedRasterPos(false);
    QString rasterValueStr = "";

    if (rasterIndex != NODATA && rasterIndex < myProject.objectList.size())
    {
        GisObject* myObject = myProject.objectList.at(rasterIndex);
        gis::Crit3DRasterGrid *myRaster = myObject->getRasterPointer();
        if (myRaster)
        {
            double utmX, utmY;
            gis::getUtmFromLatLon(myProject.getGisSettings(),
                                  geoPos.latitude(), geoPos.longitude(),
                                  &utmX, &utmY);

            float value = myRaster->getValueFromXY(utmX, utmY);

            if (! isEqual(value, myRaster->header->flag))
                rasterValueStr = QString::number(value);
        }
    }

    QString infoStr = QString("Lat:%1  Lon:%2")
                          .arg(geoPos.latitude(), 0, 'f', 6)
                          .arg(geoPos.longitude(), 0, 'f', 6);

    if (! rasterValueStr.isEmpty()) {
        infoStr += " Value: " + rasterValueStr;
    }

    ui->statusBar->showMessage(infoStr);
}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::RightButton)
        return;

    if (rubberBand)
    {
        QPoint mapPos = getMapPos(event->pos());
        QPoint widgetPos = mapPos + QPoint(MAPBORDER, MAPBORDER);

        // initialize Rect
        rubberBandRect = QRect();

        rubberBand->setOrigin(widgetPos);
        rubberBand->setGeometry(QRect(widgetPos, QSize()));
        rubberBand->show();

        event->accept();
    }
}


bool MainWindow::getRubberBandRect(const QPoint& position, QRect& rect)
{
    if (rubberBand == nullptr || !rubberBand->isVisible())
    {
        rect = QRect();
        return false;
    }

    QPoint p1 = rubberBand->getOrigin() - QPoint(MAPBORDER, MAPBORDER);
    QPoint p2 = getMapPos(position);

    rect = QRect(p1, p2).normalized();
    return true;
}


void MainWindow::contextMenuRequested(const QPoint &localPos)
{
    // Check mouse position
    const QPoint mapPos = getMapPos(localPos);
    if (! isInsideMap(mapPos))
        return;

    // Get selected item
    QListWidgetItem *itemSelected = ui->checkList->currentItem();
    if (itemSelected == nullptr)
        return;

    const int index = ui->checkList->row(itemSelected);
    if (index < 0 || index >= myProject.objectList.size())
        return;

    // Get selected object
    GisObject *myObject = myProject.objectList.at(index);
    if (myObject == nullptr)
        return;

    QMenu contextMenu(this);

    QAction *editShapeAction = nullptr;
    QAction *editRasterPointAction = nullptr;
    QAction *deleteRasterPointAction = nullptr;

    MapGraphicsShapeObject *shapeObject = nullptr;
    RasterUtmObject *rasterObject = nullptr;

    int row = NODATA;
    int col = NODATA;

    gis::Crit3DRasterGrid *rasterPointer = nullptr;

    // Get geographic position
    const Position geoPos = mapView->mapToScene(mapPos);

    gis::Crit3DGeoPoint geoPoint(geoPos.latitude(), geoPos.longitude());

    gis::Crit3DUtmPoint utmPoint;
    gis::getUtmFromLatLon(myProject.getGisSettings().utmZone, geoPoint, &utmPoint);

    switch (myObject->type)
    {
    case gisObjectShape:
    {
        shapeObject = getShapeObject(myObject);
        if (shapeObject == nullptr)
            return;

        editShapeAction = contextMenu.addAction("Edit Shape..");
        break;
    }

    case gisObjectRaster:
    {
        rasterObject = getRasterObject(myObject);
        if (rasterObject == nullptr)
            return;

        rasterPointer = rasterObject->getRasterPointer();
        if (rasterPointer == nullptr ||
            rasterPointer->header == nullptr)
        {
            return;
        }

        rasterPointer->getRowCol(
            utmPoint.x,
            utmPoint.y,
            row,
            col);

        if (rasterPointer->isOutOfGrid(row, col))
            return;

        editRasterPointAction =
            contextMenu.addAction("Edit raster point..");

        deleteRasterPointAction =
            contextMenu.addAction("Delete raster point");

        break;
    }

    default:
        return;
    }

    QAction *selectedAction =
        contextMenu.exec(mapToGlobal(localPos));

    if (selectedAction == nullptr)
        return;

    // ---------------------------------------------------------
    // Edit shape
    // ---------------------------------------------------------
    if (selectedAction == editShapeAction)
    {
        // TODO
        return;
    }

    // ---------------------------------------------------------
    // Edit raster point
    // ---------------------------------------------------------
    if (selectedAction == editRasterPointAction)
    {
        const float currentValue = rasterPointer->value[row][col];

        FormText f(
            "Choose new value:",
            QString::number(currentValue));

        if (f.exec() != QDialog::Accepted)
            return;

        const QString newValueStr = f.getText();

        bool isOk = false;
        const float newValue = newValueStr.toFloat(&isOk);

        if (!isOk)
        {
            myProject.logWarning("Wrong number!");
            return;
        }

        rasterPointer->value[row][col] = newValue;

        updateMaps();
        return;
    }

    // ---------------------------------------------------------
    // Delete raster point
    // ---------------------------------------------------------
    if (selectedAction == deleteRasterPointAction)
    {
        const QMessageBox::StandardButton confirm =
            QMessageBox::question(
                this,
                "Warning",
                "Are you sure?",
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);

        if (confirm != QMessageBox::Yes)
            return;

        rasterPointer->value[row][col] =
            rasterPointer->header->flag;

        updateMaps();
        return;
    }
}


void MainWindow::itemMenuRequested(const QPoint &point)
{
    QPoint itemPoint = ui->checkList->mapToGlobal(point);
    QListWidgetItem* item = ui->checkList->itemAt(point);

    int currentPosition = ui->checkList->row(item);
    if (currentPosition < 0)
        return;

    GisObject* myObject = myProject.objectList.at(unsigned(currentPosition));

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
                submenu.addAction("Save as");
                submenu.addSeparator();
            }

            submenu.addAction("move up ↑");
            submenu.addAction("move down ↓");
            submenu.addAction("Zoom to layer");
            submenu.addSeparator();

            submenu.addAction("Show data");
            submenu.addAction("Attribute table");
            submenu.addSeparator();

            submenu.addAction("Set style");
            submenu.addAction("Set default scale");
            submenu.addAction("Set grayscale");
            submenu.addAction("Set random colors");
            submenu.addAction("Reverse color scale");
            submenu.addAction("Disable color scale");
            submenu.addSeparator();

            if (myShapeObject->isSelectedRed())
                submenu.addAction("Selected border black");
            else
                submenu.addAction("Selected border red");

            submenu.addAction("Set opacity");

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
            submenu.addAction("Save as");
            submenu.addSeparator();

            submenu.addAction("move up ↑");
            submenu.addAction("move down ↓");
            submenu.addAction("Zoom to layer");
            submenu.addSeparator();

            submenu.addAction("Set default scale");
            submenu.addAction("Set DTM scale");
            submenu.addAction("Set grayscale");
            submenu.addAction("Set random colors");
            submenu.addAction("Reverse color scale");

            submenu.addSeparator();

            if (myRasterObject->getRasterPointer()->colorScale->isFixedRange())
                submenu.addAction("Set variable range");
            else
                submenu.addAction("Set fixed range");

            submenu.addAction("Set opacity");

            submenu.addSeparator();

            submenu.addAction("Statistical summary");
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
        if (rightClickItem->text() == "move up ↑" )
        {
            moveObject(currentPosition, UP);
        }
        else if (rightClickItem->text() == "move down ↓" )
        {
            moveObject(currentPosition, DOWN);
        }
        else if (rightClickItem->text() == "Remove" )
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
            myProject.objectList.erase(myProject.objectList.begin() + currentPosition);

            ui->checkList->takeItem(ui->checkList->indexAt(point).row());
        }
        else if (rightClickItem->text() == "Close Project" )
        {
            on_actionClose_Project_triggered();
        }
        else if (rightClickItem->text().contains("Zoom to layer"))
        {
            if (myObject->type == gisObjectRaster || myObject->type == gisObjectNetcdf)
            {
                zoomToRaster(myObject);
            }
            else if (myObject->type == gisObjectShape)
            {
                zoomToShape(myObject);
            }
        }
        else if (rightClickItem->text().contains("Show data"))
        {
            DialogShapeProperties showData(myObject->getShapeHandler(), myObject->fileName);
        }
        else if (rightClickItem->text().contains("Attribute table"))
        {
            DialogDbfTable dbfTable(myObject->getShapeHandler(), myObject->fileName);
            dbfTable.exec();
        }
        else if (rightClickItem->text().contains("Set style"))
        {
            setShapeStyle_GUI(myObject);
            if (myShapeObject)
                emit myShapeObject->redrawRequested();
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
            if (myObject->type == gisObjectRaster && myRasterObject != nullptr)
            {
                setGrayScale(myObject->getRasterPointer()->colorScale);
                emit myRasterObject->redrawRequested();
            }
            if (myObject->type == gisObjectShape && myShapeObject != nullptr)
            {
                setGrayScale(myShapeObject->colorScale);
                emit myShapeObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Set default scale"))
        {
            if (myObject->type == gisObjectRaster && myRasterObject != nullptr)
            {
                setDefaultScale(myObject->getRasterPointer()->colorScale);
                emit myRasterObject->redrawRequested();
            }
            if (myObject->type == gisObjectShape && myShapeObject != nullptr)
            {
                setDefaultScale(myShapeObject->colorScale);
                emit myShapeObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Set random colors"))
        {
            if (myObject->type == gisObjectRaster && myRasterObject != nullptr)
            {
                setRandomColors(myObject->getRasterPointer()->colorScale);
                emit myRasterObject->redrawRequested();
            }
            if (myObject->type == gisObjectShape && myShapeObject != nullptr)
            {
                setRandomColors(myShapeObject->colorScale);
                emit myShapeObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Set DTM scale"))
        {
            if (myObject->type == gisObjectRaster && myRasterObject != nullptr)
            {
                setDTMScale(myObject->getRasterPointer()->colorScale);
                emit myRasterObject->redrawRequested();
            }
            if (myObject->type == gisObjectShape && myShapeObject != nullptr)
            {
                setDTMScale(myShapeObject->colorScale);
                emit myShapeObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Reverse color scale"))
        {
            if (myObject->type == gisObjectRaster && myRasterObject != nullptr)
            {
                reverseColorScale(myObject->getRasterPointer()->colorScale);
                emit myRasterObject->redrawRequested();
            }
            else if (myObject->type == gisObjectShape && myShapeObject != nullptr)
            {
                reverseColorScale(myShapeObject->colorScale);
                emit myShapeObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Disable color scale"))
        {
            if (myObject->type == gisObjectShape && myShapeObject != nullptr)
            {
                myShapeObject->setFill(false);
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
        else if (rightClickItem->text().contains("Set fixed range"))
        {
            if (myObject->type == gisObjectRaster && myRasterObject != nullptr)
            {
                myRasterObject->getRasterPointer()->colorScale->setFixedRange(true);
                emit myRasterObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Set variable range"))
        {
            if (myObject->type == gisObjectRaster && myRasterObject != nullptr)
            {
                myRasterObject->getRasterPointer()->colorScale->setFixedRange(false);
                emit myRasterObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Set opacity"))
        {
            float currentOpacity = 0.5f;

            if (myObject->type == gisObjectRaster && myRasterObject != nullptr)
                currentOpacity = myRasterObject->opacity();
            else if (myObject->type == gisObjectShape && myShapeObject != nullptr)
                currentOpacity = myShapeObject->opacity();
            else
                return;

            FormText f("Choose opacity:", QString::number(currentOpacity));
            if (f.exec() != QDialog::Accepted)
                return;

            bool isOk = false;
            const float opacity = f.getText().toFloat(&isOk);

            if (!isOk || (opacity < 0.0f) || opacity > 1.0f)
            {
                myProject.logWarning("Wrong number! (it must be in 0-1)");
                return;
            }

            if (myObject->type == gisObjectRaster)
            {
                myRasterObject->setOpacity(opacity);
                emit myRasterObject->redrawRequested();
            }
            else // gisObjectShape
            {
                myShapeObject->setOpacity(opacity);
                emit myShapeObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Selected border black"))
        {
            if (myObject->type == gisObjectShape && myShapeObject != nullptr)
            {
                myShapeObject->setSelectedRed(false);
                emit myShapeObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Selected border red"))
        {
            if (myObject->type == gisObjectShape && myShapeObject != nullptr)
            {
                myShapeObject->setSelectedRed(true);
                emit myShapeObject->redrawRequested();
            }
        }
    }
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
    return ( pos.x() > 0 && pos.x() < (mapView->width() - MAPBORDER*2)
            && pos.y() > 0 && pos.y() < (mapView->height() - MAPBORDER*2) );
}


void MainWindow::addRasterObject(GisObject* myObject)
{
    if (myObject == nullptr)
        return;

    // Create raster object
    auto* rasterObject = new RasterUtmObject(mapView);
    rasterObject->setOpacity(0.66);
    if (! rasterObject->initialize(myObject->getRasterPointer(), myObject->gisSettings))
    {
        delete rasterObject;
        return;
    }

    // Set Z order before adding the object to the scene
    rasterObject->setZValue(qreal(myProject.lastZOrder++));

    // Store object
    rasterObjList.push_back(rasterObject);

    // Add object to scene
    mapView->scene()->addObject(rasterObject);

    // Add item to GUI
    auto* item = new QListWidgetItem("[RASTER] " + myObject->fileName);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    ui->checkList->addItem(item);

    updateMaps();
}


bool MainWindow::moveObject(int index, int direction)
{
    if (myProject.objectList.size() != ui->checkList->count())
    {
        myProject.logWarning(
            "Unable to move the item.\n"
            "The number of objects in memory differs from the number of items.");
        return false;
    }

    const int objectCount = static_cast<int>(myProject.objectList.size());

    if (index < 0 || index >= objectCount)
        return false;

    int newIndex;

    if (direction == UP)
        newIndex = index - 1;
    else if (direction == DOWN)
        newIndex = index + 1;
    else
    {
        myProject.logWarning("Wrong direction.");
        return false;
    }

    if (newIndex < 0 || newIndex >= objectCount)
        return false;

    // get objects before changing the list
    GisObject* obj1 = myProject.objectList[index];
    GisObject* obj2 = myProject.objectList[newIndex];

    if (obj1 == nullptr || obj2 == nullptr)
    {
        myProject.logWarning("Unable to move the item.");
        return false;
    }

    // remove item from the list widget
    QListWidgetItem* item = ui->checkList->takeItem(index);

    if (item == nullptr)
    {
        myProject.logWarning("Unable to move the item.");
        return false;
    }

    // swap objects in the project list
    std::swap(myProject.objectList[index], myProject.objectList[newIndex]);

    // swap drawing order
    swapZValue(obj1, obj2);

    // move item in the UI
    ui->checkList->insertItem(newIndex, item);
    ui->checkList->setCurrentRow(newIndex);

    return true;
}


/*
void MainWindow::addNetcdfObject(GisObject* myObject)
{
    QListWidgetItem* item = new QListWidgetItem("[NETCDF] " + myObject->fileName);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    ui->checkList->addItem(item);

    RasterUtmObject* netcdfObj = new RasterUtmObject(mapView);
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

    rasterObjList.push_back(netcdfObj);

    mapView->scene()->addObject(netcdfObj);
    updateMaps();
}
*/


bool MainWindow::addShapeObject(GisObject* myObject)
{
    if (myObject == nullptr)
        return false;

    // Check UTM zone
    const int zoneNumber = myObject->getShapeHandler()->getUtmZone();
    if ((zoneNumber < 1) || (zoneNumber > 60))
    {
        QMessageBox::critical(nullptr, "ERROR!", "Wrong UTM zone.");
        return false;
    }

    // Set name
    const QString itemName = !myObject->projectName.isEmpty()
                                 ? "[PROJECT] " + myObject->projectName
                                 : "[SHAPE] " + myObject->fileName;

    // Create shape object
    auto* shapeObject = new MapGraphicsShapeObject(mapView);

    if (! shapeObject->initializeUTM(myObject->getShapeHandler()))
    {
        delete shapeObject;
        return false;
    }

    shapeObject->setOpacity(0.66);

    // Set Z order
    shapeObject->setZValue(qreal(myProject.lastZOrder++));

    // Store object
    shapeObjList.push_back(shapeObject);

    // Add object to scene
    mapView->scene()->addObject(shapeObject);

    // Add item to GUI
    auto* item = new QListWidgetItem(itemName);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    ui->checkList->addItem(item);

    updateMaps();

    return true;
}


void MainWindow::zoomToShape(GisObject* myObject)
{
    if (! myObject)
        return;

    Crit3DShapeHandler* shapeHandler = myObject->getShapeHandler();
    if (! shapeHandler)
        return;

    gis::Crit3DUtmPoint p0, p1;
    if (! shapeHandler->getBounds(p0.x, p0.y, p1.x, p1.y))
        return;

    gis::Crit3DGeoPoint g0, g1, center;
    gis::getLatLonFromUtm(myProject.getGisSettings(), p0, g0);
    gis::getLatLonFromUtm(myProject.getGisSettings(), p1, g1);

    center.latitude = (g0.latitude + g1.latitude) * 0.5;
    center.longitude = (g0.longitude + g1.longitude) * 0.5;

    double latSize = std::abs(g0.latitude - g1.latitude);
    const double cosLat = std::max(0.01, std::cos(center.latitude * DEG_TO_RAD));
    double lonSize = std::abs(g0.longitude - g1.longitude) * cosLat;

    // padding and avoid division by zero
    latSize = std::max(latSize * 1.2, 1e-6);
    lonSize = std::max(lonSize * 1.2, 1e-6);

    const double viewW = double(ui->widgetMap->width());
    const double viewH = double(ui->widgetMap->height());
    const double scaleX = viewW / lonSize;
    const double scaleY = viewH / latSize;

    double zoom = std::log2(std::min(scaleX, scaleY));
    zoom = std::clamp(zoom, 1.0, 20.0);

    mapView->setZoomLevel(static_cast<quint8>(std::round(zoom)));
    mapView->centerOn(center.longitude, center.latitude);

    updateMaps();
}


void MainWindow::zoomToRaster(GisObject* myObject)
{
    if (! myObject)
        return;

    RasterUtmObject* rasterObj = getRasterObject(myObject);
    if (! rasterObj)
        return;

    const Position center = rasterObj->getRasterCenter();

    // degrees (padding)
    const double lonSize = std::max(rasterObj->getSizeX() * 1.1, 1e-6);
    const double latSize = std::max(rasterObj->getSizeY() * 1.1, 1e-6);

    const double viewW = double(ui->widgetMap->width());
    const double viewH = double(ui->widgetMap->height());

    const double scaleX = viewW / lonSize;
    const double scaleY = viewH / latSize;

    double zoom = std::log2(std::min(scaleX, scaleY));
    zoom = std::clamp(zoom, 1.0, 20.0);

    mapView->setZoomLevel(static_cast<quint8>(std::round(zoom)));
    mapView->centerOn(center.longitude(), center.latitude());

    updateMaps();
}


void MainWindow::on_actionLoadRaster_triggered()
{
    #ifdef USE_GDAL
        QStringList rasterFormats = getGdalRasterReadExtension();

        rasterFormats.sort();
        rasterFormats.insert(0, tr("all files (*.*)"));
        rasterFormats.insert(1, tr("ESRI float (*.flt)"));
        rasterFormats.insert(1, tr("ESRI bil (*.bil)"));
        rasterFormats.insert(2, tr("ENVI IMG (*.img)"));

        QString fileNameWithPath = QFileDialog::getOpenFileName(this, tr("Open raster file"), "", rasterFormats.join(";;"));
    #else
         QString fileNameWithPath = QFileDialog::getOpenFileName(this, tr("Open raster file"), "",
                                                            tr("ESRI FLT (*.flt);;"ESRI BIL (*.bil);;ENVI IMG (*.img)"));
    #endif

    if (fileNameWithPath == "") return;

    FormInfo formInfo;
    formInfo.start("Load raster...", 0);

    if (! myProject.loadRaster(fileNameWithPath))
    {
        myProject.logError();
        return;
    }

    formInfo.close();

    addRasterObject(myProject.objectList.back());
    zoomToRaster(myProject.objectList.back());
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
    {
        myProject.logError();
        return;
    }

    GisObject* myObject = myProject.objectList.back();

    this->addShapeObject(myObject);
}


int MainWindow::getShapeIndex(GisObject* myObject)
{
    if (myObject == nullptr || myObject->type != gisObjectShape)
        return NODATA;

    const Crit3DShapeHandler* shapeHandler = myObject->getShapeHandler();

    for (size_t i = 0; i < shapeObjList.size(); ++i)
    {
        if (shapeObjList[i]->getShapePointer() == shapeHandler)
            return static_cast<int>(i);
    }

    return NODATA;
}


int MainWindow::getRasterIndex(GisObject* myObject)
{
    if (myObject == nullptr || myObject->type != gisObjectRaster)
        return NODATA;

    const gis::Crit3DRasterGrid* rasterPointer = myObject->getRasterPointer();

    for (size_t i = 0; i < rasterObjList.size(); ++i)
    {
        if (rasterObjList[i]->getRasterPointer() == rasterPointer)
            return static_cast<int>(i);
    }

    return NODATA;
}


int MainWindow::getObjectIndex(GisObject * obj)
{
    if (obj == nullptr)
        return NODATA;

    switch (obj->type)
    {
    case gisObjectRaster:
        return getRasterIndex(obj);

    case gisObjectShape:
        return getShapeIndex(obj);

    default:
        return NODATA;
    }
}


qreal MainWindow::getZValue(GisObject* obj)
{
    if (obj == nullptr)
        return NODATA;

    const int index = getObjectIndex(obj);

    if (index == NODATA)
        return NODATA;

    switch (obj->type)
    {
    case gisObjectRaster:
        return rasterObjList[index]->zValue();

    case gisObjectShape:
        return shapeObjList[index]->zValue();

    default:
        return NODATA;
    }
}

bool MainWindow::setZValue(GisObject* obj, qreal zValue)
{
    if (obj == nullptr)
        return false;

    const int index = getObjectIndex(obj);

    if (index == NODATA)
        return false;

    switch (obj->type)
    {
    case gisObjectRaster:
        rasterObjList[index]->setZValue(zValue);
        return true;

    case gisObjectShape:
        shapeObjList[index]->setZValue(zValue);
        return true;

    default:
        return false;
    }
}


bool MainWindow::swapZValue(GisObject* obj1, GisObject* obj2)
{
    if (obj1 == nullptr || obj2 == nullptr || obj1 == obj2)
        return false;

    const qreal z1 = getZValue(obj1);
    const qreal z2 = getZValue(obj2);

    if (z1 == NODATA || z2 == NODATA)
        return false;

    return setZValue(obj1, z2) && setZValue(obj2, z1);
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

    QString fieldName = numericField.getFieldSelected();
    double cellSize = numericField.getCellSizeValue();
    if (cellSize <= 0 || cellSize == NODATA)
    {
        QMessageBox::information(nullptr, "Wrong cellSize", "Insert a positive cellsize.");
        return false;
    }

    QString outputFileName = QFileDialog::getSaveFileName(this, tr("Save NetCDF as"), "", tr("NetCDF files (*.nc)"));
    if (outputFileName == "")
    {
        QMessageBox::information(nullptr, "Insert output name", "missing NetCDF filename");
        return false;
    }

    FormInfo formInfo;
    formInfo.start("Export to NetCDF...", 0);

    std::string variableName = fieldName.toStdString();     // TODO
    std::string unit = "";                                  // TODO
    bool isOK = myProject.output.convertShapeToNetcdf(*(myObject->getShapeHandler()), outputFileName.toStdString(),
                                                      fieldName.toStdString(), variableName, unit, cellSize, NO_DATE, 0);
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
#ifdef USE_GDAL
    DialogSelectField shapeFieldDialog(myObject->getShapeHandler(), myObject->fileName, true, GDALRASTER);

    if (shapeFieldDialog.result() != QDialog::Accepted)
        return false;

    QString fieldName = shapeFieldDialog.getFieldSelected();
    std::string shapeFilePath = (myObject->getShapeHandler())->getFilepath();
    QString shapeFileName = QString::fromStdString(shapeFilePath);

    double cellSize = shapeFieldDialog.getCellSizeValue();
    if (cellSize <= 0 || cellSize == NODATA)
    {
        QMessageBox::information(nullptr, "Wrong cellSize", "Insert a positive value.");
        return false;
    }

    QString resolutionStr = QString::number(cellSize);

    QStringList gdalExt = getGdalRasterWriteExtension();
    QString outputName = QFileDialog::getSaveFileName(this, tr("Save raster as"), "", gdalExt.join(";\n"));
    if (outputName.isEmpty())
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

    if (! myProject.loadRaster(outputName))
    {
        myProject.logError();
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


bool MainWindow::selectShape(const QPoint &screenPos)
{
    // Check if there is an item selected
    QListWidgetItem *itemSelected = ui->checkList->currentItem();
    if (itemSelected == nullptr)
        return false;

    // Check if the selected element is a shape
    const int row = ui->checkList->row(itemSelected);
    if (row < 0 || row >= myProject.objectList.size())
        return false;

    GisObject *gisObject = myProject.objectList.at(row);
    if (gisObject == nullptr || gisObject->type != gisObjectShape)
        return false;

    // Check shape object
    MapGraphicsShapeObject *shapeObject = getShapeObject(gisObject);
    if (shapeObject == nullptr)
        return false;

    // Check position
    const QPoint mapPos = getMapPos(screenPos);
    if (! isInsideMap(mapPos))
        return false;

    // Convert screen position to UTM coordinates
    const Position geoPos = mapView->mapToScene(mapPos);

    double x = 0.0;
    double y = 0.0;

    gis::latLonToUtmForceZone(myProject.getGisSettings().utmZone,
                              geoPos.latitude(), geoPos.longitude(), &x, &y);

    // Find the shape containing the point
    Crit3DShapeHandler *shapeHandler = gisObject->getShapeHandler();
    if (shapeHandler == nullptr)
        return false;

    const int index = shapeHandler->getShapeIndexfromPoint(x, y);

    // Update only if the selected shape has changed
    if (index == shapeObject->getSelected())
        return true;

    shapeObject->setSelected(index);
    emit shapeObject->redrawRequested();

    // Update shape information dialog
    if (index == NODATA)
    {
        shapeInfoDialog.close();
    }
    else
    {
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
    if (ucmDialog.result() == QDialog::Rejected)
        return;

    QString ucmFileName = QFileDialog::getSaveFileName(this, tr("Save Shapefile"), "", tr("shp files (*.shp)"));
    if (ucmFileName.isEmpty())
        return;

    bool isPrevailing = true;
    double threshold = 0.5;         // TODO aggiungere a ucmDialog

    if (! myProject.computeUnitCropMap(ucmDialog.getCrop(), ucmDialog.getSoil(), ucmDialog.getMeteo(),
                                        ucmDialog.getIdCrop().toStdString(), ucmDialog.getIdSoil().toStdString(),
                                        ucmDialog.getIdMeteo().toStdString(), ucmFileName,
                                        ucmDialog.getCellSize(), threshold, isPrevailing))
    {
        myProject.logError();
        return;
    }

    addShapeObject(myProject.objectList.back());
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
    if (ucmDialog.result() == QDialog::Rejected)
        return;

    QString ucmFileName = QFileDialog::getSaveFileName(this, tr("Save Shapefile"), "", tr("shp files (*.shp)"));
    if (ucmFileName.isEmpty())
        return;

    bool isPrevailing = false;
    double threshold = 0.5;

    if (! myProject.computeUnitCropMap(ucmDialog.getCrop(), ucmDialog.getSoil(), ucmDialog.getMeteo(),
                                        ucmDialog.getIdCrop().toStdString(), ucmDialog.getIdSoil().toStdString(),
                                        ucmDialog.getIdMeteo().toStdString(), ucmFileName,
                                        NODATA, threshold, isPrevailing))
    {
        myProject.logError();
        return;
    }

    addShapeObject(myProject.objectList.back());
}


void MainWindow::on_actionExtract_Unit_Crop_Map_list_triggered()
{
    QListWidgetItem * itemSelected = ui->checkList->currentItem();
    if (itemSelected == nullptr || !itemSelected->text().contains("SHAPE"))
    {
        QMessageBox::warning(nullptr, "No shape selected", "Select Computational Units Map shapefile.");
        return;
    }

    int pos = ui->checkList->row(itemSelected);
    GisObject* myObject = myProject.objectList.at(unsigned(pos));
    Crit3DShapeHandler* shapeHandler = myObject->getShapeHandler();

    if (! myProject.extractUcmListToDb(shapeHandler, true))
        myProject.logError();
}



void MainWindow::on_actionAssign_IDCase_triggered()
{
    QListWidgetItem * itemSelected = ui->checkList->currentItem();
    if (itemSelected == nullptr || !itemSelected->text().contains("SHAPE"))
    {
        QMessageBox::warning(nullptr, "No UCM selected", "Select Computational Units Map shapefile.");
        return;
    }

    int pos = ui->checkList->row(itemSelected);
    GisObject* myObject = myProject.objectList.at(unsigned(pos));
    Crit3DShapeHandler* shapeHandler = myObject->getShapeHandler();

    if (! myProject.assignIdCase(shapeHandler))
    {
        myProject.logError();
        return;
    }

    myProject.logInfo("ID_CASE has been assigned.");
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

    if (! myProject.createShapeFromCsv(shapeIndex, fileCsv, fileCsvFormat, outputFileName))
    {
        myProject.logError();
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
        else
            return;
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
    if (! myProject.output.isProjectLoaded )
        return;

    QMessageBox::StandardButton confirm;
    QString msg = "This operation will close the project: " + myProject.output.projectName + "\nAre you sure?";

    confirm = QMessageBox::question(nullptr, "Warning", msg, QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if (confirm == QMessageBox::No)
        return;

    closeGeoProject();

    // disable output map action
    ui->actionOutput_Map->setEnabled(false);
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
        if (isInfo) {
            QMessageBox::warning(nullptr, "No raster loaded", "Load a raster before.");
        }
        return NODATA;
    }

    QListWidgetItem * itemSelected = ui->checkList->currentItem();

    if (itemSelected == nullptr || !itemSelected->text().contains("RASTER"))
    {
        if (isInfo) {
            QMessageBox::warning(nullptr, "No raster selected", "Select a raster before.");
        }
        return NODATA;
    }

    return ui->checkList->row(itemSelected);
}


int MainWindow::getSelectedShapePos()
{
    if (shapeObjList.empty())
    {
        QMessageBox::warning(nullptr, "No shape loaded", "Load a shapefile before.");
        return NODATA;
    }

    QListWidgetItem * itemSelected = ui->checkList->currentItem();

    if (itemSelected == nullptr || ! itemSelected->text().contains("SHAPE"))
    {
        QMessageBox::warning(nullptr, "No shape selected", "Select a shapefile before.");
        return NODATA;
    }

    return ui->checkList->row(itemSelected);
}


void MainWindow::on_actionRasterizeShape_whole_shapefile_triggered()
{
    int pos = getSelectedShapePos();
    if (pos == NODATA)
        return;

    GisObject* myObject = myProject.objectList.at(unsigned(pos));
    if (! myObject)
        return;

    const bool isOnlyNumeric = true;
    DialogSelectField numericField(myObject->getShapeHandler(), myObject->fileName, isOnlyNumeric, RASTERIZE);
    if (numericField.result() != QDialog::Accepted)
        return;

    const QString fieldName = numericField.getFieldSelected();
    if (fieldName.isEmpty())
    {
        myProject.logError("Missing field name");
        return;
    }

    double cellSize = numericField.getCellSizeValue();
    if (cellSize <= 0 || isEqual(cellSize, NODATA))
    {
        myProject.logError("Insert a positive cellSize.");
        return;
    }

    double threshold = numericField.getThresholdValue();
    if (threshold <= 0 || threshold > 1)
    {
        myProject.logError("Insert a coverage threshold in (0,1].");
        return;
    }

    QString outputName = numericField.getStringValue();
    if (outputName.isEmpty())
    {
        myProject.logError("Missing output name");
        return;
    }

    Crit3DShapeHandler* shapeHandler = myObject->getShapeHandler();
    if (! shapeHandler)
    {
        myProject.logError("Wrong shapefile");
        return;
    }

    size_t oldObjectSize = myProject.objectList.size();

    const bool showInfo = true;
    if (! myProject.newRasterFromShape(*shapeHandler, fieldName, outputName,
                                      cellSize, threshold, showInfo) )
    {
        myProject.logError("Error in rasterize shape.");
        return;
    }

    if (myProject.objectList.size() <= oldObjectSize)
    {
        myProject.logError("Raster created but not added.");
        return;
    }

    addRasterObject(myProject.objectList.back());
    updateMaps();
}


gis::Crit3DRasterGrid* MainWindow::selectRaster(const QString &title, QString &rasterFileName, bool &isOk)
{
    isOk = false;

    // raster list
    QList<QString> rasterList;
    for (std::size_t i = 0; i < myProject.objectList.size(); i++)
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
    for (std::size_t i = 0; i < myProject.objectList.size(); i++)
    {
        if (myProject.objectList[i]->fileName == rasterFileName)
        {
            isOk = true;
            return myProject.objectList[i]->getRasterPointer();
        }
    }

    return nullptr;
}


void MainWindow::on_actionRasterizeShape_with_raster_mask_triggered()
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
    QString outputName = numericField.getStringValue();
    if (outputName.isEmpty())
    {
        myProject.logError("Missing output name");
        return;
    }

    if (! myProject.fillRasterFromShape(*shapeHandler, *refRaster, numericField.getFieldSelected(), outputName, showInfo))
        myProject.logError("Error in fillRasterFromShape");

    addRasterObject(myProject.objectList.back());
    updateMaps();
}


void MainWindow::on_actionAssign_shape_prevailing_value_raster_triggered()
{
    // select shapefile
    int pos = getSelectedShapePos();
    if (pos == NODATA)
        return;

    GisObject* shapeObject = myProject.objectList.at(unsigned(pos));
    Crit3DShapeHandler* shapeHandler = shapeObject->getShapeHandler();

    // select raster
    QString rasterFileName;
    bool isOk;
    gis::Crit3DRasterGrid *rasterPtr = selectRaster("Select the value raster", rasterFileName, isOk);
    if (! isOk)
        return;

    // select shape field
    const bool isOnlyNumeric = true;
    DialogSelectField numericField(shapeHandler, shapeObject->fileName, isOnlyNumeric, PREVAILING);

    if (numericField.result() != QDialog::Accepted)
        return;

    QString fieldName = numericField.getStringValue();
    if (fieldName.isEmpty())
        fieldName = numericField.getFieldSelected();

    const double threshold = numericField.getThresholdValue();
    if ((threshold < 0) || (threshold > 1.0) || (threshold == NODATA))
    {
        QMessageBox::information(nullptr, "Wrong threshold", "Insert a value in [0,1]");
        return;
    }

    const bool isProportional = numericField.isChecked();

    FormInfo formInfo;

    formInfo.start("Open shapefile read/write..", 0);
    shapeHandler->open(shapeHandler->getFilepath(), true);

    formInfo.setText("Compute prevailing...");

    std::vector<int> categories, vectorNull;
    std::vector <std::vector<int>> matrix = computeMatrixAnalysisRaster(*shapeHandler, *rasterPtr, categories, vectorNull);

    std::string errorStr;
    if (isProportional)
        isOk = zonalStatisticsShapeCategories_proportional(*shapeHandler, categories, matrix,
                                                                   rasterPtr->header->cellSize,
                                                                   fieldName.toStdString(), threshold, errorStr);
    else
        isOk = zonalStatisticsShapeCategories_majority(*shapeHandler, categories, matrix,
                                                      rasterPtr->header->cellSize,
                                                      fieldName.toStdString(), threshold, errorStr);
    formInfo.close();

    if (! isOk)
        myProject.logError(QString::fromStdString(errorStr));
}


void MainWindow::on_actionClipRaster_with_raster_mask_triggered()
{
    bool isOk;

    QString refRasterFileName;
    gis::Crit3DRasterGrid *refRaster = selectRaster("Reference raster", refRasterFileName, isOk);
    if (! isOk)
        return;

    QString maskRasterFileName;
    gis::Crit3DRasterGrid *maskRaster = selectRaster("Mask raster", maskRasterFileName, isOk);
    if (! isOk)
        return;

    gis::Crit3DRasterGrid* outputRaster = new gis::Crit3DRasterGrid();
    if (! gis::clipRasterWithRaster(refRaster, maskRaster, outputRaster))
    {
        myProject.logError("Error in clipping.");
        return;
    }

    setDefaultScale(outputRaster->colorScale);
    myProject.addRaster(outputRaster, refRasterFileName + "_clip", myProject.getGisSettings().utmZone);

    addRasterObject(myProject.objectList.back());
    updateMaps();
}


void MainWindow::on_actionClipRaster_cut_null_values_triggered()
{
    bool isOk;

    QString rasterFileName;
    gis::Crit3DRasterGrid *inputRaster = selectRaster("Select raster to cut", rasterFileName, isOk);
    if (! isOk)
        return;

    gis::Crit3DRasterGrid* outputRaster = new gis::Crit3DRasterGrid();
    std::string errorStr;
    if (! gis::resizeRasterCutEmptyFrame(inputRaster, outputRaster, errorStr))
    {
        myProject.logWarning(QString::fromStdString(errorStr));
        return;
    }

    setDefaultScale(outputRaster->colorScale);
    myProject.addRaster(outputRaster, "Cut_Null_" + rasterFileName, myProject.getGisSettings().utmZone);

    addRasterObject(myProject.objectList.back());
    updateMaps();
}


void MainWindow::on_actionReplace_values_with_raster_mask_triggered()
{
    bool isOk;

    QString refRasterFileName;
    gis::Crit3DRasterGrid *refRaster = selectRaster("Reference raster", refRasterFileName, isOk);
    if (! isOk) return;

    QString maskRasterFileName;
    gis::Crit3DRasterGrid *maskRaster = selectRaster("Mask raster", maskRasterFileName, isOk);
    if (! isOk) return;

    FormInfo formInfo;
    formInfo.start("Clip raster...", 0);
    {
        gis::Crit3DRasterGrid* outputRaster = new gis::Crit3DRasterGrid();
        if (! gis::replaceRasterValues(refRaster, maskRaster, outputRaster))
        {
            myProject.logError("Error in clipping.");
            formInfo.close();
            return;
        }

        setDefaultScale(outputRaster->colorScale);
        myProject.addRaster(outputRaster, refRasterFileName + "_clip", myProject.getGisSettings().utmZone);
    }
    formInfo.close();

    addRasterObject(myProject.objectList.back());
    updateMaps();
}


void MainWindow::on_actionDelete_a_range_of_values_raster_triggered()
{
    QString refRasterFileName;
    bool isOk;
    gis::Crit3DRasterGrid *refRaster = selectRaster("Select raster to delete a range of values", refRasterFileName, isOk);
    if (! isOk)
        return;

    FormText formMinimum("Choose minimum value:", QString::number(refRaster->minimum));
    if (formMinimum.exec() != QDialog::Accepted)
        return;

    QString minStr = formMinimum.getText();
    float minimum = minStr.toFloat(&isOk);
    if (! isOk)
    {
        myProject.logWarning("Wrong number!");
        return;
    }
    formMinimum.close();

    FormText formMaximum("Choose maximum value:", QString::number(refRaster->maximum));
    if (formMaximum.exec() != QDialog::Accepted)
    {
        return;                         // Cancel or close
    }

    QString maxStr = formMaximum.getText();
    float maximum = maxStr.toFloat(&isOk);
    if (! isOk)
    {
        myProject.logWarning("Wrong number!");
        return;
    }
    formMaximum.close();

    gis::Crit3DRasterGrid* outputRaster = new gis::Crit3DRasterGrid();
    if (! gis::deleteRangeOfValuesRaster(refRaster, minimum, maximum, outputRaster))
        return;

    setDTMScale(outputRaster->colorScale);
    myProject.addRaster(outputRaster, refRasterFileName + "_range_deleted", myProject.getGisSettings().utmZone);

    addRasterObject(myProject.objectList.back());
    updateMaps();
}


void MainWindow::on_actionClipRaster_via_bounding_box_triggered()
{       
    QString refRasterFileName;
    bool isOk = false;
    gis::Crit3DRasterGrid *refRaster = selectRaster("Select raster to crop", refRasterFileName, isOk);

    if (!isOk || refRaster == nullptr)
        return;

    myProject.logInfo("Select the bounding box (use the right mouse button)");

    rubberBandRect.setSize(QSize(0, 0));

    // Wait for the user to define the bounding box.
    while (rubberBandRect.width() < 2 || rubberBandRect.height() < 2)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    }

    const QRect normalizedRect = rubberBandRect.normalized();

    const Position topLeft = mapView->mapToScene(normalizedRect.topLeft());
    const Position bottomRight = mapView->mapToScene(normalizedRect.bottomRight());

    const gis::Crit3DGeoPoint p1(topLeft.latitude(), topLeft.longitude());
    const gis::Crit3DGeoPoint p2(bottomRight.latitude(), bottomRight.longitude());

    gis::Crit3DRasterGrid* outputRaster = new gis::Crit3DRasterGrid();
    if (! gis::cropRaster(refRaster, outputRaster, myProject.getGisSettings().utmZone, p1, p2))
    {
        delete outputRaster;
        myProject.logWarning("Unable to crop raster.");
        return;
    }

    setDTMScale(outputRaster->colorScale);
    myProject.addRaster(outputRaster, "Crop_" + refRasterFileName, myProject.getGisSettings().utmZone);

    addRasterObject(myProject.objectList.back());
    updateMaps();
}


void MainWindow::on_action_Raster_map_algebra_triggered()
{
    bool isOk;
    QString rasterFileName;

    // select raster
    gis::Crit3DRasterGrid *rasterX = selectRaster("Select raster X (reference)", rasterFileName, isOk);
    if (! isOk)
        return;

    gis::Crit3DRasterGrid *rasterY = selectRaster("Select raster Y", rasterFileName, isOk);
    if (! isOk)
        return;

    QStringList operationList = {"x + y", "x - y", "x * y", "x / y", "min(x,y)", "max(x,y)"};
    FormSelection formSelection(operationList, "Select operation");
    if (formSelection.result() != QDialog::Accepted)
        return;

    QString operationStr = formSelection.getSelection();
    operationType operation;
    if (operationStr == "x + y")
        operation = operationSum;
    else if (operationStr == "x - y")
        operation = operationSubtract;
    else if (operationStr == "x * y")
        operation = operationProduct;
    else if (operationStr == "x / y")
        operation = operationDivide;
    else if (operationStr == "min(x,y)")
        operation = operationMin;
    else if (operationStr == "max(x,y)")
        operation = operationMax;
    else
    {
        myProject.logWarning("Wrong operation: " + operationStr);
        return;
    }

    gis::Crit3DRasterGrid* outputRaster = new gis::Crit3DRasterGrid();
    if (! gis::mapAlgebra(rasterX, rasterY, outputRaster, operation))
    {
        myProject.logWarning("Error in map algebra.");
        return;
    }

    setDTMScale(outputRaster->colorScale);
    myProject.addRaster(outputRaster, operationStr, myProject.getGisSettings().utmZone);

    addRasterObject(myProject.objectList.back());
    updateMaps();
}


