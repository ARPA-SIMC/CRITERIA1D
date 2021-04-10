#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commonConstants.h"
#include "heat1D.h"

#include <QDebug>
#include <iostream>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <qclipboard.h>


extern Heat1DSimulation myHeat1D;

bool useInputMeteoData, useInputSoilData;

long myInputNumber;

bool meteoDataLoaded = false;
bool soilDataLoaded = false;

Crit3DOut myHeatOutput;
Qsoil *myInputSoils = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    outLinearPlot = new LinearGraph();
    ui->layoutPlot->addWidget(outLinearPlot);
    outColorMapPlot = new ColorMapGraph();
    ui->layoutPlot->addWidget(outColorMapPlot);
    outColorMapPlot->hide();
    outColorMapPlot->setFixedSize(outLinearPlot->size());

    ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->listWidget->addItem("soil temperature (degC)");
    ui->listWidget->addItem("soil water content (m3 m-3)");
    ui->listWidget->addItem("total heat flux (W m-2)");
    ui->listWidget->addItem("diffusive heat flux (W m-2)");
    ui->listWidget->addItem("isothermal latent heat flux (W m-2)");
    ui->listWidget->addItem("thermal latent heat flux (W m-2)");
    ui->listWidget->addItem("isothermal water liquid flux (l s-1)");
    ui->listWidget->addItem("thermal water liquid flux (l s-1)");
    ui->listWidget->addItem("isothermal water vapor flux (l s-1)");
    ui->listWidget->addItem("thermal water vapor flux (l s-1)");
    ui->listWidget->addItem("surface energy balance (W m-2)");
    ui->listWidget->addItem("surface resistances (s m-1)");
    ui->listWidget->addItem("heat conductivity (W m-1 K-1)");
    ui->listWidget->addItem("error balance ()");
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::initializeModel()
{

    useInputMeteoData = ui->chkUseInputMeteo->isChecked() && meteoDataLoaded;
    useInputSoilData = ui->chkUseInputSoil->isChecked() && soilDataLoaded;

    setThickness(ui->lineEditThickness->text().toDouble());
    setTotalDepth(ui->lineEditDepth->text().toDouble());

    // initialization
    setInitialSaturation(ui->lineEditIniWaterContentTop->text().toDouble(), ui->lineEditIniWaterContentBottom->text().toDouble());
    setInitialTemperature(ui->lineEditIniTTop->text().toDouble(), ui->lineEditIniTBottom->text().toDouble());

    // processes
    bool computeHeat = ui->chkBoxHeat->isChecked();
    bool computeWater = ui->chkBoxWater->isChecked();
    bool computeLatent = ui->chkBoxLatent->isChecked();
    bool computeAdvective = ui->chkBoxAdvective->isChecked();

    setProcesses(computeWater, computeHeat, false);
    setProcessesHeat(computeLatent, computeAdvective);

    // surface
    setSurface(ui->lineEditWidth->text().toDouble(),
               ui->lineEditManningRoughness->text().toDouble(),
               ui->lineEditPlough->text().toDouble(),
               ui->lineEditRoughnessHeight->text().toDouble());


    // bottom boundary
    setBottomTemperature(ui->lineEditBottomT->text().toDouble(), ui->lineEditBottomZ->text().toDouble());

    // set soil
    if (! soilDataLoaded)
        setSoil(ui->lineEditThetaS->text().toDouble(),
                ui->lineEditThetaR->text().toDouble(),
                ui->lineEditClay->text().toDouble(),
                ui->lineEditOrganic->text().toDouble());

    return (initializeHeat1D(useInputSoilData));
}

