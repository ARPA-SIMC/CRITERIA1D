#ifndef MAINWINDOW_H
#define MAINWINDOW_H

    #include <QMainWindow>
    #include <QtWidgets>

    #include "tileSources/WebTileSource.h"
    #include "Position.h"

    #include "mapGraphicsRasterObject.h"
    #include "mapGraphicsShapeObject.h"
    #include "colorLegend.h"
    #include "criteriaGeoProject.h"
    #include "dialogShapeProperties.h"

    namespace Ui
    {
        class MainWindow;
    }


    /*!
     * \brief The MainWindow class
     */
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:

        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private slots:

        void on_actionLoadRaster_triggered();
        void on_actionLoadShapefile_triggered();
        void on_actionRasterize_shape_triggered();
        void on_actionCompute_Ucm_prevailing_triggered();
        void on_actionCompute_Ucm_intersection_triggered();

        void updateMaps();
        void mouseMove(const QPoint &eventPos);

        void on_actionExtract_Unit_Crop_Map_list_triggered();
        void on_actionCreate_Shape_file_from_Csv_triggered();

        void on_actionMapOpenStreetMap_triggered();
        void on_actionMapESRISatellite_triggered();
        void on_actionMapStamenTerrain_triggered();
        void on_actionMapGoogle_triggered();
        void on_actionMapGoogleSatellite_triggered();
        void on_actionMapGoogleHybridSatellite_triggered();

        void on_actionMapGoogleTerrain_triggered();

    protected:
        /*!
         * \brief mouseReleaseEvent call moveCenter
         * \param event
         */
        void mouseReleaseEvent(QMouseEvent *event);

        /*!
         * \brief mouseDoubleClickEvent implements zoom In and zoom Out
         * \param event
         */
        void mouseDoubleClickEvent(QMouseEvent * event);

        void mousePressEvent(QMouseEvent *event);

        void resizeEvent(QResizeEvent * event);

    private:
        Ui::MainWindow* ui;

        Position* startCenter;
        MapGraphicsScene* mapScene;
        MapGraphicsView* mapView;
        std::vector<RasterObject *> rasterObjList;
        std::vector<MapGraphicsShapeObject *> shapeObjList;

        void setTileSource(WebTileSource::WebTileType tileType);

        void addRasterObject(GisObject* myObject);
        bool addShapeObject(GisObject* myObject);
        void itemClicked(QListWidgetItem* item);
        void itemMenuRequested(const QPoint point);
        void saveRaster(GisObject* myObject);
        void removeRaster(GisObject* myObject);
        void removeShape(GisObject* myObject);
        void setShapeStyle(GisObject* myObject);
        void exportToGeotiff(GisObject* myObject);

        MapGraphicsShapeObject* getShapeObject(GisObject* myObject);
        RasterObject* getRasterObject(GisObject* myObject);

        QPoint getMapPos(const QPoint& pos);
        bool isInsideMap(const QPoint& pos);
    };


#endif // MAINWINDOW_H
