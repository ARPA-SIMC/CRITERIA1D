#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "linearGraph.h"
#include "colorMapGraph.h"

class Plot;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_pushRunAllPeriod_clicked();  
    void on_pushLoadFileSoil_clicked();
    void on_pushLoadFileMeteo_clicked();
    void on_pushCopyOutput_clicked();
    void on_listWidget_itemClicked(QListWidgetItem *selItem);
    void on_chkUseInputMeteo_clicked();
    void on_chkUseInputSoil_clicked();

    bool initializeModel();

    void on_chkBoxHeat_clicked();

private:
    Ui::MainWindow *ui;

private:
    LinearGraph* outLinearPlot;
    ColorMapGraph* outColorMapPlot;

};

#endif // MAINWINDOW_H
