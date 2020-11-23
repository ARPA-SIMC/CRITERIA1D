#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "heat1D.h"
#include "commonConstants.h"

#include <iostream>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <qclipboard.h>


extern Heat1DSimulation myHeat1D;

bool useInputMeteoData, useInputSoilData;

double *myTempInput;
double *myPrecInput;
double *myRHInput;
double *myWSInput;
double *myRadInput;
double *myNetRadInput;

long myInputNumber;

bool meteoDataLoaded = false;
bool soilDataLoaded = false;

Crit3DOut myHeatOutput;
Qsoil *myInputSoils = nullptr;

void initializeWeatherData()
{
    myTempInput = nullptr;
    myPrecInput = nullptr;
    myRHInput = nullptr;
    myWSInput = nullptr;
    myRadInput = nullptr;
    myNetRadInput = nullptr;
}

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

    // simulation
    if (!useInputMeteoData)
    {
        myHeat1D.SimulationStart = ui->lineEditSimStart->text().toInt();
        myHeat1D.SimulationStop = ui->lineEditSimStop->text().toInt();
    }

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

    ui->prgBar->setMaximum(myHeat1D.SimulationStop);

    double myPIniHour, myPHours;
    double myT, myRH, myWS, myNR, myP;

    myPIniHour = ui->lineEditPrecStart->text().toInt();
    myPHours = ui->lineEditPrecHours->text().toInt();
    myNR = NODATA;

    int outTimeStep = ui->lineEditTimeStep->text().toInt();

    do
    {
        ++(myHeat1D.CurrentHour);

        qApp->processEvents();

        if (useInputMeteoData)
        {
            myT = myTempInput[myHeat1D.CurrentHour-1] + 273.16;
            myP = myPrecInput[myHeat1D.CurrentHour-1];
            myRH = myRHInput[myHeat1D.CurrentHour-1];
            myWS = myWSInput[myHeat1D.CurrentHour-1];
            myNR = myNetRadInput[myHeat1D.CurrentHour-1];
        }
        else
        {
            myT = ui->lineEditAtmT->text().toDouble() + 273.16;
            myRH = ui->lineEditAtmRH->text().toDouble();
            myWS = ui->lineEditAtmWS->text().toDouble();
            myNR = ui->lineEditAtmFlux->text().toDouble();

            if ((myHeat1D.CurrentHour >= myPIniHour) && (myHeat1D.CurrentHour < myPIniHour + myPHours))
                myP = ui->lineEditPrecHourlyAmount->text().toDouble();
            else
                myP = 0.;
        }

        runHeat1D(myT, myRH, myWS, myNR, myP);

        getHourlyOutputAllPeriod(0, getNodesNumber(), &myHeatOutput);

        ui->prgBar->setValue(myHeat1D.CurrentHour);

    } while (myHeat1D.CurrentHour < myHeat1D.SimulationStop);

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
    initializeWeatherData();

    QString myFilename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Files (*.*)"));

    myInputNumber = 0;

    QFile myFile(myFilename);
    QTextStream myStream(&myFile);
    QStringList myWords;
    QString myWord;

    if (myFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {       
        while (!myStream.atEnd()) {
            QString myLine = myStream.readLine();
            myWords += myLine.split(",");
            myInputNumber++;
        }

        myTempInput = (double *) calloc(myInputNumber, sizeof(double));
        myPrecInput = (double *) calloc(myInputNumber, sizeof(double));
        myRHInput= (double *) calloc(myInputNumber, sizeof(double));
        myWSInput = (double *) calloc(myInputNumber, sizeof(double));
        myNetRadInput = (double *) calloc(myInputNumber, sizeof(double));

        long myHourIndex;

        int myFieldNumber = 6;

        for (myHourIndex = 1; myHourIndex < myInputNumber; myHourIndex++)
        {
            myWord = myWords.at(myHourIndex*myFieldNumber+1);
            if (myWord != "")
                myTempInput[myHourIndex-1] = myWord.toDouble();
            else
            {
                std::cout << "missing data at line " << myHourIndex+1 << std::endl;
                return;
            }

            myWord = myWords.at(myHourIndex*myFieldNumber+2);
            if (myWord != "")
                myPrecInput[myHourIndex-1] = myWord.toDouble();
            else
            {
                std::cout << "missing data at line " << myHourIndex+1 << std::endl;
                return;
            }

            myWord = myWords.at(myHourIndex*myFieldNumber+3);
            if (myWord != "")
                myRHInput[myHourIndex-1] = myWord.toDouble();
            else
            {
                std::cout << "missing data at line " << myHourIndex+1 << std::endl;
                return;
            }

            myWord = myWords.at(myHourIndex*myFieldNumber+4);
            if (myWord != "")
                myWSInput[myHourIndex-1] = myWord.toDouble();
            else
            {
                std::cout << "missing data at line " << myHourIndex+1 << std::endl;
                return;
            }

            myWord = myWords.at(myHourIndex*myFieldNumber+5);
            if (myWord != "")
                myNetRadInput[myHourIndex-1] = myWord.toDouble();
            else
            {
                std::cout << "missing data at line " << myHourIndex+1 << std::endl;
                return;
            }

            myHeat1D.SimulationStop++;
        }

        meteoDataLoaded = true;

        ui->chkUseInputMeteo->setEnabled(true);
        ui->chkUseInputMeteo->setChecked(true);
        ui->groupBox_atmFixedData->setEnabled(false);
        ui->groupBox_simTime->setEnabled(false);
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
    ui->groupBox_simTime->setEnabled(! ui->chkUseInputMeteo->isChecked());
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

