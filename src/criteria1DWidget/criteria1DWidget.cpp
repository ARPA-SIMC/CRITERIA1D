/*!
    CRITERIA3D
    \copyright 2016 Fausto Tomei, Gabriele Antolini, Laura Costantini
    Alberto Pistocchi, Marco Bittelli, Antonio Volta
    You should have received a copy of the GNU General Public License
    along with Nome-Programma.  If not, see <http://www.gnu.org/licenses/>.
    This file is part of CRITERIA3D.
    CRITERIA3D has been developed under contract issued by A.R.P.A. Emilia-Romagna
    CRITERIA3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    CRITERIA3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License
    along with CRITERIA3D.  If not, see <http://www.gnu.org/licenses/>.
    contacts:
    fausto.tomei@gmail.com
    ftomei@arpae.it
*/


#include "criteria1DWidget.h"
#include "dialogNewCrop.h"
#include "dialogNewProject.h"
#include "cropDbTools.h"
#include "cropDbQuery.h"
#include "criteria1DMeteo.h"
#include "soilDbTools.h"
#include "utilities.h"
#include "commonConstants.h"
#include "soilWidget.h"
#include "meteoWidget.h"
#include "criteria1DMeteo.h"
#include "utilities.h"
#include "basicMath.h"
#include "root.h"
#include "formInfo.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QDate>
#include <QSqlQuery>
#include <QSqlError>