void MainWindow::on_pushRunAllPeriod_clicked()
{
    myHeatOutput.clean();

    if (! initializeModel())
    {
        ui->labelInfo->setText("Initialization failed");
        return;
    }

    long nodesNr = getNodesNumber();
    myHeatOutput.nrLayers = nodesNr;
    myHeatOutput.layerThickness = ui->lineEditThickness->text().toFloat();

    double myPIniHour, myPHours;
    double myT, myRH, myWS, myNR, myP;

    myPIniHour = ui->lineEditPrecStart->text().toInt();
    myPHours = ui->lineEditPrecHours->text().toInt();
    myNR = NODATA;

    int outTimeStep = ui->lineEditTimeStep->text().toInt();

    int hourFin;
    if (!useInputMeteoData)
        hourFin = ui->lineEditSimDuration->text().toInt();
    else
        hourFin = int(myHeat1D.meteoValues.size());


    QDateTime myTimeIni = QDateTime::currentDateTime();
    myTimeIni.setTime(QTime(0,0,0,0));
    QDateTime myTimeFin = myTimeIni.addSecs(3600 * hourFin);
    QDateTime myTime = myTimeIni;
    QDateTime myRefHour = myTimeIni;

    int indexHour = 0;

    ui->prgBar->setMaximum(hourFin);

    do
    {
        myTime = myTime.addSecs(outTimeStep);

        if (myTime.secsTo(myRefHour) < 0)
        {
            myRefHour = myRefHour.addSecs(3600);
            indexHour++;
        }

        if (useInputMeteoData)
        {
            myT = myHeat1D.meteoValues[indexHour - 1].airTemperature + 273.16;
            myP = myHeat1D.meteoValues[indexHour - 1].precipitation;
            myRH = myHeat1D.meteoValues[indexHour - 1].relativeHumidity;
            myWS = myHeat1D.meteoValues[indexHour - 1].windSpeed;
            myNR = myHeat1D.meteoValues[indexHour - 1].netIrradiance;
        }
        else
        {
            myT = ui->lineEditAtmT->text().toDouble() + 273.16;
            myRH = ui->lineEditAtmRH->text().toDouble();
            myWS = ui->lineEditAtmWS->text().toDouble();
            myNR = ui->lineEditAtmFlux->text().toDouble();

            if ((indexHour >= myPIniHour) && (indexHour < myPIniHour + myPHours))
                myP = ui->lineEditPrecHourlyAmount->text().toDouble();
            else
                myP = 0.;
        }

        runHeat1D(myT, myRH, myWS, myNR, myP, outTimeStep);

        getOutputAllPeriod(0, getNodesNumber(), &myHeatOutput);

        ui->prgBar->setValue(indexHour);
        qApp->processEvents();

    } while (myTime < myTimeFin);

    //outPlot->drawOutput(outputGroup::soilTemperature, &myHeatOutput);
    Crit3DColorScale myColorScale;
    bool graphLinear;
    setColorScale(&myColorScale, outputGroup::soilTemperature, &myHeatOutput, &graphLinear);
    if (myHeatOutput.nrValues != 0)
    {
        if (graphLinear == true)
        {
            outColorMapPlot->hide();
            outLinearPlot->setVisible(true);
            outLinearPlot->draw(myColorScale, &myHeatOutput, outputGroup::soilTemperature);
        }
        else
        {
            outLinearPlot->hide();
            outColorMapPlot->setVisible(true);
            outColorMapPlot->draw(&myHeatOutput, outputGroup::soilTemperature);
        }
    }
    ui->listWidget->item(outputGroup::soilTemperature)->setSelected(true);
}


void MainWindow::on_pushLoadFileSoil_clicked()
{
    myInputSoils = nullptr;

    QString myFilename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Files (*.*)"));

    myInputNumber = 0;

    QFile myFile(myFilename);
    QTextStream myStream(&myFile);
    QStringList myWords;

    if (myFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!myStream.atEnd()) {
            QString myLine = myStream.readLine();
            myWords += myLine.split(",");
            myInputNumber++;
        }

        myInputSoils = (Qsoil *) calloc(myInputNumber, sizeof(Qsoil));

        long myIndex;
        int myFieldNumber = 22;

        setSoilHorizonNumber(myInputNumber-1);

        for (myIndex = 1; myIndex < myInputNumber; myIndex++)
        {
            myInputSoils[myIndex-1].profSup = myWords.at(myIndex*myFieldNumber).toDouble();
            myInputSoils[myIndex-1].profInf = myWords.at(myIndex*myFieldNumber + 1).toDouble();
            myInputSoils[myIndex-1].Clay = myWords.at(myIndex*myFieldNumber + 5).toDouble();
            myInputSoils[myIndex-1].OrganicMatter = myWords.at(myIndex*myFieldNumber + 8).toDouble();
            myInputSoils[myIndex-1].CEC = myWords.at(myIndex*myFieldNumber + 9).toDouble();
            myInputSoils[myIndex-1].K_sat = myWords.at(myIndex*myFieldNumber + 11).toDouble();
            myInputSoils[myIndex-1].PH = myWords.at(myIndex*myFieldNumber + 12).toDouble();
            myInputSoils[myIndex-1].VG_alfa = myWords.at(myIndex*myFieldNumber + 15).toDouble();
            myInputSoils[myIndex-1].VG_n = myWords.at(myIndex*myFieldNumber + 16).toDouble();
            myInputSoils[myIndex-1].VG_m = 1.0 - 1.0 / myInputSoils[myIndex-1].VG_n;
            myInputSoils[myIndex-1].Theta_r = myWords.at(myIndex*myFieldNumber + 18).toDouble();
            myInputSoils[myIndex-1].Theta_s = myWords.at(myIndex*myFieldNumber + 19).toDouble();
            myInputSoils[myIndex-1].VG_he = myWords.at(myIndex*myFieldNumber + 20).toDouble();
            myInputSoils[myIndex-1].Mualem_L = myWords.at(myIndex*myFieldNumber + 21).toDouble();
            myInputSoils[myIndex-1].Plough = 0.005;
            myInputSoils[myIndex-1].Roughness = 0.024;
        }

        soilDataLoaded = true;
        ui->chkUseInputSoil->setEnabled(true);
        ui->chkUseInputSoil->setChecked(true);
        ui->groupBox_soil->setEnabled(false);
    }

    myFile.close();
}



