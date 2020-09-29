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

#include "dialogSelectField.h"
#include "dialogUcmPrevailing.h"
#include "dialogUcmIntersection.h"
#include "dbfTableDialog.h"
#include "commonConstants.h"
#include "shapeUtilities.h"

#ifdef GDAL
    #include "gdalExtensions.h"
#endif

#include "mainWindow.h"
#include "ui_mainWindow.h"


#define MAPBORDER 11
#define INFOHEIGHT 40
#define TOOLSWIDTH 260


static CriteriaGeoProject myProject;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isDoubleClick = false;

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
    //connect(this->mapView, SIGNAL(mouseMoveSignal(const QPoint&)), this, SLOT(mouseMove(const QPoint&)));

    connect(ui->checkList, &QListWidget::itemClicked, [=](QListWidgetItem* item){ this->itemClicked(item); });
    ui->checkList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->checkList, &QListWidget::customContextMenuRequested, [=](const QPoint point){ this->itemMenuRequested(point); });

    this->setMouseTracking(true);
}


MainWindow::~MainWindow()
{
    if (! this->rasterObjList.empty())
        for (unsigned int i = 0; i < this->rasterObjList.size(); i++)
            delete this->rasterObjList[i];

    if (! this->shapeObjList.empty())
        for (unsigned int i = 0; i < this->shapeObjList.size(); i++)
            delete this->shapeObjList[i];

    if (myProject.outputProject.isProjectLoaded)
    {
        QDir tmpDir(myProject.outputProject.path + "tmp");
        tmpDir.removeRecursively();
        myProject.outputProject.closeProject();
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
    if (! this->rasterObjList.empty())
        for (unsigned int i = 0; i < this->rasterObjList.size(); i++)
            this->rasterObjList[i]->updateCenter();
    if (! this->shapeObjList.empty())
        for (unsigned int i = 0; i < this->shapeObjList.size(); i++)
            this->shapeObjList[i]->updateCenter();
}


void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    if (! this->isDoubleClick)
    {
        if (event->button() == Qt::LeftButton)
        {
            QString selShape = checkSelectedShape();
            this->ui->statusBar->showMessage(selShape);
        }
    }
    this->isDoubleClick = false;

    this->updateMaps();
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


void MainWindow::mouseMove(const QPoint& eventPos)
{
    if (! isInsideMap(eventPos)) return;

    Position geoPoint = this->mapView->mapToScene(eventPos);
    this->ui->statusBar->showMessage(QString::number(geoPoint.latitude()) + " " + QString::number(geoPoint.longitude()));
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


QPoint MainWindow::getMapPos(const QPoint& pos)
{
    QPoint mapPos;
    int dx = ui->widgetMap->x();
    int dy = ui->widgetMap->y() + ui->menuBar->height();
    mapPos.setX(pos.x() - dx - MAPBORDER);
    mapPos.setY(pos.y() - dy - MAPBORDER);
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

    RasterObject* newRasterObj = new RasterObject(this->mapView);
    newRasterObj->setOpacity(0.66);
    newRasterObj->initializeUTM(myObject->getRaster(), myObject->gisSettings, false);
    this->rasterObjList.push_back(newRasterObj);

    this->mapView->scene()->addObject(newRasterObj);
    this->updateMaps();
}


bool MainWindow::addShapeObject(GisObject* myObject)
{
    // check zoneNumber
    int zoneNumber = myObject->getShapeHandler()->getUtmZone();
    if (zoneNumber < 1 || zoneNumber > 60)
    {
        QMessageBox::critical(nullptr, "ERROR!", "Wrong UTM zone.");
        return false;
    }

    // add item
    QListWidgetItem* item = new QListWidgetItem("[SHAPE] " + myObject->fileName);
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


void MainWindow::on_actionLoadRaster_triggered()
{
    #ifdef GDAL
        QStringList rasterFormats = getGdalRasterReadExtension();

        rasterFormats.sort();
        rasterFormats.insert(0, tr("ESRI float (*.flt)"));
        //rasterFormats.insert(0, tr("all files (*.*)"));

        QString fileNameWithPath = QFileDialog::getOpenFileName(this, tr("Open raster file"), "", rasterFormats.join(";;"));
    #else
         QString fileNameWithPath = QFileDialog::getOpenFileName(this, tr("Open raster file"), "", tr("ESRI grid files (*.flt)"));
    #endif

    if (fileNameWithPath == "") return;

    if (! myProject.loadRaster(fileNameWithPath))
        return;

    this->addRasterObject(myProject.objectList.back());

    // resize and center map
    gis::Crit3DGeoPoint* center = this->rasterObjList.back()->getRasterCenter();
    float size = this->rasterObjList.back()->getRasterMaxSize();
    this->mapView->setZoomLevel(quint8(log2(ui->widgetMap->width() / size)));
    this->mapView->centerOn(qreal(center->longitude), qreal(center->latitude));
    this->updateMaps();
}


void MainWindow::on_actionLoadShapefile_triggered()
{
    QString fileNameWithPath = QFileDialog::getOpenFileName(this, tr("Open Shapefile"), "", tr("shp files (*.shp)"));
    if (fileNameWithPath == "") return;

    if (! myProject.loadShapefile(fileNameWithPath))
        return;

    GisObject* myObject = myProject.objectList.back();

    this->addShapeObject(myObject);
}


void MainWindow::itemClicked(QListWidgetItem* item)
{

    int pos = ui->checkList->row(item);
    GisObject* myObject = myProject.objectList.at(unsigned(pos));

    if (myObject->type == gisObjectRaster)
    {
        unsigned int i;
        for (i = 0; i < rasterObjList.size(); i++)
        {
            if (rasterObjList.at(i)->getRaster() == myObject->getRaster())
            {
                break;
            }
        }

        myObject->isSelected = item->checkState();
        rasterObjList.at(i)->setVisible(myObject->isSelected);
    }
    else if (myObject->type == gisObjectShape)
    {
        unsigned int i;
        for (i = 0; i < shapeObjList.size(); i++)
        {
            if (shapeObjList.at(i)->getShapePointer() == myObject->getShapeHandler())
            {
                break;
            }
        }

        myObject->isSelected = item->checkState();
        shapeObjList.at(i)->setVisible(myObject->isSelected);
    }
}


void MainWindow::saveRaster(GisObject* myObject)
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save raster Grid"), "", tr("ESRI grid files (*.flt)"));
    if (fileName == "") return;

    std::string error;
    if (! gis::writeEsriGrid(fileName.toStdString(), myObject->getRaster(), &error))
    {
        QMessageBox::information(nullptr, "Error", QString::fromStdString(error));
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
    unsigned int i;
    for (i = 0; i < this->rasterObjList.size(); i++)
    {
        if (this->rasterObjList.at(i)->getRaster() == myObject->getRaster()) break;
    }

    if (i < this->rasterObjList.size())
    {
        // remove from scene
        this->mapView->scene()->removeObject(this->rasterObjList.at(i));
        this->rasterObjList.at(i)->clear();
        this->rasterObjList.erase(this->rasterObjList.begin()+i);
    }
}


RasterObject* MainWindow::getRasterObject(GisObject* myObject)
{
    for (unsigned int i = 0; i < rasterObjList.size(); i++)
    {
        if (rasterObjList.at(i)->getRaster() == myObject->getRaster())
            return rasterObjList.at(i);
    }

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


void MainWindow::setShapeStyle(GisObject* myObject)
{
    DialogSelectField shapeFieldDialog(myObject->getShapeHandler(), myObject->fileName, false, SHAPESTYLE);
    if (shapeFieldDialog.result() == QDialog::Accepted)
    {
        MapGraphicsShapeObject* shapeObject = getShapeObject(myObject);
        std::string fieldName = shapeFieldDialog.getFieldSelected().toStdString();
        DBFFieldType fieldType = myObject->getShapeHandler()->getFieldType(fieldName);

        if (fieldType == FTString)
        {
            shapeObject->setCategories(fieldName);
        }
        else
        {
            shapeObject->setNumericValues(fieldName);
        }

        setZeroCenteredScale(shapeObject->colorScale);
        //setTemperatureScale(shapeObject->colorScale);
        shapeObject->setFill(true);
    }
}

bool MainWindow::exportToRaster(GisObject* myObject)
{
#ifdef GDAL
    DialogSelectField shapeFieldDialog(myObject->getShapeHandler(), myObject->fileName, true, GDALRASTER);
    if (shapeFieldDialog.result() == QDialog::Accepted)
    {
        std::string fieldName = shapeFieldDialog.getFieldSelected().toStdString();
        std::string shapeFilePath = (myObject->getShapeHandler())->getFilepath();
        QString res;
        if (shapeFieldDialog.getCellSize() == 0)
        {
            res = "200";  // default resolution value
        }
        else
        {
            res = QString::number(shapeFieldDialog.getCellSize());
        }

        QStringList gdalExt = getGdalRasterWriteExtension();
        QString outputName = QFileDialog::getSaveFileName(this, tr("Save raster as"), "", gdalExt.join(";\n"));
        if (outputName == "")
        {
            QMessageBox::information(nullptr, "Insert output name", "missing raster name");
            return false;
        }

        QString errorStr;
        if (!myProject.createRaster(QString::fromStdString(shapeFilePath), fieldName, res, outputName, errorStr))
        {
            QMessageBox::critical(nullptr, "ERROR!", "GDAL Error: " + errorStr);
            return false;
        }
        addRasterObject(myProject.objectList.back());
        this->updateMaps();
        return true;
    }
    return true;
#else
    QMessageBox::critical(nullptr, "ERROR", "Missing GDAL");
    return false;
#endif
}


void MainWindow::itemMenuRequested(const QPoint point)
{
    QPoint itemPoint = ui->checkList->mapToGlobal(point);
    QListWidgetItem* item = ui->checkList->itemAt(point);
    int pos = ui->checkList->row(item);
    GisObject* myObject = myProject.objectList.at(unsigned(pos));

    QMenu submenu;
    submenu.addAction("Close");
    submenu.addSeparator();
    RasterObject* myRasterObject = nullptr;

    if (myObject->type == gisObjectShape)
    {
        submenu.addAction("Save as");
        submenu.addSeparator();
        submenu.addAction("Show data");
        submenu.addAction("Attribute table");
        submenu.addSeparator();
        submenu.addAction("Set style");
        submenu.addAction("Export to raster");
    }
    else if (myObject->type == gisObjectRaster)
    {
        myRasterObject = getRasterObject(myObject);
        if (myRasterObject != nullptr)
        {
            submenu.addAction("Save as");
            submenu.addSeparator();
            submenu.addAction("Set grayscale");
            submenu.addAction("Set default scale");
            submenu.addSeparator();
            if (myRasterObject->opacity() < 1)
                submenu.addAction("Set opaque");
            else
                submenu.addAction("Set transparent");
        }
    }
    QAction* rightClickItem = submenu.exec(itemPoint);

    if (rightClickItem)
    {
        if (rightClickItem->text().contains("Close") )
        {
            if (myObject->type == gisObjectRaster)
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
        else if (rightClickItem->text().contains("Show data"))
        {
            DialogShapeProperties showData(myObject->getShapeHandler(), myObject->fileName);
        }
        else if (rightClickItem->text().contains("Attribute table"))
        {
            DbfTableDialog Table(myObject->getShapeHandler(), myObject->fileName);
        }
        else if (rightClickItem->text().contains("Set style"))
        {
            setShapeStyle(myObject);
        }
        else if (rightClickItem->text().contains("Export to raster"))
        {
            exportToRaster(myObject);
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
        else if (rightClickItem->text().contains("Set grayscale"))
        {
            if (myObject->type == gisObjectRaster)
            {
                setGrayScale(myObject->getRaster()->colorScale);
                myRasterObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Set default scale"))
        {
            if (myObject->type == gisObjectRaster)
            {
                setDefaultDEMScale(myObject->getRaster()->colorScale);
                myRasterObject->redrawRequested();
            }
        }
        else if (rightClickItem->text().contains("Set opaque"))
        {
            if (myObject->type == gisObjectRaster)
            {
                myRasterObject->setOpacity(1.0);
                update();
            }
        }
        else if (rightClickItem->text().contains("Set transparent"))
        {
            if (myObject->type == gisObjectRaster)
            {
                myRasterObject->setOpacity(0.66);
                update();
            }
        }
    }
    return;
}


QString MainWindow::checkSelectedShape()
{
    QListWidgetItem * itemSelected = ui->checkList->currentItem();

    // no shapefile selected -> exit
    if (itemSelected == nullptr)
        return "";
    if (! itemSelected->text().contains("SHAPE"))
        return"";

    int pos = ui->checkList->row(itemSelected);
    GisObject* myObject = myProject.objectList.at(unsigned(pos));

    return myObject->fileName;
}


void MainWindow::on_actionRasterize_shape_triggered()
{
    QListWidgetItem * itemSelected = ui->checkList->currentItem();
    if (itemSelected == nullptr)
    {
        QMessageBox::information(nullptr, "No items selected", "Select a shape");
        return;
    }
    else if (!itemSelected->text().contains("SHAPE"))
    {
        QMessageBox::information(nullptr, "No shape selected", "Select a shape");
        return;
    }
    else
    {
        int pos = ui->checkList->row(itemSelected);
        GisObject* myObject = myProject.objectList.at(unsigned(pos));
        DialogSelectField numericField(myObject->getShapeHandler(), myObject->fileName, true, RASTERIZE);
        if (numericField.result() == QDialog::Accepted)
        {
            double resolution;
            if (numericField.getCellSize() == 0)
            {
                resolution = 100;  // default resolution value
            }
            else
            {
                resolution = numericField.getCellSize();
            }
            myProject.getRasterFromShape(*(myObject->getShapeHandler()), numericField.getFieldSelected(),
                                         numericField.getOutputName(), resolution, true);
            addRasterObject(myProject.objectList.back());
            this->updateMaps();
        }
    }
}


void MainWindow::on_actionCompute_Ucm_prevailing_triggered()
{

    if (shapeObjList.empty())
    {
        QMessageBox::information(nullptr, "No shape loaded", "Load a shape");
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

    if (myProject.addUnitCropMap(ucmDialog.getCrop(), ucmDialog.getSoil(), ucmDialog.getMeteo(),
                                 ucmDialog.getIdCrop().toStdString(), ucmDialog.getIdSoil().toStdString(),
                                 ucmDialog.getIdMeteo().toStdString(), ucmDialog.getCellSize(), ucmFileName, isPrevailing, true))
    {
        addShapeObject(myProject.objectList.back());
    }
}

void MainWindow::on_actionCompute_Ucm_intersection_triggered()
{
    if (shapeObjList.empty())
    {
        QMessageBox::information(nullptr, "No shape loaded", "Load a shape");
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

    if (myProject.addUnitCropMap(ucmDialog.getCrop(), ucmDialog.getSoil(), ucmDialog.getMeteo(),
                                 ucmDialog.getIdCrop().toStdString(), ucmDialog.getIdSoil().toStdString(),
                                 ucmDialog.getIdMeteo().toStdString(), NODATA, ucmFileName, isPrevailing, true))
    {
        addShapeObject(myProject.objectList.back());
    }

}


void MainWindow::on_actionExtract_Unit_Crop_Map_list_triggered()
{
    QListWidgetItem * itemSelected = ui->checkList->currentItem();
    if (itemSelected == nullptr || !itemSelected->text().contains("SHAPE"))
    {
        QMessageBox::information(nullptr, "No shape selected", "Select a shape");
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

    QListWidgetItem * itemSelected = ui->checkList->currentItem();
    if (shapeObjList.empty())
    {
        QMessageBox::information(nullptr, "No shape loaded", "Load a shape");
        return;
    }
    else if (itemSelected == nullptr || !itemSelected->text().contains("SHAPE"))
    {
        QMessageBox::information(nullptr, "No shape selected", "Select a shape");
        return;
    }
    else
    {
        int pos = ui->checkList->row(itemSelected);
        QString fileCsv = QFileDialog::getOpenFileName(this, tr("Open CSV data file"), "", tr("CSV files (*.csv)"));

        if (fileCsv == "")
        {
            QMessageBox::information(nullptr, "Select CSV data to read", "missing CSV data");
            return;
        }

        QString fileCsvRef = QFileDialog::getOpenFileName(this, tr("Open output format"), "", tr("CSV files (*.csv)"));

        if (fileCsvRef == "")
        {
            QMessageBox::information(nullptr, "Select output format", "missing CSV file");
            return;
        }

        QString outputName = QFileDialog::getSaveFileName(this, tr("Save Shapefile as"), "", tr("shp files (*.shp)"));
        if (outputName == "")
        {
            QMessageBox::information(nullptr, "Insert output name", "missing shapefile name");
            return;
        }

        myProject.createShapeFromCsv(pos, fileCsv, fileCsvRef, outputName);
    }
}


void MainWindow::on_actionLoadProject_triggered()
{
    QString projFileName = QFileDialog::getOpenFileName(this, tr("Open project"), "", tr("Settings files (*.ini)"));

    if (projFileName == "") return;

    // set current dateTime, then GUI overwrite this innformation
    int myResult = myProject.outputProject.initializeProject(projFileName, QDateTime::currentDateTime().date());
    if (myResult != CRIT3D_OK)
    {
        QMessageBox::information(nullptr, "Project setting error", myProject.outputProject.projectError);
        return;
    }
    else
    {
        QDir().mkdir(myProject.outputProject.path + "tmp");
    }

    if (! myProject.loadShapefile(myProject.outputProject.ucmFileName))
        return;

    GisObject* myObject = myProject.objectList.back();

    // enable Output map action
    this->addShapeObject(myObject);
    QMenu *menu = nullptr;
    menu = this->menuBar()->findChild<QMenu *>("menuTools");
    if (menu != nullptr)
    {
        QList<QAction*> list = menu->actions();
        foreach (QAction *action, list)
        {
            if (action->text() == "Output map")
            {
                action->setEnabled(true);
            }
        }
    }

}

void MainWindow::on_actionOutput_Map_triggered()
{

}