Criteria1DWidget::Criteria1DWidget()
{
    resize(1000, 800);
    setWindowState(Qt::WindowMaximized);

    // font
    QFont currentFont = this->font();
    currentFont.setPointSize(9);
    setFont(currentFont);

    // layout
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *saveButtonLayout = new QHBoxLayout();
    QHBoxLayout *WidgetLayout = new QHBoxLayout();
    QVBoxLayout *infoLayout = new QVBoxLayout();
    QGridLayout *caseInfoLayout = new QGridLayout();
    QGridLayout *cropInfoLayout = new QGridLayout();
    QGridLayout *meteoInfoLayout = new QGridLayout();
    QHBoxLayout *soilInfoLayout = new QHBoxLayout();
    QGridLayout *parametersLaiLayout = new QGridLayout();
    QGridLayout *parametersRootDepthLayout = new QGridLayout();
    QGridLayout *parametersIrrigationLayout = new QGridLayout();
    QGridLayout *parametersWaterStressLayout = new QGridLayout();
    QVBoxLayout *waterContentLayout = new QVBoxLayout();
    QVBoxLayout *carbonNitrogenLayout = new QVBoxLayout();

    // check save button pic
    QString docPath, saveButtonPath, updateButtonPath;
    if (searchDocPath(&docPath))
    {
        saveButtonPath = docPath + "img/saveButton.png";
        updateButtonPath = docPath + "img/updateButton.png";
    }
    else
    {
        // default appimage linux
        saveButtonPath = QCoreApplication::applicationDirPath() + "/../share/CRITERIA1D/images/saveButton.png";
        updateButtonPath = QCoreApplication::applicationDirPath() + "/../share/CRITERIA1D/images/updateButton.png";
    }

    QPixmap savePixmap;
    QPixmap updatePixmap;
    if (QFileInfo::exists(saveButtonPath))
    {
        savePixmap.load(saveButtonPath);
    }
    else
    {
        QMessageBox::critical(nullptr, "errorStr", "missing file: img/saveButton.png");
    }

    if (QFileInfo::exists(updateButtonPath))
    {
        updatePixmap.load(updateButtonPath);
    }
    else
    {
        QMessageBox::critical(nullptr, "errorStr", "missing file: img/updateButton.png");
    }

    saveButton = new QPushButton();
    updateButton = new QPushButton();
    QIcon saveButtonIcon(savePixmap);
    QIcon updateButtonIcon(updatePixmap);
    saveButton->setIcon(saveButtonIcon);
    saveButton->setIconSize(savePixmap.rect().size());
    saveButton->setFixedSize(savePixmap.rect().size());
    saveButton->setEnabled(false);
    updateButton->setEnabled(false);

    saveButtonLayout->setAlignment(Qt::AlignLeft);
    saveButtonLayout->addWidget(saveButton);

    updateButton->setIcon(updateButtonIcon);
    updateButton->setIconSize(savePixmap.rect().size());
    updateButton->setFixedSize(savePixmap.rect().size());

    saveButtonLayout->setAlignment(Qt::AlignLeft);
    saveButtonLayout->addWidget(updateButton);

    QLabel *cropId = new QLabel(tr("ID_CROP: "));
    QLabel *cropName = new QLabel(tr("crop name: "));

    cropNameValue = new QLineEdit();
    cropNameValue->setReadOnly(true);

    QLabel * cropType= new QLabel(tr("crop type: "));
    cropTypeValue = new QLineEdit();
    cropTypeValue->setReadOnly(true);

    cropSowingValue = new QSpinBox();
    cropCycleMaxValue = new QSpinBox();
    cropSowingValue->setMinimum(-365);
    cropSowingValue->setMaximum(365);
    cropCycleMaxValue->setMinimum(0);
    cropCycleMaxValue->setMaximum(365);
    cropSowing.setText("sowing DOY: ");
    cropCycleMax.setText("cycle max duration: ");

    infoCaseGroup = new QGroupBox(tr(""));
    infoCropGroup = new QGroupBox(tr(""));
    infoMeteoGroup = new QGroupBox(tr(""));
    infoSoilGroup = new QGroupBox(tr(""));
    laiParametersGroup = new QGroupBox(tr(""));
    rootParametersGroup = new QGroupBox(tr(""));
    irrigationParametersGroup = new QGroupBox(tr(""));
    waterStressParametersGroup = new QGroupBox(tr(""));
    waterContentGroup = new QGroupBox(tr(""));
    carbonNitrogenGroup = new QGroupBox(tr(""));

    float widthRatio = 0.25f;
    infoCaseGroup->setFixedWidth(this->width() * widthRatio);
    infoCropGroup->setFixedWidth(this->width() * widthRatio);
    infoMeteoGroup->setFixedWidth(this->width() * widthRatio);
    laiParametersGroup->setFixedWidth(this->width() * widthRatio);
    rootParametersGroup->setFixedWidth(this->width() * widthRatio);
    irrigationParametersGroup->setFixedWidth(this->width() * widthRatio);
    waterStressParametersGroup->setFixedWidth(this->width() * widthRatio);
    waterContentGroup->setFixedWidth(this->width() * widthRatio);
    carbonNitrogenGroup->setFixedWidth(this->width() * widthRatio);

    QFont normalFont, boldFont;
    boldFont.setBold(true);
    normalFont.setBold(false);

    infoCaseGroup->setFont(boldFont);
    infoCaseGroup->setTitle("Case study");
    infoMeteoGroup->setTitle("Meteo");
    infoSoilGroup->setTitle("Soil");
    infoCropGroup->setTitle("Crop");
    laiParametersGroup->setTitle("Crop parameters");
    rootParametersGroup->setTitle("Root parameters");
    irrigationParametersGroup->setTitle("Irrigation parameters");
    waterStressParametersGroup->setTitle("Water Stress parameters");
    waterContentGroup->setTitle("Water Content variable");
    carbonNitrogenGroup->setTitle("Carbon Nitrogen variable");

    caseListComboBox.setFont(normalFont);
    caseInfoLayout->addWidget(&caseListComboBox);

    cropInfoLayout->addWidget(cropId, 0, 0);
    cropInfoLayout->addWidget(&cropListComboBox, 0, 1);
    cropInfoLayout->addWidget(cropName, 1, 0);
    cropInfoLayout->addWidget(cropNameValue, 1, 1);
    cropInfoLayout->addWidget(cropType, 2, 0);
    cropInfoLayout->addWidget(cropTypeValue, 2, 1);
    cropInfoLayout->addWidget(&cropSowing, 3, 0);
    cropInfoLayout->addWidget(cropSowingValue, 3, 1);
    cropInfoLayout->addWidget(&cropCycleMax, 4, 0);
    cropInfoLayout->addWidget(cropCycleMaxValue, 4, 1);

    QLabel *meteoName = new QLabel(tr("METEO_NAME: "));

    QLabel *meteoYearFirst = new QLabel(tr("first year: "));
    QLabel *meteoYearLast = new QLabel(tr("last year: "));

    meteoInfoLayout->addWidget(meteoName, 0, 0);
    meteoInfoLayout->addWidget(&meteoListComboBox, 0, 1);
    meteoInfoLayout->addWidget(meteoYearFirst, 1, 0);
    meteoInfoLayout->addWidget(&firstYearListComboBox, 1, 1);
    meteoInfoLayout->addWidget(meteoYearLast, 2, 0);
    meteoInfoLayout->addWidget(&lastYearListComboBox, 2, 1);

    soilInfoLayout->addWidget(&soilListComboBox);

    QLabel *LAImin = new QLabel(tr("LAI min [m2 m-2]: "));
    LAIminValue = new QDoubleSpinBox();
    LAIminValue->setMaximumWidth(laiParametersGroup->width()/5);
    LAIminValue->setMinimum(0);
    LAIminValue->setMaximum(10);
    LAIminValue->setDecimals(1);
    LAIminValue->setSingleStep(0.5);

    QLabel *LAImax = new QLabel(tr("LAI max [m2 m-2]: "));
    LAImaxValue = new QDoubleSpinBox();
    LAImaxValue->setMaximumWidth(laiParametersGroup->width()/5);
    LAImaxValue->setMinimum(0);
    LAImaxValue->setMaximum(10);
    LAImaxValue->setDecimals(1);
    LAImaxValue->setSingleStep(0.5);

    LAIgrass = new QLabel(tr("LAI grass [m2 m-2]: "));
    LAIgrassValue = new QLineEdit();
    LAIgrassValue->setMaximumWidth(laiParametersGroup->width()/5);
    QDoubleValidator* firstValidator = new QDoubleValidator(-99.0, 99.0, 3, this);
    QDoubleValidator* secondValidator = new QDoubleValidator(-9999.0, 9999.0, 3, this);
    QDoubleValidator* positiveValidator = new QDoubleValidator(0, 99999.0, 3, this);
    firstValidator->setNotation(QDoubleValidator::StandardNotation);
    secondValidator->setNotation(QDoubleValidator::StandardNotation);
    positiveValidator->setNotation(QDoubleValidator::StandardNotation);
    LAIgrassValue->setValidator(firstValidator);

    QLabel *thermalThreshold = new QLabel(tr("thermal threshold [°C]: "));
    thermalThresholdValue = new QLineEdit();
    thermalThresholdValue->setMaximumWidth(laiParametersGroup->width()/5);
    thermalThresholdValue->setValidator(firstValidator);

    QLabel *upperThermalThreshold = new QLabel(tr("upper thermal threshold [°C]: "));
    upperThermalThresholdValue = new QLineEdit();
    upperThermalThresholdValue->setMaximumWidth(laiParametersGroup->width()/5);
    upperThermalThresholdValue->setValidator(firstValidator);

    QLabel *degreeDaysEmergence = new QLabel(tr("degree days emergence [°C]: "));
    degreeDaysEmergenceValue = new QLineEdit();
    degreeDaysEmergenceValue->setMaximumWidth(laiParametersGroup->width()/5);
    degreeDaysEmergenceValue->setValidator(positiveValidator);

    QLabel *degreeDaysLAIinc = new QLabel(tr("degree days phase 1 [°C]: "));
    degreeDaysLaiIncreaseValue = new QLineEdit();
    degreeDaysLaiIncreaseValue->setMaximumWidth(laiParametersGroup->width()/5);
    degreeDaysLaiIncreaseValue->setValidator(positiveValidator);

    QLabel *degreeDaysLAIdec = new QLabel(tr("degree days phase 2 [°C]: "));
    degreeDaysLaiDecreaseValue = new QLineEdit();
    degreeDaysLaiDecreaseValue->setMaximumWidth(laiParametersGroup->width()/5);
    degreeDaysLaiDecreaseValue->setValidator(positiveValidator);

    QLabel *LAIcurveA = new QLabel(tr("LAI curve factor A [-]: "));
    LAIcurveAValue = new QLineEdit();
    LAIcurveAValue->setMaximumWidth(laiParametersGroup->width()/5);
    LAIcurveAValue->setValidator(firstValidator);

    QLabel *LAIcurveB = new QLabel(tr("LAI curve factor B [-]: "));
    LAIcurveBValue = new QLineEdit();
    LAIcurveBValue->setMaximumWidth(laiParametersGroup->width()/5);
    LAIcurveBValue->setValidator(firstValidator);

    QLabel * maxKc= new QLabel(tr("kc max [-]: "));
    maxKcValue = new QLineEdit();
    maxKcValue->setMaximumWidth(laiParametersGroup->width()/5);
    maxKcValue->setValidator(firstValidator);

    parametersLaiLayout->addWidget(LAImin, 0, 0);
    parametersLaiLayout->addWidget(LAIminValue, 0, 1);
    parametersLaiLayout->addWidget(LAImax, 1, 0);
    parametersLaiLayout->addWidget(LAImaxValue, 1, 1);
    parametersLaiLayout->addWidget(LAIgrass, 3, 0);
    parametersLaiLayout->addWidget(LAIgrassValue, 3, 1);
    parametersLaiLayout->addWidget(thermalThreshold, 4, 0);
    parametersLaiLayout->addWidget(thermalThresholdValue, 4, 1);
    parametersLaiLayout->addWidget(upperThermalThreshold, 5, 0);
    parametersLaiLayout->addWidget(upperThermalThresholdValue, 5, 1);
    parametersLaiLayout->addWidget(degreeDaysEmergence, 6, 0);
    parametersLaiLayout->addWidget(degreeDaysEmergenceValue, 6, 1);
    parametersLaiLayout->addWidget(degreeDaysLAIinc, 7, 0);
    parametersLaiLayout->addWidget(degreeDaysLaiIncreaseValue, 7, 1);
    parametersLaiLayout->addWidget(degreeDaysLAIdec, 8, 0);
    parametersLaiLayout->addWidget(degreeDaysLaiDecreaseValue, 8, 1);
    parametersLaiLayout->addWidget(LAIcurveA, 9, 0);
    parametersLaiLayout->addWidget(LAIcurveAValue, 9, 1);
    parametersLaiLayout->addWidget(LAIcurveB, 10, 0);
    parametersLaiLayout->addWidget(LAIcurveBValue, 10, 1);
    parametersLaiLayout->addWidget(maxKc, 11, 0);
    parametersLaiLayout->addWidget(maxKcValue, 11, 1);

    QLabel *rootDepthZero = new QLabel(tr("root depth zero [m]: "));
    rootDepthZeroValue = new QLineEdit();
    rootDepthZeroValue->setMaximumWidth(rootParametersGroup->width()/5);
    rootDepthZeroValue->setValidator(firstValidator);

    QLabel *rootDepthMax = new QLabel(tr("root depth max [m]: "));
    rootDepthMaxValue = new QLineEdit();
    rootDepthMaxValue->setMaximumWidth(rootParametersGroup->width()/5);
    rootDepthMaxValue->setValidator(firstValidator);

    QLabel *rootShape = new QLabel(tr("root shape: "));
    rootShapeComboBox = new QComboBox();
    rootShapeComboBox->setMaximumWidth(rootParametersGroup->width()/3);

    for (int i=0; i < nrRootDistributionType; i++)
    {
        rootDistributionType type = rootDistributionType(i);
        rootShapeComboBox->addItem(QString::fromStdString(root::getRootDistributionTypeString(type)));
    }

    QLabel *shapeDeformation = new QLabel(tr("shape deformation [-]: "));
    shapeDeformationValue = new QDoubleSpinBox();
    shapeDeformationValue->setMaximumWidth(rootParametersGroup->width()/5);
    shapeDeformationValue->setMinimum(0);
    shapeDeformationValue->setMaximum(2);
    shapeDeformationValue->setDecimals(1);
    shapeDeformationValue->setSingleStep(0.1);

    rootDegreeDaysGrowth = new QLabel(tr("degree days root inc [°C]: "));
    rootDegreeDaysGrowthValue = new QLineEdit();
    rootDegreeDaysGrowthValue->setMaximumWidth(rootParametersGroup->width()/5);
    rootDegreeDaysGrowthValue->setValidator(positiveValidator);

    parametersRootDepthLayout->addWidget(rootDepthZero, 0, 0);
    parametersRootDepthLayout->addWidget(rootDepthZeroValue, 0, 1);
    parametersRootDepthLayout->addWidget(rootDepthMax, 1, 0);
    parametersRootDepthLayout->addWidget(rootDepthMaxValue, 1, 1);
    parametersRootDepthLayout->addWidget(rootShape, 2, 0);
    parametersRootDepthLayout->addWidget(rootShapeComboBox, 2, 1);
    parametersRootDepthLayout->addWidget(shapeDeformation, 3, 0);
    parametersRootDepthLayout->addWidget(shapeDeformationValue, 3, 1);
    parametersRootDepthLayout->addWidget(rootDegreeDaysGrowth, 4, 0);
    parametersRootDepthLayout->addWidget(rootDegreeDaysGrowthValue, 4, 1);

    QLabel *irrigationVolume = new QLabel(tr("irrigation quantity [mm]:"));
    irrigationVolumeValue = new QLineEdit();
    irrigationVolumeValue->setText(QLocale().toString(0));
    irrigationVolumeValue->setMaximumWidth(irrigationParametersGroup->width()/5);
    irrigationVolumeValue->setValidator(positiveValidator);
    QLabel *irrigationShift = new QLabel(tr("irrigation shift [days]:"));
    irrigationShiftValue = new QSpinBox();
    irrigationShiftValue->setMaximumWidth(irrigationParametersGroup->width()/5);
    irrigationShiftValue->setMinimum(0);
    irrigationShiftValue->setMaximum(365);
    irrigationShiftValue->setEnabled(false);

    QLabel *degreeDaysStart = new QLabel(tr("deg. days start irrigation [°C]:"));
    degreeDaysStartValue = new QLineEdit();
    degreeDaysStartValue->setMaximumWidth(irrigationParametersGroup->width()/5);
    degreeDaysStartValue->setValidator(positiveValidator);
    degreeDaysStartValue->setEnabled(false);
    QLabel *degreeDaysEnd = new QLabel(tr("deg. days end irrigation [°C]:"));
    degreeDaysEndValue = new QLineEdit();
    degreeDaysEndValue->setMaximumWidth(irrigationParametersGroup->width()/5);
    degreeDaysEndValue->setValidator(positiveValidator);
    degreeDaysEndValue->setEnabled(false);

    parametersIrrigationLayout->addWidget(irrigationVolume, 0, 0);
    parametersIrrigationLayout->addWidget(irrigationVolumeValue, 0, 1);
    parametersIrrigationLayout->addWidget(irrigationShift, 1, 0);
    parametersIrrigationLayout->addWidget(irrigationShiftValue, 1, 1);
    parametersIrrigationLayout->addWidget(degreeDaysStart, 2, 0);
    parametersIrrigationLayout->addWidget(degreeDaysStartValue, 2, 1);
    parametersIrrigationLayout->addWidget(degreeDaysEnd, 3, 0);
    parametersIrrigationLayout->addWidget(degreeDaysEndValue, 3, 1);

    QLabel *psiLeaf = new QLabel(tr("psi leaf [cm]: "));
    psiLeafValue = new QLineEdit();
    psiLeafValue->setMaximumWidth(waterStressParametersGroup->width()/5);
    psiLeafValue->setValidator(positiveValidator);

    QLabel *rawFraction = new QLabel(tr("raw fraction [-]: "));
    rawFractionValue = new QDoubleSpinBox();
    rawFractionValue->setMaximumWidth(waterStressParametersGroup->width()/5);
    rawFractionValue->setMinimum(0);
    rawFractionValue->setMaximum(1);
    rawFractionValue->setDecimals(2);
    rawFractionValue->setSingleStep(0.05);

    QLabel *stressTolerance = new QLabel(tr("stress tolerance [-]: "));
    stressToleranceValue = new QDoubleSpinBox();
    stressToleranceValue->setMaximumWidth(waterStressParametersGroup->width()/5);
    stressToleranceValue->setMinimum(0);
    stressToleranceValue->setMaximum(1);
    stressToleranceValue->setDecimals(2);
    stressToleranceValue->setSingleStep(0.01);

    parametersWaterStressLayout->addWidget(psiLeaf, 0, 0);
    parametersWaterStressLayout->addWidget(psiLeafValue, 0, 1);
    parametersWaterStressLayout->addWidget(rawFraction, 1, 0);
    parametersWaterStressLayout->addWidget(rawFractionValue, 1, 1);
    parametersWaterStressLayout->addWidget(stressTolerance, 2, 0);
    parametersWaterStressLayout->addWidget(stressToleranceValue, 2, 1);

    volWaterContent = new QRadioButton(tr("&volumetric water content [m3 m-3]"));
    degreeSat = new QRadioButton(tr("&degree of saturation [-]"));
    volWaterContent->setChecked(true);
    waterContentLayout->addWidget(volWaterContent);
    waterContentLayout->addWidget(degreeSat);

    nitrogen_NO3 = new QRadioButton(tr("&NO3 - Nitrogen in form of nitrates [g m-2]"));
    nitrogen_NH4 = new QRadioButton(tr("&NH4 - Nitrogen in form of ammonium [g m-2]"));
    nitrogen_humus = new QRadioButton(tr("&Nitrogen in humus [g m-2]"));
    nitrogen_litter = new QRadioButton(tr("Nitrogen in litter [g m-2]"));
    carbon_humus = new QRadioButton(tr("&Carbon in humus [g m-2]"));
    carbon_litter = new QRadioButton(tr("Carbon in litter [g m-2]"));
    nitrogen_NO3->setChecked(true);
    carbonNitrogenLayout->addWidget(nitrogen_NO3);
    carbonNitrogenLayout->addWidget(nitrogen_NH4);
    carbonNitrogenLayout->addWidget(nitrogen_humus);
    carbonNitrogenLayout->addWidget(nitrogen_litter);
    carbonNitrogenLayout->addWidget(carbon_humus);
    carbonNitrogenLayout->addWidget(carbon_litter);

    infoCaseGroup->setLayout(caseInfoLayout);
    infoCropGroup->setLayout(cropInfoLayout);
    infoMeteoGroup->setLayout(meteoInfoLayout);
    infoSoilGroup->setLayout(soilInfoLayout);
    laiParametersGroup->setLayout(parametersLaiLayout);
    rootParametersGroup->setLayout(parametersRootDepthLayout);
    irrigationParametersGroup->setLayout(parametersIrrigationLayout);
    waterStressParametersGroup->setLayout(parametersWaterStressLayout);
    waterContentGroup->setLayout(waterContentLayout);
    carbonNitrogenGroup->setLayout(carbonNitrogenLayout);

    infoLayout->addWidget(infoCaseGroup);
    infoLayout->addWidget(infoMeteoGroup);
    infoLayout->addWidget(infoSoilGroup);
    infoLayout->addWidget(infoCropGroup);
    infoLayout->addWidget(laiParametersGroup);
    infoLayout->addWidget(rootParametersGroup);
    infoLayout->addWidget(irrigationParametersGroup);
    infoLayout->addWidget(waterStressParametersGroup);
    infoLayout->addWidget(waterContentGroup);
    infoLayout->addWidget(carbonNitrogenGroup);

    mainLayout->addLayout(saveButtonLayout);
    mainLayout->addLayout(WidgetLayout);
    mainLayout->setAlignment(Qt::AlignTop);

    WidgetLayout->addLayout(infoLayout);
    tabWidget = new QTabWidget;
    tabLAI = new TabLAI();
    tabRootDepth = new TabRootDepth();
    tabRootDensity = new TabRootDensity();
    tabIrrigation = new TabIrrigation();
    tabWaterContent = new TabWaterContent();
    tabCarbonNitrogen = new TabCarbonNitrogen();

    tabWidget->addTab(tabLAI, tr("LAI development"));
    tabWidget->addTab(tabRootDepth, tr("Root depth"));
    tabWidget->addTab(tabRootDensity, tr("Root density"));
    tabWidget->addTab(tabIrrigation, tr("Irrigation"));
    tabWidget->addTab(tabWaterContent, tr("Water Content"));
    tabWidget->addTab(tabCarbonNitrogen, tr("Carbon Nitrogen"));
    WidgetLayout->addWidget(tabWidget);

    this->setLayout(mainLayout);

    // menu
    QMenuBar* menuBar = new QMenuBar();
    QMenu *fileMenu = new QMenu("File");
    QMenu *editMenu = new QMenu("Edit");
    viewMenu = new QMenu("View Data");
    viewMenu->setEnabled(false);

    menuBar->addMenu(fileMenu);
    menuBar->addMenu(editMenu);
    menuBar->addMenu(viewMenu);
    this->layout()->setMenuBar(menuBar);

    QAction* openProject = new QAction(tr("&Open Project"), this);
    QAction* newProject = new QAction(tr("&New Project"), this);
    QAction* openCropDB = new QAction(tr("&Open dbCrop"), this);
    QAction* openMeteoDB = new QAction(tr("&Open dbMeteo"), this);
    QAction* openSoilDB = new QAction(tr("&Open dbSoil"), this);

    saveChanges = new QAction(tr("&Save Changes"), this);
    saveChanges->setEnabled(false);
    QAction* executeCase = new QAction(tr("&Execute case"), this);

    QAction* newCrop = new QAction(tr("&New Crop"), this);
    QAction* deleteCrop = new QAction(tr("&Delete Crop"), this);
    restoreData = new QAction(tr("&Restore Data"), this);

    fileMenu->addAction(openProject);
    fileMenu->addAction(newProject);
    fileMenu->addSeparator();
    fileMenu->addAction(openCropDB);
    fileMenu->addAction(openMeteoDB);
    fileMenu->addAction(openSoilDB);
    fileMenu->addSeparator();
    fileMenu->addAction(saveChanges);
    fileMenu->addSeparator();
    fileMenu->addAction(executeCase);

    editMenu->addAction(newCrop);
    editMenu->addAction(deleteCrop);
    editMenu->addAction(restoreData);

    viewWeather = new QAction(tr("&Weather"), this);
    viewSoil = new QAction(tr("&Soil"), this);
    viewMenu->addAction(viewWeather);
    viewMenu->addAction(viewSoil);

    isCropChanged = false;
    isOnlyOneYear = false;

    connect(openProject, &QAction::triggered, this, &Criteria1DWidget::on_actionOpenProject);
    connect(newProject, &QAction::triggered, this, &Criteria1DWidget::on_actionNewProject);
    connect(&caseListComboBox, &QComboBox::currentTextChanged, this, &Criteria1DWidget::on_actionChooseCase);

    connect(openCropDB, &QAction::triggered, this, &Criteria1DWidget::on_actionOpenCropDB);
    connect(&cropListComboBox, &QComboBox::currentTextChanged, this, &Criteria1DWidget::on_actionChooseCrop);

    connect(openMeteoDB, &QAction::triggered, this, &Criteria1DWidget::on_actionOpenMeteoDB);
    connect(&meteoListComboBox, &QComboBox::currentTextChanged, this, &Criteria1DWidget::on_actionChooseMeteo);
    connect(&firstYearListComboBox, &QComboBox::currentTextChanged, this, &Criteria1DWidget::on_actionChooseFirstYear);
    connect(&lastYearListComboBox, &QComboBox::currentTextChanged, this, &Criteria1DWidget::on_actionChooseLastYear);

    connect(openSoilDB, &QAction::triggered, this, &Criteria1DWidget::on_actionOpenSoilDB);
    connect(&soilListComboBox, &QComboBox::currentTextChanged, this, &Criteria1DWidget::on_actionChooseSoil);
    connect(irrigationVolumeValue, &QLineEdit::editingFinished, [=](){ this->irrigationVolumeChanged(); });

    connect(volWaterContent, &QRadioButton::toggled, [=](){ this->updateTabWaterContent(); });

    connect(tabWidget, &QTabWidget::currentChanged, [=](int index){ this->tabChanged(index); });

    connect(viewWeather, &QAction::triggered, this, &Criteria1DWidget::on_actionViewWeather);
    connect(viewSoil, &QAction::triggered, this, &Criteria1DWidget::on_actionViewSoil);

    connect(newCrop, &QAction::triggered, this, &Criteria1DWidget::on_actionNewCrop);
    connect(deleteCrop, &QAction::triggered, this, &Criteria1DWidget::on_actionDeleteCrop);
    connect(restoreData, &QAction::triggered, this, &Criteria1DWidget::on_actionRestoreData);

    connect(saveButton, &QPushButton::clicked, this, &Criteria1DWidget::on_actionSave);
    connect(updateButton, &QPushButton::clicked, this, &Criteria1DWidget::on_actionUpdate);

    connect(executeCase, &QAction::triggered, this, &Criteria1DWidget::on_actionExecuteCase);

    //set current tab
    tabChanged(0);

    isRedraw = true;
}