void MainWindow::on_pushLoadFileMeteo_clicked()
{
    myHeat1D.cleanMeteo();

    QString myFilename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Files (*.*)"));

    QFile myFile(myFilename);
    QTextStream myStream(&myFile);
    QStringList myWords;
    QString myWord;

    if (myFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //headers
        QString myLine = myStream.readLine();

        while (!myStream.atEnd())
        {
            QString myLine = myStream.readLine();
            myWords = myLine.split(",");

            meteo* myMeteo = new meteo();

            if (myWords[1] == "" || myWords[2] == "" || myWords[3] == "" || myWords[4] == "" || myWords[5] == "")
            {
                QMessageBox::information(nullptr, "No data", "Missing weather data in line " + QString::number(myHeat1D.meteoValues.size() + 1));
                return;
            }

            myMeteo->airTemperature = myWords.at(1).toDouble();
            myMeteo->precipitation = myWords.at(2).toDouble();
            myMeteo->relativeHumidity = myWords.at(3).toDouble();
            myMeteo->windSpeed = myWords.at(4).toDouble();
            myMeteo->netIrradiance = myWords.at(5).toDouble();

            myHeat1D.meteoValues.push_back(*myMeteo);
        }

        meteoDataLoaded = true;

        ui->chkUseInputMeteo->setEnabled(true);
        ui->chkUseInputMeteo->setChecked(true);
        ui->groupBox_atmFixedData->setEnabled(false);
        ui->lineEditSimDuration->setEnabled(false);
    }

    myFile.close();

}


void MainWindow::on_pushCopyOutput_clicked()
{
    QString myTextOutput("");
    myTextOutput = myHeatOutput.getTextOutput((outputGroup)ui->listWidget->row(ui->listWidget->selectedItems().first()));

    QClipboard *myClip = QApplication::clipboard();
    myClip->setText(myTextOutput);
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem * selItem)
{
    outputGroup myType = (outputGroup)ui->listWidget->row(selItem);
    Crit3DColorScale myColorScale;
    bool graphLinear;
    setColorScale(&myColorScale, myType, &myHeatOutput, &graphLinear);
    if (myHeatOutput.nrValues != 0)
    {
        if (graphLinear == true)
        {
            outColorMapPlot->hide();
            outLinearPlot->setVisible(true);
            outLinearPlot->draw(myColorScale, &myHeatOutput, myType);
        }
        else
        {
            outLinearPlot->hide();
            outColorMapPlot->setVisible(true);
            outColorMapPlot->draw(&myHeatOutput, myType);
        }
    }
}


void MainWindow::on_chkUseInputMeteo_clicked()
{
    if (ui->chkUseInputMeteo->isChecked() && ! meteoDataLoaded) return;

    ui->groupBox_atmFixedData->setEnabled(! ui->chkUseInputMeteo->isChecked());
    ui->lineEditSimDuration->setEnabled(! ui->chkUseInputMeteo->isChecked());
}

void MainWindow::on_chkUseInputSoil_clicked()
{
    if (ui->chkUseInputSoil->isChecked() && ! soilDataLoaded) return;

    ui->groupBox_soil->setEnabled(! ui->chkUseInputSoil->isCheckable());
}

void MainWindow::on_chkBoxHeat_clicked()
{
    ui->chkBoxLatent->setEnabled(ui->chkBoxHeat->isEnabled());
    ui->chkBoxAdvective->setEnabled(ui->chkBoxHeat->isEnabled());
}