void Criteria1DWidget::on_actionOpenProject()
{
    isRedraw = false;
    QString dataPath, projectPath;

    if (searchDataPath(&dataPath))
        projectPath = dataPath + PATH_PROJECT;
    else
        projectPath = "";

    checkCropUpdate();
    QString projFileName = QFileDialog::getOpenFileName(this, tr("Open Criteria-1D project"), projectPath, tr("Settings files (*.ini)"));

    if (projFileName.isEmpty())
        return;

    myProject.initialize();
    int myResult = myProject.initializeProject(projFileName);
    if (myResult != CRIT1D_OK)
    {
        QMessageBox::critical(nullptr, "Error", myProject.projectError);
        return;
    }

    this->cropListComboBox.blockSignals(true);
    this->soilListComboBox.blockSignals(true);
    this->meteoListComboBox.blockSignals(true);
    this->firstYearListComboBox.blockSignals(true);
    this->lastYearListComboBox.blockSignals(true);

    openCropDB(myProject.dbCropName);
    openSoilDB(myProject.dbSoilName);
    openMeteoDB(myProject.dbMeteoName, false);

    this->cropListComboBox.blockSignals(false);
    this->soilListComboBox.blockSignals(false);
    this->meteoListComboBox.blockSignals(false);
    this->firstYearListComboBox.blockSignals(false);
    this->lastYearListComboBox.blockSignals(false);

    openComputationUnitsDB(myProject.dbComputationUnitsName);

    viewMenu->setEnabled(true);
    if (soilListComboBox.count() == 0)
    {
        viewSoil->setEnabled(false);
    }
    else
    {
        viewSoil->setEnabled(true);
    }
    if (meteoListComboBox.count() == 0)
    {
        viewWeather->setEnabled(false);
    }
    else
    {
        viewWeather->setEnabled(true);
    }

    isRedraw = true;
}


void Criteria1DWidget::on_actionNewProject()
{
    DialogNewProject dialog;
    if (dialog.result() != QDialog::Accepted)
    {
        return;
    }
    else
    {
        QString dataPath;
        QString projectName = dialog.getProjectName();
        projectName = projectName.simplified().remove(' ');
        if (searchDataPath(&dataPath))
        {
            QString completePath = dataPath+PATH_PROJECT+projectName;
            if(!QDir().mkdir(completePath))
            {
                QMessageBox::StandardButton confirm;
                QString msg = "Project " + completePath + " already exists, do you want to overwrite it?";
                confirm = QMessageBox::question(nullptr, "Warning", msg, QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

                if (confirm == QMessageBox::Yes)
                {
                    clearDir(completePath);
                    QDir().mkdir(completePath + "/data");
                }
                else
                {
                    return;
                }
            }
            else
            {
                QDir().mkdir(completePath + "/data");
            }
            // copy template computational units
            if (!QFile::copy(dataPath + PATH_TEMPLATE + "template_comp_units.db",
                             completePath + "/data/" + "comp_units.db"))
            {
                QMessageBox::critical(nullptr, "Copy failed", "Error in copying template_comp_units.db");
                return;
            }
            QString db_soil, db_meteo, db_crop;
            // db soil
            if (dialog.getSoilDbOption() == NEW_DB)
            {
                db_soil = "soil.db";
                if (!QFile::copy(dataPath + PATH_TEMPLATE + "template_soil.db", completePath + "/data/" + db_soil))
                {
                    QMessageBox::critical(nullptr, "Copy failed", "Error in copying template_soil.db");
                    return;
                }
            }
            else if (dialog.getSoilDbOption() == DEFAULT_DB)
            {
                db_soil = "soil_ER_2021.db";
                if (! QFile::copy(dataPath + "SOIL/soil_ER_2021.db", completePath + "/data/" + db_soil))
                {
                    QMessageBox::critical(nullptr, "Copy failed", "Error in copying soil_ER_2021.db");
                    return;
                }
            }
            else if (dialog.getSoilDbOption() == CHOOSE_DB)
            {
                QString soilPath = dialog.getDbSoilCompletePath();
                db_soil = QFileInfo(soilPath).baseName()+".db";
                if (!QFile::copy(soilPath, completePath+"/data/"+db_soil))
                {
                    QMessageBox::critical(nullptr, "Copy failed", "Error in copying " + soilPath);
                    return;
                }
            }
            // db meteo
            if (dialog.getMeteoDbOption() == NEW_DB)
            {
                db_meteo = "meteo.db";
                if (!QFile::copy(dataPath  +PATH_TEMPLATE + "template_meteo.db", completePath + "/data/" + db_meteo))
                {
                    QMessageBox::critical(nullptr, "Copy failed", "Error in copying template_meteo.db");
                    return;
                }
            }
            else if (dialog.getMeteoDbOption() == DEFAULT_DB)
            {
                db_meteo = "meteo.db";
                if (!QFile::copy(dataPath+PATH_PROJECT+"test/data/meteo.db", completePath+"/data/"+db_meteo))
                {
                    QMessageBox::critical(nullptr, "Copy failed", "Error in copying meteo.db");
                    return;
                }
            }
            else if (dialog.getMeteoDbOption() == CHOOSE_DB)
            {
                QString meteoPath = dialog.getDbMeteoCompletePath();
                db_meteo = QFileInfo(meteoPath).baseName() + ".db";
                if (!QFile::copy(meteoPath, completePath + "/data/" + db_meteo))
                {
                    QMessageBox::critical(nullptr, "Copy failed", "Error in copying " + meteoPath);
                    return;
                }
            }
            // db crop
            if (dialog.getCropDbOption() == DEFAULT_DB)
            {
                db_crop = "crop.db";
                if (!QFile::copy(dataPath + PATH_TEMPLATE + "crop_default.db", completePath + "/data/" + "crop.db"))
                {
                    QMessageBox::critical(nullptr, "Copy failed", "Error in copying crop_default.db");
                    return;
                }
            }
            else if (dialog.getCropDbOption() == CHOOSE_DB)
            {
                QString cropPath = dialog.getDbCropCompletePath();
                db_crop = QFileInfo(cropPath).baseName()+".db";
                if (!QFile::copy(cropPath, completePath+"/data/"+db_crop))
                {
                    QMessageBox::critical(nullptr, "Copy failed", "Error in copying " + cropPath);
                    return;
                }
            }

            // write .ini
            QSettings* projectSetting = new QSettings(dataPath+PATH_PROJECT+projectName+"/"+projectName+".ini", QSettings::IniFormat);
            projectSetting->beginGroup("software");
                    projectSetting->setValue("software", "CRITERIA1D");
            projectSetting->endGroup();
            projectSetting->beginGroup("project");
                    projectSetting->setValue("path", "./");
                    projectSetting->setValue("name", projectName);
                    projectSetting->setValue("db_soil", "./data/" + db_soil);
                    projectSetting->setValue("db_meteo", "./data/" + db_meteo);
                    projectSetting->setValue("db_crop", "./data/" + db_crop);
                    projectSetting->setValue("db_comp_units", "./data/comp_units.db");
                    projectSetting->setValue("db_output", "./output/" + projectName + ".db");
            projectSetting->endGroup();
            projectSetting->sync();
        }

        QMessageBox::information(nullptr, "Success!", "project created: " + dataPath + PATH_PROJECT + projectName);
    }
}


void Criteria1DWidget::on_actionOpenCropDB()
{
    checkCropUpdate();

    QString newDbCropName = QFileDialog::getOpenFileName(this, tr("Open crop database"), "", tr("SQLite files (*.db)"));

    if (newDbCropName == "")
        return;
    else
        openCropDB(newDbCropName);
}


void Criteria1DWidget::checkCropUpdate()
{
    if (!myProject.myCase.crop.idCrop.empty())
    {
        if (checkCropIsChanged())
        {
            QString idCropChanged = QString::fromStdString(myProject.myCase.crop.idCrop);
            QMessageBox::StandardButton confirm;
            QString msg = "Do you want to save changes to crop "+ idCropChanged + " ?";
            confirm = QMessageBox::question(nullptr, "Warning", msg, QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

            if (confirm == QMessageBox::Yes)
            {
                if (updateCrop())
                {
                    if (saveCrop())
                    {
                        // already saved
                        isCropChanged = false;
                    }
                }
            }
        }
    }
}


void Criteria1DWidget::openComputationUnitsDB(QString dbComputationUnitsName)
{  
    QString errorStr;
    if (! readComputationUnitList(dbComputationUnitsName, myProject.compUnitList, errorStr))
    {
        QMessageBox::critical(nullptr, "Error in DB Units:", errorStr);
        return;
    }

    // unit list
    this->caseListComboBox.blockSignals(true);
    this->caseListComboBox.clear();
    this->caseListComboBox.blockSignals(false);

    for (unsigned int i = 0; i < myProject.compUnitList.size(); i++)
    {
        this->caseListComboBox.addItem(myProject.compUnitList[i].idCase);
    }
}


void Criteria1DWidget::clearCrop()
{
        myProject.myCase.crop.clear();
        cropFromDB.clear();
}


void Criteria1DWidget::openCropDB(QString newDbCropName)
{
    clearCrop();

    QString errorStr;
    if (! openDbCrop(myProject.dbCrop, newDbCropName, errorStr))
    {
        QMessageBox::critical(nullptr, "Error DB crop", errorStr);
        return;
    }

    // read crop list
    QList<QString> cropStringList;
    if (! getCropIdList(myProject.dbCrop, cropStringList, errorStr))
    {
        QMessageBox::critical(nullptr, "Error!", errorStr);
        return;
    }

    // show crop list
    this->cropListComboBox.clear();
    for (int i = 0; i < cropStringList.size(); i++)
    {
        this->cropListComboBox.addItem(cropStringList[i]);
    }

    saveChanges->setEnabled(true);
    saveButton->setEnabled(true);
    updateButton->setEnabled(true);   
}


void Criteria1DWidget::on_actionOpenMeteoDB()
{
    QString dbMeteoName = QFileDialog::getOpenFileName(this, tr("Open meteo database"), "", tr("SQLite files or XML (*.db *xml)"));
    if (! dbMeteoName.isEmpty())
    {
        if (dbMeteoName.right(3) == "xml")
            myProject.setIsXmlGrid(true);
        else
            myProject.setIsXmlGrid(false);

        openMeteoDB(dbMeteoName, true);
    }
}


void Criteria1DWidget::openMeteoDB(QString dbMeteoName, bool isMenu)
{
    QString errorStr;
    QList<QString> idMeteoList;
    if (myProject.isXmlMeteoGrid())
    {
        if (isMenu)
        {
            if (! myProject.observedMeteoGrid->parseXMLGrid(dbMeteoName, errorStr))
            {
                QMessageBox::critical(nullptr, "Error XML meteo grid", errorStr);
                return;
            }
            if (! myProject.observedMeteoGrid->openDatabase(errorStr, "observed"))
            {
                QMessageBox::critical(nullptr, "Error DB Grid", errorStr);
                return;
            }
        }

        // check daily list
        if (! myProject.observedMeteoGrid->idDailyList(errorStr, idMeteoList))
        {
            QMessageBox::critical(nullptr, "Error daily table list", errorStr);
            return;
        }
    }
    else
    {
        if (isMenu)
        {
            if (! openDbMeteo(dbMeteoName, myProject.dbMeteo, errorStr))
            {
                QMessageBox::critical(nullptr, "Error DB meteo", errorStr);
                return;
            }
        }

        // read id_meteo list
        if (! getMeteoPointList(myProject.dbMeteo, idMeteoList, errorStr))
        {
            QMessageBox::critical(nullptr, "Error!", errorStr);
            return;
        }
    }

    // clear combo box
    meteoListComboBox.clear();
    firstYearListComboBox.clear();
    lastYearListComboBox.clear();
    // show id_meteo list
    for (int i = 0; i < idMeteoList.size(); i++)
    {
        meteoListComboBox.addItem(idMeteoList[i]);
    }

    saveChanges->setEnabled(true);
    saveButton->setEnabled(true);
    updateButton->setEnabled(true);
    viewMenu->setEnabled(true);
    if (soilListComboBox.count() == 0)
    {
        viewSoil->setEnabled(false);
    }
    else
    {
        viewSoil->setEnabled(true);
    }
    if (meteoListComboBox.count() == 0)
    {
        viewWeather->setEnabled(false);
    }
    else
    {
        viewWeather->setEnabled(true);
    }
}


void Criteria1DWidget::on_actionOpenSoilDB()
{
    QString dbSoilName = QFileDialog::getOpenFileName(this, tr("Open soil database"), "", tr("SQLite files (*.db)"));
    if (dbSoilName == "")
        return;
    else
        openSoilDB(dbSoilName);
}


void Criteria1DWidget::openSoilDB(QString dbSoilName)
{
    QString errorStr;
    if (! openDbSoil(dbSoilName, myProject.dbSoil, errorStr))
    {
        QMessageBox::critical(nullptr, "Error!", errorStr);
        return;
    }

    // load default VG parameters
    if (! loadVanGenuchtenParameters(myProject.dbSoil, myProject.texturalClassList, errorStr))
    {
        QMessageBox::critical(nullptr, "Error!", errorStr);
        return;
    }

    // load default Driessen parameters
    if (! loadDriessenParameters(myProject.dbSoil, myProject.texturalClassList, errorStr))
    {
        QMessageBox::critical(nullptr, "Error!", errorStr);
        return;
    }

    // read soil list
    QList<QString> soilStringList;
    if (! getSoilList(myProject.dbSoil, soilStringList, errorStr))
    {
        QMessageBox::critical(nullptr, "Error!", errorStr);
        return;
    }

    // show soil list
    this->soilListComboBox.clear();
    for (int i = 0; i < soilStringList.size(); i++)
    {
        this->soilListComboBox.addItem(soilStringList[i]);
    }
    viewMenu->setEnabled(true);
    if (soilListComboBox.count() == 0)
    {
        viewSoil->setEnabled(false);
    }
    else
    {
        viewSoil->setEnabled(true);
    }
    if (meteoListComboBox.count() == 0)
    {
        viewWeather->setEnabled(false);
    }
    else
    {
        viewWeather->setEnabled(true);
    }
}


void Criteria1DWidget::on_actionExecuteCase()
{
    if (! myProject.isProjectLoaded)
    {
        QMessageBox::warning(nullptr, "Warning", "Open a project before.");
        return;
    }

    if (! myProject.computeUnit(myProject.myCase.unit))
    {
        QMessageBox::critical(nullptr, "Error!", myProject.projectError);
    }
    else
    {
        QMessageBox::warning(nullptr, "Case executed: "+ myProject.myCase.unit.idCase, "Output:\n" + QDir().cleanPath(myProject.dbOutputName));
    }
}


void Criteria1DWidget::on_actionChooseCase()
{
    isRedraw = false;
    FormInfo formInfo;
    formInfo.showInfo("Load case: " + caseListComboBox.currentText());

    bool isFirstCase = true;
    if (! firstYearListComboBox.currentText().isEmpty())
    {
        firstYearListComboBox.blockSignals(true);
        lastYearListComboBox.blockSignals(true);
        isFirstCase = false;
    }

    int index = caseListComboBox.currentIndex();
    QString errorStr;

    myProject.myCase.unit = myProject.compUnitList[unsigned(index)];
    myProject.myCase.fittingOptions.useWaterRetentionData = myProject.myCase.unit.useWaterRetentionData;

    // Read watertable parameters
    if (myProject.myCase.unit.useWaterTableData &&  !myProject.dbWaterTableName.isEmpty() && !myProject.myCase.unit.idWaterTable.isEmpty())
    {
        WaterTableDb wtDataBase = WaterTableDb(myProject.dbWaterTableName, errorStr);
        if (! errorStr.isEmpty())
        {
            QMessageBox::warning(nullptr, "Error!", errorStr);
        }
        if (! wtDataBase.readSingleWaterTableParameters(myProject.myCase.unit.idWaterTable, myProject.myCase.waterTableParameters, errorStr))
        {
            QMessageBox::warning(nullptr, "Error!", errorStr);
        }
    }

    // METEO
    if (myProject.myCase.unit.idMeteo != meteoListComboBox.currentText())
    {
        if (meteoListComboBox.findText(myProject.myCase.unit.idMeteo) == -1)
        {
            errorStr = "Wrong ID_METEO: " + myProject.myCase.unit.idMeteo + "\nIn the ID_CASE: " + caseListComboBox.currentText();
            QMessageBox::critical(nullptr, "Error!", errorStr);
            return;
        }
        else
        {
            meteoListComboBox.setCurrentText(myProject.myCase.unit.idMeteo);
        }
    }
    else if (isFirstCase)
    {
        on_actionChooseMeteo(myProject.myCase.unit.idMeteo);
    }
    else
    {
        updateMeteoPointData();
    }

    // CROP ID
    myProject.myCase.unit.idCrop = getIdCropFromClass(myProject.dbCrop, "crop_class", "id_class", myProject.myCase.unit.idCropClass, errorStr);
    if (myProject.myCase.unit.idCrop == "")
    {
        // it is a single crop, not a crop class
        myProject.myCase.unit.idCrop = myProject.myCase.unit.idCropClass;
    }
    if ( cropListComboBox.findText(myProject.myCase.unit.idCrop) != -1 )
    {
        cropListComboBox.setCurrentText(myProject.myCase.unit.idCrop);
    }
    else
    {
        QMessageBox::critical(nullptr, "Error!", "Wrong ID_CROP: " + myProject.myCase.unit.idCropClass + "\n" + errorStr);
        return;
    }

    // SOIL
    if (myProject.myCase.unit.idSoilNumber != NODATA)
    {
        myProject.myCase.unit.idSoil = getIdSoilString(myProject.dbSoil, myProject.myCase.unit.idSoilNumber, errorStr);
    }
    if (myProject.myCase.unit.idSoil != "")
    {
        if (soilListComboBox.findText(myProject.myCase.unit.idSoil) == -1)
        {
            errorStr = "Wrong ID_SOIL: " + myProject.myCase.unit.idSoil + "\nIn the ID_CASE: " + caseListComboBox.currentText();
            QMessageBox::critical(nullptr, "Error!", errorStr);
            return;
        }
        else
        {
            if (myProject.myCase.unit.idSoil != soilListComboBox.currentText())
            {
                soilListComboBox.setCurrentText(myProject.myCase.unit.idSoil);
            }
            else
            {
                on_actionChooseSoil(myProject.myCase.unit.idSoil);
            }
        }
    }
    else
    {
        QString soilNumber = QString::number(myProject.myCase.unit.idSoilNumber);
        QMessageBox::critical(nullptr, "Error!", "Missing soil nr: " + soilNumber + "\n" + errorStr);
    }

    firstYearListComboBox.blockSignals(false);
    lastYearListComboBox.blockSignals(false);

    formInfo.close();

    isRedraw = true;
    on_actionUpdate();
}


void Criteria1DWidget::on_actionChooseCrop(QString idCrop)
{

    if (idCrop.isEmpty())
    {
        return;
    }
    if (checkCropIsChanged())
    {
        QString idCropChanged = QString::fromStdString(myProject.myCase.crop.idCrop);
        QMessageBox::StandardButton confirm;
        QString msg = "Do you want to save changes to crop "+ idCropChanged + " ?";
        confirm = QMessageBox::question(nullptr, "Warning", msg, QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

        if (confirm == QMessageBox::Yes)
        {
            if (updateCrop())
            {
                if (saveCrop())
                {
                    isCropChanged = false; //already saved
                }
            }
        }
    }

    // clear previous crop
    clearCrop();
    updateCropParam(idCrop);

    if (isRedraw) on_actionUpdate();
}


void Criteria1DWidget::updateCropParam(QString idCrop)
{
    QString errorStr;
    if (!loadCropParameters(myProject.dbCrop, idCrop, myProject.myCase.crop, errorStr))
    {
        if (errorStr.contains("Empty"))
        {
            QMessageBox::information(nullptr, "Warning", errorStr);
        }
        else
        {
            QMessageBox::critical(nullptr, "Error!", errorStr);
            return;
        }
    }

    cropNameValue->setText(QString::fromStdString(myProject.myCase.crop.name));
    cropTypeValue->setText(QString::fromStdString(getCropTypeString(myProject.myCase.crop.type)));

    if (myProject.myCase.crop.isSowingCrop())
    {
        cropSowing.setVisible(true);
        cropCycleMax.setVisible(true);
        cropSowingValue->setValue(myProject.myCase.crop.sowingDoy);
        cropSowingValue->setVisible(true);
        cropCycleMaxValue->setValue(myProject.myCase.crop.plantCycle);
        cropCycleMaxValue->setVisible(true);
    }
    else
    {
        cropSowing.setVisible(false);
        cropCycleMax.setVisible(false);
        cropSowingValue->setVisible(false);
        cropCycleMaxValue->setVisible(false);
    }
    maxKcValue->setText(QLocale().toString(myProject.myCase.crop.kcMax));

    // LAI parameters
    LAIminValue->setValue(myProject.myCase.crop.LAImin);
    LAImaxValue->setValue(myProject.myCase.crop.LAImax);
    if (myProject.myCase.crop.type == TREE)
    {
        LAIgrass->setVisible(true);
        LAIgrassValue->setVisible(true);
        LAIgrassValue->setText(QLocale().toString(myProject.myCase.crop.LAIgrass));
    }
    else
    {
        LAIgrass->setVisible(false);
        LAIgrassValue->setVisible(false);
    }
    thermalThresholdValue->setText(QLocale().toString(myProject.myCase.crop.thermalThreshold));
    upperThermalThresholdValue->setText(QLocale().toString(myProject.myCase.crop.upperThermalThreshold));
    degreeDaysEmergenceValue->setText(QString::number(myProject.myCase.crop.degreeDaysEmergence));
    degreeDaysLaiIncreaseValue->setText(QString::number(myProject.myCase.crop.degreeDaysIncrease));
    degreeDaysLaiDecreaseValue->setText(QString::number(myProject.myCase.crop.degreeDaysDecrease));
    LAIcurveAValue->setText(QLocale().toString(myProject.myCase.crop.LAIcurve_a));
    LAIcurveBValue->setText(QLocale().toString(myProject.myCase.crop.LAIcurve_b));

    // root parameters
    rootDepthZeroValue->setText(QLocale().toString(myProject.myCase.crop.roots.rootDepthMin));
    rootDepthMaxValue->setText(QLocale().toString(myProject.myCase.crop.roots.rootDepthMax));
    shapeDeformationValue->setValue(myProject.myCase.crop.roots.shapeDeformation);
    rootShapeComboBox->setCurrentText(QString::fromStdString(root::getRootDistributionTypeString(myProject.myCase.crop.roots.rootShape)));

    if (myProject.myCase.crop.isRootStatic())
    {
        rootDegreeDaysGrowth->setVisible(false);
        rootDegreeDaysGrowthValue->setVisible(false);
    }
    else
    {
        rootDegreeDaysGrowth->setVisible(true);
        rootDegreeDaysGrowthValue->setVisible(true);
        rootDegreeDaysGrowthValue->setText(QString::number(myProject.myCase.crop.roots.degreeDaysRootGrowth));
    }
    // irrigation parameters
    irrigationVolumeValue->setText(QLocale().toString(myProject.myCase.crop.irrigationVolume));
    if (irrigationVolumeValue->text() == "0")
    {
        irrigationShiftValue->setValue(0);
        irrigationShiftValue->setEnabled(false);
        degreeDaysStartValue->setText(nullptr);
        degreeDaysStartValue->setEnabled(false);
        degreeDaysEndValue->setText(nullptr);
        degreeDaysEndValue->setEnabled(false);
    }
    else if (QLocale().toDouble(irrigationVolumeValue->text()) > 0)
    {
        irrigationShiftValue->setEnabled(true);
        irrigationShiftValue->setValue(myProject.myCase.crop.irrigationShift);
        degreeDaysStartValue->setEnabled(true);
        degreeDaysStartValue->setText(QString::number(myProject.myCase.crop.degreeDaysStartIrrigation));
        degreeDaysEndValue->setEnabled(true);
        degreeDaysEndValue->setText(QString::number(myProject.myCase.crop.degreeDaysEndIrrigation));
    }
    // water stress parameters
    psiLeafValue->setText(QString::number(myProject.myCase.crop.psiLeaf));
    rawFractionValue->setValue(myProject.myCase.crop.fRAW);
    stressToleranceValue->setValue(myProject.myCase.crop.stressTolerance);

    cropFromDB = myProject.myCase.crop;
}


void Criteria1DWidget::on_actionChooseMeteo(QString idMeteo)
{
    if (idMeteo.isEmpty())
        return;

    // clear prev year list
    this->firstYearListComboBox.blockSignals(true);
    this->lastYearListComboBox.blockSignals(true);
    this->firstYearListComboBox.clear();
    this->lastYearListComboBox.clear();
    this->yearList.clear();
    this->firstYearListComboBox.blockSignals(false);

    myProject.myCase.meteoPoint.setId(idMeteo.toStdString());
    QString errorStr;

    if (myProject.isXmlMeteoGrid())
    {
        if (! myProject.observedMeteoGrid->loadIdMeteoProperties(idMeteo, errorStr))
        {
            QMessageBox::critical(nullptr, "Error load properties DB Grid", errorStr);
            return;
        }

        double lat;
        if (! myProject.observedMeteoGrid->meteoGrid()->getLatFromId(idMeteo.toStdString(), &lat))
        {
            errorStr = "Missing  meteo cell: " + idMeteo;
            return;
        }
        myProject.myCase.meteoPoint.latitude = lat;

        meteoTableName = myProject.observedMeteoGrid->tableDaily().prefix + idMeteo + myProject.observedMeteoGrid->tableDaily().postFix;
        if (!myProject.observedMeteoGrid->getYearList(errorStr, idMeteo, &yearList))
        {
            QMessageBox::critical(nullptr, "Error!", errorStr);
            return;
        }

        int pos = 0;
        if (myProject.observedMeteoGrid->gridStructure().isFixedFields())
        {
            QString fieldTmin = myProject.observedMeteoGrid->getDailyVarField(dailyAirTemperatureMin);
            QString fieldTmax = myProject.observedMeteoGrid->getDailyVarField(dailyAirTemperatureMax);
            QString fieldPrec = myProject.observedMeteoGrid->getDailyVarField(dailyPrecipitation);

            // last year can be incomplete
            for (int i = 0; i<yearList.size()-1; i++)
            {
                if (! checkYearMeteoGridFixedFields(myProject.dbMeteo, meteoTableName,
                                                   myProject.observedMeteoGrid->tableDaily().fieldTime,
                                                   fieldTmin, fieldTmax, fieldPrec, yearList[i], errorStr))
                {
                    yearList.removeAt(pos);
                    i--;
                }
                else
                {
                    pos++;
                }
            }

            // store last Date
            getLastDateGrid(myProject.dbMeteo, meteoTableName, myProject.observedMeteoGrid->tableDaily().fieldTime,
                            yearList[yearList.size()-1], myProject.lastSimulationDate, errorStr);
        }
        else
        {
            int varCodeTmin = myProject.observedMeteoGrid->getDailyVarCode(dailyAirTemperatureMin);
            int varCodeTmax = myProject.observedMeteoGrid->getDailyVarCode(dailyAirTemperatureMax);
            int varCodePrec = myProject.observedMeteoGrid->getDailyVarCode(dailyPrecipitation);
            if (varCodeTmin == NODATA || varCodeTmax == NODATA || varCodePrec == NODATA)
            {
                errorStr = "Variable not existing";
                QMessageBox::critical(nullptr, "Error!", errorStr);
                return;
            }

            // last year can be incomplete
            for (int i = 0; i < yearList.size()-1; i++)
            {
                    if (! checkYearMeteoGrid(myProject.observedMeteoGrid->db(), meteoTableName,
                                        myProject.observedMeteoGrid->tableDaily().fieldTime,
                                        varCodeTmin, varCodeTmax, varCodePrec, yearList[i], errorStr))
                    {
                        yearList.removeAt(pos);
                        i--;
                    }
                    else
                    {
                        pos++;
                    }
             }

            // store last Date
            getLastDateGrid(myProject.dbMeteo, meteoTableName, myProject.observedMeteoGrid->tableDaily().fieldTime,
                             yearList[yearList.size()-1], myProject.lastSimulationDate, errorStr);
        }
    }
    else
    {
        QString lat,lon;
        if (getLatLonFromIdMeteo(myProject.dbMeteo, idMeteo, lat, lon, errorStr))
        {
            myProject.myCase.meteoPoint.latitude = lat.toDouble();
        }

        meteoTableName = getTableNameFromIdMeteo(myProject.dbMeteo, idMeteo, errorStr);
        if (meteoTableName.isEmpty())
        {
            errorStr = "Missing  table name for idMeteo: " + idMeteo;
            return;
        }

        if (! getYearList(myProject.dbMeteo, meteoTableName, yearList, errorStr))
        {
            QMessageBox::critical(nullptr, "Error!", "Error loading meteo data: " + meteoTableName + "\n" + errorStr);
            return;
        }

        int pos = 0;

        // last year can be incomplete
        for (int i = 0; i < yearList.size()-1; i++)
        {
            if (! checkYearDbMeteo(myProject.dbMeteo, meteoTableName, yearList[i], errorStr))
            {
                yearList.removeAt(pos);
                i--;
            }
            else
            {
                pos++;
            }
        }
        // store last Date
        getLastDateDbMeteo(myProject.dbMeteo, meteoTableName, yearList[yearList.size()-1], myProject.lastSimulationDate, errorStr);
    }

    if (yearList.size() == 1)
    {
        isOnlyOneYear = true;
        yearList.insert(0, QString::number(yearList[0].toInt()-1));
    }
    else
    {
        isOnlyOneYear = false;
    }

    // add year if exists previous year
    for (int i = 1; i < yearList.size(); i++)
    {
        if (yearList[i].toInt() == yearList[i-1].toInt()+1)
        {
            firstYearListComboBox.addItem(yearList[i]);
        }
    }

    this->lastYearListComboBox.blockSignals(false);
}


void Criteria1DWidget::on_actionChooseFirstYear(QString year)
{
    this->lastYearListComboBox.blockSignals(true);
    this->lastYearListComboBox.clear();

    // add first year
    this->lastYearListComboBox.addItem(year);
    int index = yearList.indexOf(year);

    // add consecutive valid years
    for (int i = index+1; i<yearList.size(); i++)
    {
        if (yearList[i].toInt() == yearList[i-1].toInt()+1)
        {
            this->lastYearListComboBox.addItem(yearList[i]);
        }
        else
        {
            break;
        }
    }

    updateMeteoPointData();
    this->lastYearListComboBox.blockSignals(false);
}


void Criteria1DWidget::on_actionChooseLastYear(QString year)
{
    if (year.toInt() - this->firstYearListComboBox.currentText().toInt() > MAX_YEARS)
    {
        QString msg = "Period is too long: maximum " + QString::number(MAX_YEARS) + " years";
        QMessageBox::information(nullptr, "Error", msg);
        int maxYear = this->firstYearListComboBox.currentText().toInt() + MAX_YEARS;
        this->lastYearListComboBox.setCurrentText(QString::number(maxYear));
        return;
    }

    updateMeteoPointData();
}


bool Criteria1DWidget::updateMeteoPointData()
{
    QString errorStr;

    int firstYear = firstYearListComboBox.currentText().toInt() - 1;
    int lastYear = lastYearListComboBox.currentText().toInt();

    // add one year of data (if available) for watertable
    if (myProject.myCase.unit.useWaterTableData && myProject.myCase.waterTableParameters.isLoaded)
    {
        int firstAvailableYear = firstYearListComboBox.itemText(0).toInt() - 1;
        if (firstYear > firstAvailableYear)
        {
            firstYear--;
        }
    }

    // initialize meteoPoint with all the required years
    QDate firstDate(firstYear, 1, 1);
    QDate lastDate(lastYear, 12, 31);
    unsigned int numberDays = firstDate.daysTo(lastDate) + 1;

    myProject.myCase.meteoPoint.initializeObsDataD(numberDays, getCrit3DDate(firstDate));

    if (myProject.isXmlMeteoGrid())
    {
        unsigned row, col;
        if (! myProject.observedMeteoGrid->meteoGrid()->findMeteoPointFromId(&row, &col, myProject.myCase.meteoPoint.id))
        {
            errorStr = "Missing observed meteo cell";
            QMessageBox::critical(nullptr, "Error!", errorStr);
            return false;
        }

        if (! myProject.observedMeteoGrid->gridStructure().isFixedFields())
        {
            if (! myProject.observedMeteoGrid->loadGridDailyData(errorStr, QString::fromStdString(myProject.myCase.meteoPoint.id),
                                                                firstDate, lastDate) )
            {
                errorStr = "Missing observed data";
                QMessageBox::critical(nullptr, "Error!", errorStr);
                return false;
            }
        }
        else
        {
            if (! myProject.observedMeteoGrid->loadGridDailyDataFixedFields(errorStr, QString::fromStdString(myProject.myCase.meteoPoint.id),
                                                                           firstDate, lastDate) )
            {
                errorStr = "Missing observed data";
                QMessageBox::critical(nullptr, "Error!", errorStr);
                return false;
            }
        }

        float tmin, tmax, tavg, prec, waterDepth;

        for (unsigned int i = 0; i < numberDays; i++)
        {
            Crit3DDate myDate = getCrit3DDate(firstDate.addDays(i));
            tmin = myProject.observedMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(myDate, dailyAirTemperatureMin);
            myProject.myCase.meteoPoint.setMeteoPointValueD(myDate, dailyAirTemperatureMin, tmin);

            tmax = myProject.observedMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(myDate, dailyAirTemperatureMax);
            myProject.myCase.meteoPoint.setMeteoPointValueD(myDate, dailyAirTemperatureMax, tmax);

            tavg = myProject.observedMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(myDate, dailyAirTemperatureAvg);
            myProject.myCase.meteoPoint.setMeteoPointValueD(myDate, dailyAirTemperatureAvg, tavg);

            prec = myProject.observedMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(myDate, dailyPrecipitation);
            myProject.myCase.meteoPoint.setMeteoPointValueD(myDate, dailyPrecipitation, prec);

            waterDepth = myProject.observedMeteoGrid->meteoGrid()->meteoPointPointer(row, col)->getMeteoPointValueD(myDate, dailyWaterTableDepth);
            myProject.myCase.meteoPoint.setMeteoPointValueD(myDate, dailyWaterTableDepth, waterDepth);
        }

        if (isOnlyOneYear)
        {
            // copy values to previous year
            Crit3DDate myDate = Crit3DDate(lastYear, 1, 1);
            Crit3DDate prevDate = Crit3DDate(firstYear, 1, 1);
            for (int i = 0; i < 365; i++)
            {
                myProject.myCase.meteoPoint.setMeteoPointValueD(prevDate, dailyAirTemperatureMin, myProject.myCase.meteoPoint.getMeteoPointValueD(myDate, dailyAirTemperatureMin));
                myProject.myCase.meteoPoint.setMeteoPointValueD(prevDate, dailyAirTemperatureMax, myProject.myCase.meteoPoint.getMeteoPointValueD(myDate, dailyAirTemperatureMax));
                myProject.myCase.meteoPoint.setMeteoPointValueD(prevDate, dailyAirTemperatureAvg, myProject.myCase.meteoPoint.getMeteoPointValueD(myDate, dailyAirTemperatureAvg));
                myProject.myCase.meteoPoint.setMeteoPointValueD(prevDate, dailyPrecipitation, myProject.myCase.meteoPoint.getMeteoPointValueD(myDate, dailyPrecipitation));
                myProject.myCase.meteoPoint.setMeteoPointValueD(prevDate, dailyWaterTableDepth, myProject.myCase.meteoPoint.getMeteoPointValueD(myDate, dailyWaterTableDepth));
                ++myDate;
                ++prevDate;
            }
            // todo leap year
        }
    }
    else
    {
        if (isOnlyOneYear)
        {
            if (! fillDailyTempPrecCriteria1D(myProject.dbMeteo, meteoTableName, myProject.myCase.meteoPoint, lastYear, errorStr))
            {
                QMessageBox::critical(nullptr, "Error! ", errorStr + " year: " + QString::number(lastYear));
                return false;
            }
            // copy values to previous year
            Crit3DDate myDate = Crit3DDate(lastYear, 1, 1);
            Crit3DDate prevDate = Crit3DDate(firstYear, 1, 1);
            for (int i = 0; i < 365; i++)
            {
                myProject.myCase.meteoPoint.setMeteoPointValueD(prevDate, dailyAirTemperatureMin, myProject.myCase.meteoPoint.getMeteoPointValueD(myDate, dailyAirTemperatureMin));
                myProject.myCase.meteoPoint.setMeteoPointValueD(prevDate, dailyAirTemperatureMax, myProject.myCase.meteoPoint.getMeteoPointValueD(myDate, dailyAirTemperatureMax));
                myProject.myCase.meteoPoint.setMeteoPointValueD(prevDate, dailyAirTemperatureAvg, myProject.myCase.meteoPoint.getMeteoPointValueD(myDate, dailyAirTemperatureAvg));
                myProject.myCase.meteoPoint.setMeteoPointValueD(prevDate, dailyPrecipitation, myProject.myCase.meteoPoint.getMeteoPointValueD(myDate, dailyPrecipitation));
                myProject.myCase.meteoPoint.setMeteoPointValueD(prevDate, dailyWaterTableDepth, myProject.myCase.meteoPoint.getMeteoPointValueD(myDate, dailyWaterTableDepth));
                ++myDate;
                ++prevDate;
            }
            // todo leap year
        }
        else
        {
            for (int year = firstYear; year <= lastYear; year++)
            {
                if (! fillDailyTempPrecCriteria1D(myProject.dbMeteo, meteoTableName, myProject.myCase.meteoPoint, year, errorStr))
                {
                    QMessageBox::critical(nullptr, "Error! ", errorStr + " year: " + QString::number(year));
                    return false;
                }
            }
        }
    }

    // fill water table
    if (myProject.myCase.unit.useWaterTableData && myProject.myCase.waterTableParameters.isLoaded)
    {
        myProject.myCase.fillWaterTableData();
    }

    // update simulation
    if (! myProject.myCase.crop.idCrop.empty())
    {
        on_actionUpdate();
    }

    return true;
}


void Criteria1DWidget::on_actionChooseSoil(QString soilCode)
{
    if (soilCode.isEmpty())
        return;

    myProject.myCase.mySoil.cleanSoil();

    QString errorStr;
    if (! myProject.setSoil(soilCode, errorStr))
    {
        QMessageBox::critical(nullptr, "Error!", errorStr);
        return;
    }

    if (tabWidget->currentIndex() != 0)
    {
        if (isRedraw) on_actionUpdate();
    }
}


void Criteria1DWidget::on_actionDeleteCrop()
{
    if (cropListComboBox.currentText().isEmpty())
    {
        QString msg = "Select the crop to be deleted.";
        QMessageBox::information(nullptr, "Warning", msg);
        return;
    }

    QMessageBox::StandardButton confirm;
    QString msg = "Are you sure you want to delete " + cropListComboBox.currentText() + " ?";
    confirm = QMessageBox::question(nullptr, "Warning", msg, QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    QString errorStr;

    if (confirm == QMessageBox::Yes)
    {
        if (deleteCropData(myProject.dbCrop, cropListComboBox.currentText(), errorStr))
        {
            cropListComboBox.removeItem(cropListComboBox.currentIndex());
        }
    }
}


void Criteria1DWidget::on_actionRestoreData()
{
    if (checkCropIsChanged())
    {
        myProject.myCase.crop = cropFromDB;
        updateCropParam(QString::fromStdString(myProject.myCase.crop.idCrop));
    }
}


void Criteria1DWidget::on_actionSave()
{
    QMessageBox::StandardButton confirm;
    QString msg = "Are you sure you want to save "+cropListComboBox.currentText()+" ?";
    confirm = QMessageBox::question(nullptr, "Warning", msg, QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    if (confirm == QMessageBox::Yes)
    {
        if (updateCrop())
        {
            if (saveCrop())
            {
                isCropChanged = false;
            }
        }
    }
}


bool Criteria1DWidget::saveCrop()
{
    QString errorStr;
    if (! updateCropLAIparam(myProject.dbCrop, myProject.myCase.crop, errorStr)
            || !updateCropRootparam(myProject.dbCrop, myProject.myCase.crop, errorStr)
            || !updateCropIrrigationparam(myProject.dbCrop, myProject.myCase.crop, errorStr) )
    {
        QMessageBox::critical(nullptr, "Update param failed!", errorStr);
        return false;
    }

    cropFromDB = myProject.myCase.crop;

    return true;
}


void Criteria1DWidget::on_actionUpdate()
{
    if (! updateCrop())
    {
        // something is wrong
        return;
    }
    if (! firstYearListComboBox.currentText().isEmpty())
    {
        if (tabWidget->currentIndex() == 0)
        {
            updateTabLAI();
        }
        else
        {
            if ((! myProject.myCase.mySoil.code.empty()) && isRedraw)
            {
                if (tabWidget->currentIndex() == 1)
                {
                    updateTabRootDepth();
                }
                if (tabWidget->currentIndex() == 2)
                {
                    updateTabRootDensity();
                }
                if (tabWidget->currentIndex() == 3)
                {
                    updateTabIrrigation();
                }
                if (tabWidget->currentIndex() == 4)
                {
                    updateTabWaterContent();
                }
                if (tabWidget->currentIndex() == 5)
                {
                    updateTabCarbonNitrogen();
                }
            }
        }
    }
}


bool Criteria1DWidget::updateCrop()
{
    if (myProject.myCase.crop.idCrop.empty())
    {
        return false;
    }

    myProject.myCase.crop.type = getCropType(cropTypeValue->text().toStdString());
    // sowing
    if (cropSowing.isVisible())
    {
        myProject.myCase.crop.sowingDoy = cropSowingValue->value();
        myProject.myCase.crop.plantCycle = cropCycleMaxValue->value();
    }
    else
    {
        myProject.myCase.crop.sowingDoy = NODATA;
        myProject.myCase.crop.plantCycle = NODATA;
    }

    myProject.myCase.crop.kcMax = QLocale().toDouble(maxKcValue->text());

    // LAI
    myProject.myCase.crop.LAImin = LAIminValue->value();
    myProject.myCase.crop.LAImax = LAImaxValue->value();
    if (myProject.myCase.crop.type == TREE)
    {
        myProject.myCase.crop.LAIgrass = QLocale().toDouble(LAIgrassValue->text());
    }
    else
    {
        myProject.myCase.crop.LAIgrass = NODATA;
    }
    myProject.myCase.crop.thermalThreshold = QLocale().toDouble(thermalThresholdValue->text());
    myProject.myCase.crop.upperThermalThreshold = QLocale().toDouble(upperThermalThresholdValue->text());
    myProject.myCase.crop.LAIcurve_a = QLocale().toDouble(LAIcurveAValue->text());
    myProject.myCase.crop.LAIcurve_b = QLocale().toDouble(LAIcurveBValue->text());

    myProject.myCase.crop.degreeDaysEmergence = degreeDaysEmergenceValue->text().toDouble();
    myProject.myCase.crop.degreeDaysIncrease = degreeDaysLaiIncreaseValue->text().toDouble();
    myProject.myCase.crop.degreeDaysDecrease = degreeDaysLaiDecreaseValue->text().toDouble();

    // root
    myProject.myCase.crop.roots.rootDepthMin = QLocale().toDouble(rootDepthZeroValue->text());
    myProject.myCase.crop.roots.rootDepthMax = QLocale().toDouble(rootDepthMaxValue->text());
    myProject.myCase.crop.roots.shapeDeformation = shapeDeformationValue->value();
    myProject.myCase.crop.roots.rootShape = root::getRootDistributionTypeFromString(rootShapeComboBox->currentText().toStdString());
    if (myProject.myCase.crop.isRootStatic())
    {
        myProject.myCase.crop.roots.degreeDaysRootGrowth = NODATA;
    }
    else
    {
        myProject.myCase.crop.roots.degreeDaysRootGrowth = rootDegreeDaysGrowthValue->text().toInt();
    }

    // irrigation
    QString errorStr;
    if (irrigationVolumeValue->text().isEmpty())
    {
        errorStr = "irrigation Volume is NULL, insert a valid value";
        QMessageBox::critical(nullptr, "Error irrigation update", errorStr);
        return false;
    }
    else if (irrigationVolumeValue->text() == "0")
    {
        myProject.myCase.crop.irrigationVolume = 0;
        myProject.myCase.crop.irrigationShift = NODATA;
        myProject.myCase.crop.degreeDaysStartIrrigation = NODATA;
        myProject.myCase.crop.degreeDaysEndIrrigation = NODATA;
    }
    else if (QLocale().toDouble(irrigationVolumeValue->text()) > 0)
    {
        if (irrigationShiftValue->value() == 0)
        {
            errorStr = "irrigation shift sould be > 0";
            QMessageBox::critical(nullptr, "Error irrigation update", errorStr);
            return false;
        }
        if (degreeDaysStartValue->text().isEmpty() || degreeDaysEndValue->text().isEmpty())
        {
            errorStr = "irrigation degree days is NULL, insert a valid value";
            QMessageBox::critical(nullptr, "Error irrigation update", errorStr);
            return false;
        }
        myProject.myCase.crop.irrigationVolume = QLocale().toDouble(irrigationVolumeValue->text());
        myProject.myCase.crop.irrigationShift = irrigationShiftValue->value();
        myProject.myCase.crop.degreeDaysStartIrrigation = degreeDaysStartValue->text().toInt();
        myProject.myCase.crop.degreeDaysEndIrrigation = degreeDaysEndValue->text().toInt();
    }

    // water stress
    myProject.myCase.crop.psiLeaf = psiLeafValue->text().toInt();
    myProject.myCase.crop.fRAW = rawFractionValue->value();
    myProject.myCase.crop.stressTolerance = stressToleranceValue->value();

    isCropChanged = true;

    return true;
}


void Criteria1DWidget::on_actionNewCrop()
{
    if (! myProject.dbCrop.isOpen())
    {
        QString msg = "Open a Db Crop";
        QMessageBox::information(nullptr, "Warning", msg);
        return;
    }

    Crit3DCrop* newCrop = new Crit3DCrop();
    DialogNewCrop dialog(&(myProject.dbCrop), newCrop);
    if (dialog.result() == QDialog::Accepted)
    {
        // TODO write newCrop on Db
    }

    delete newCrop;
}


void Criteria1DWidget::updateTabLAI()
{
    if (!myProject.myCase.crop.idCrop.empty() && !myProject.myCase.meteoPoint.id.empty())
    {
        tabLAI->computeLAI(&(myProject.myCase.crop), &(myProject.myCase.meteoPoint),
                           firstYearListComboBox.currentText().toInt(),
                           lastYearListComboBox.currentText().toInt(),
                           myProject.lastSimulationDate, myProject.myCase.soilLayers);
    }
}


void Criteria1DWidget::updateTabRootDepth()
{
    if (myProject.isProjectLoaded)
    {
        tabRootDepth->computeRootDepth(myProject.myCase.crop, myProject.myCase.meteoPoint,
                                       firstYearListComboBox.currentText().toInt(),
                                       lastYearListComboBox.currentText().toInt(),
                                       myProject.lastSimulationDate, myProject.myCase.soilLayers);
    }
}

void Criteria1DWidget::updateTabRootDensity()
{
    if (myProject.isProjectLoaded)
    {
        tabRootDensity->computeRootDensity(myProject,
                                           firstYearListComboBox.currentText().toInt(),
                                           lastYearListComboBox.currentText().toInt());
    }
}

void Criteria1DWidget::updateTabIrrigation()
{
    if (myProject.isProjectLoaded)
    {
        tabIrrigation->computeIrrigation(myProject.myCase,
                                         firstYearListComboBox.currentText().toInt(),
                                         lastYearListComboBox.currentText().toInt(),
                                         myProject.lastSimulationDate);
    }
}

void Criteria1DWidget::updateTabWaterContent()
{
    if (myProject.isProjectLoaded)
    {
        tabWaterContent->computeWaterContent(myProject.myCase,
                                             firstYearListComboBox.currentText().toInt(),
                                             lastYearListComboBox.currentText().toInt(),
                                             myProject.lastSimulationDate, volWaterContent->isChecked());
    }
}

void Criteria1DWidget::updateTabCarbonNitrogen()
{
    if (myProject.isProjectLoaded)
    {
        carbonNitrogenVariable currentVar = NO3;
        if (this->nitrogen_NH4->isChecked()) currentVar = NH4;
        if (this->nitrogen_humus->isChecked()) currentVar = N_HUMUS;
        if (this->nitrogen_litter->isChecked()) currentVar = N_LITTER;
        if (this->carbon_humus->isChecked()) currentVar = C_HUMUS;
        if (this->carbon_litter->isChecked()) currentVar = C_LITTER;

        tabCarbonNitrogen->computeCarbonNitrogen(myProject, currentVar,
                                             firstYearListComboBox.currentText().toInt(),
                                             lastYearListComboBox.currentText().toInt());
    }
}


void Criteria1DWidget::tabChanged(int index)
{
    if (index == 0) //LAI tab
    {
        rootParametersGroup->hide();
        irrigationParametersGroup->hide();
        waterStressParametersGroup->hide();
        waterContentGroup->hide();
        carbonNitrogenGroup->hide();
        laiParametersGroup->setVisible(true);
        updateTabLAI();

    }
    else if(index == 1) //root depth tab
    {
        laiParametersGroup->hide();
        irrigationParametersGroup->hide();
        waterStressParametersGroup->hide();
        waterContentGroup->hide();
        carbonNitrogenGroup->hide();

        rootParametersGroup->setVisible(true);
        if (! myProject.isProjectLoaded)
        {
            QString msg = "Open a project before";
            QMessageBox::information(nullptr, "Warning", msg);
            return;
        }
        updateTabRootDepth();
    }
    else if(index == 2) //root density tab
    {
        laiParametersGroup->hide();
        irrigationParametersGroup->hide();
        waterStressParametersGroup->hide();
        waterContentGroup->hide();
        carbonNitrogenGroup->hide();

        rootParametersGroup->setVisible(true);
        if (! myProject.isProjectLoaded)
        {
            QString msg = "Open a project before";
            QMessageBox::information(nullptr, "Warning", msg);
            return;
        }
        updateTabRootDensity();
    }
    else if(index == 3) //irrigation tab
    {
        laiParametersGroup->hide();
        rootParametersGroup->hide();
        waterContentGroup->hide();
        carbonNitrogenGroup->hide();

        irrigationParametersGroup->setVisible(true);
        waterStressParametersGroup->setVisible(true);

        if (! myProject.isProjectLoaded)
        {
            QString msg = "Open a project before";
            QMessageBox::information(nullptr, "Warning", msg);
            return;
        }
        updateTabIrrigation();
    }
    else if(index == 4) //water content tab
    {
        laiParametersGroup->hide();
        rootParametersGroup->hide();
        irrigationParametersGroup->hide();
        waterStressParametersGroup->hide();
        carbonNitrogenGroup->hide();

        waterContentGroup->setVisible(true);

        if (! myProject.isProjectLoaded)
        {
            QString msg = "Open a project before";
            QMessageBox::information(nullptr, "Warning", msg);
            return;
        }
        updateTabWaterContent();
    }
    else if(index == 5) // carbon nitrogen tab
    {
        laiParametersGroup->hide();
        rootParametersGroup->hide();
        irrigationParametersGroup->hide();
        waterStressParametersGroup->hide();
        waterContentGroup->hide();

        carbonNitrogenGroup->setVisible(true);
        if (! myProject.isProjectLoaded)
        {
            QString msg = "Open a project before";
            QMessageBox::information(nullptr, "Warning", msg);
            return;
        }
        updateTabCarbonNitrogen();
    }
}


bool Criteria1DWidget::checkCropIsChanged()
{
    // check all editable fields
    if (myProject.myCase.crop.idCrop.empty())
    {
        isCropChanged = false;
        return isCropChanged;
    }

    if(cropSowingValue->isVisible())
    {
        if (cropFromDB.sowingDoy != cropSowingValue->value() || cropFromDB.plantCycle != cropCycleMaxValue->value())
        {
            isCropChanged = true;
            return isCropChanged;
        }
    }
    // LAI
    if (cropFromDB.LAImin != LAIminValue->value() || cropFromDB.LAImax != LAImaxValue->value())
    {
        isCropChanged = true;
        return isCropChanged;

    }
    if (cropFromDB.type == TREE && cropFromDB.LAIgrass != QLocale().toDouble(LAIgrassValue->text()))
    {
        isCropChanged = true;
        return isCropChanged;
    }

    // degree days
    if ( ! isEqual(cropFromDB.thermalThreshold, QLocale().toDouble(thermalThresholdValue->text()))
            || ! isEqual(cropFromDB.upperThermalThreshold, QLocale().toDouble(upperThermalThresholdValue->text()))
            || ! isEqual(cropFromDB.LAIcurve_a, QLocale().toDouble(LAIcurveAValue->text()))
            || ! isEqual(cropFromDB.LAIcurve_b, QLocale().toDouble(LAIcurveBValue->text()))
            || cropFromDB.degreeDaysEmergence != degreeDaysEmergenceValue->text().toDouble()
            || cropFromDB.degreeDaysIncrease != degreeDaysLaiIncreaseValue->text().toDouble()
            || cropFromDB.degreeDaysDecrease != degreeDaysLaiDecreaseValue->text().toDouble() )
    {
        isCropChanged = true;
        return isCropChanged;
    }

    // roots
    if(! isEqual(cropFromDB.roots.rootDepthMin, QLocale().toDouble(rootDepthZeroValue->text()))
            || ! isEqual(cropFromDB.roots.rootDepthMax, QLocale().toDouble(rootDepthMaxValue->text()))
            || ! isEqual(cropFromDB.roots.shapeDeformation, shapeDeformationValue->value())
            || cropFromDB.roots.rootShape != root::getRootDistributionTypeFromString(rootShapeComboBox->currentText().toStdString()))
    {
        isCropChanged = true;
        return isCropChanged;
    }
    if (! cropFromDB.isRootStatic())
    {
        if ( cropFromDB.roots.degreeDaysRootGrowth != rootDegreeDaysGrowthValue->text().toInt() )
        {
            isCropChanged = true;
            return isCropChanged;
        }
    }

    // water needs
    if( ! isEqual(cropFromDB.kcMax, QLocale().toDouble(maxKcValue->text()))
       || ! (cropFromDB.psiLeaf == psiLeafValue->text().toInt())
       || ! isEqual(cropFromDB.fRAW, rawFractionValue->value())
       || ! isEqual(cropFromDB.stressTolerance, stressToleranceValue->value()) )
    {
        isCropChanged = true;
        return isCropChanged;
    }

    // irrigation parameters
    if(irrigationShiftValue->isVisible())
    {
        if(! isEqual(cropFromDB.irrigationVolume, QLocale().toDouble(irrigationVolumeValue->text())) )
        {
            isCropChanged = true;
            return isCropChanged;
        }
        if (QLocale().toDouble(irrigationVolumeValue->text()) > 0)
        {
            if ( cropFromDB.irrigationShift != irrigationShiftValue->value()
                || cropFromDB.degreeDaysStartIrrigation != degreeDaysStartValue->text().toInt()
                || cropFromDB.degreeDaysEndIrrigation != degreeDaysEndValue->text().toInt() )
            {
                isCropChanged = true;
                return isCropChanged;
            }
        }
    }

    isCropChanged = false;
    return isCropChanged;
}


void Criteria1DWidget::irrigationVolumeChanged()
{
    double irrigationVolume = QLocale().toDouble(irrigationVolumeValue->text());

    if (irrigationVolume == 0)
    {
        irrigationShiftValue->setEnabled(false);
        degreeDaysStartValue->setEnabled(false);
        degreeDaysEndValue->setEnabled(false);
    }
    else if (irrigationVolume > 0)
    {
        irrigationShiftValue->setEnabled(true);
        degreeDaysStartValue->setEnabled(true);
        degreeDaysEndValue->setEnabled(true);
    }
}


bool Criteria1DWidget::setMeteoSqlite(QString& errorStr)
{
    if (myProject.myCase.meteoPoint.id.empty())
        return false;

    QString queryString = "SELECT * FROM '" + meteoTableName + "' ORDER BY [date]";
    QSqlQuery query = myProject.dbMeteo.exec(queryString);
    query.last();

    if (! query.isValid())
    {
        if (query.lastError().text() != "")
            errorStr = "dbMeteo errorStr: " + query.lastError().text();
        else
            errorStr = "Missing meteo table: " + meteoTableName;
        return false;
    }

    query.first();
    QDate firstDate = query.value("date").toDate();
    query.last();
    QDate lastDate = query.value("date").toDate();
    unsigned nrDays = unsigned(firstDate.daysTo(lastDate)) + 1;

    // Initialize data
    myProject.myCase.meteoPoint.initializeObsDataD(nrDays, getCrit3DDate(firstDate));

    // Read observed data
    int maxNrDays = NODATA; // all data
    if (! readDailyDataCriteria1D(query, myProject.myCase.meteoPoint, maxNrDays, errorStr))
        return false;

    if (errorStr != "")
    {
        QMessageBox::warning(nullptr, "WARNING!", errorStr);
    }

    // fill watertable
    if(myProject.myCase.unit.useWaterTableData && myProject.myCase.waterTableParameters.isLoaded)
    {
        myProject.myCase.fillWaterTableData();
    }

    return true;
}


void Criteria1DWidget::on_actionViewWeather()
{
    if (! myProject.isXmlMeteoGrid())
    {
        if (! setMeteoSqlite(myProject.projectError))
        {
            QMessageBox::critical(nullptr, "ERROR!", myProject.projectError);
            return;
        }
    }

    Crit3DMeteoWidget* meteoWidgetPoint = new Crit3DMeteoWidget(myProject.isXmlMeteoGrid(), myProject.path, &meteoSettings);

    QDate lastDate = getQDate(myProject.myCase.meteoPoint.getLastDailyData());
    meteoWidgetPoint->setCurrentDate(lastDate);

    meteoWidgetPoint->drawMeteoPoint(myProject.myCase.meteoPoint, false);
}


void Criteria1DWidget::on_actionViewSoil()
{
    QString docPath;
    if (! searchDocPath(&docPath))
    {
        QMessageBox::critical(nullptr, "", "Missing DOC/img/ directory");
        return;
    }

    QString imgPath = docPath + "/img/";
    Crit3DSoilWidget* soilWidget = new Crit3DSoilWidget(imgPath);
    soilWidget->setDbSoil(myProject.dbSoil, soilListComboBox.currentText());
}

