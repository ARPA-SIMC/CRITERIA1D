#include "tabMap.h"

TabMap::TabMap()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *varLayout = new QHBoxLayout;
    QHBoxLayout *elaborationLayout = new QHBoxLayout;
    QHBoxLayout *dateLayout = new QHBoxLayout;
    QHBoxLayout *climateElabLayout = new QHBoxLayout;
    QHBoxLayout *outputNameLayout = new QHBoxLayout;

    QLabel *variableLabel = new QLabel(tr("Variable:"));
    varLayout->addWidget(variableLabel);
    //varLayout->addWidget(variableList);

    QLabel *elabLabel = new QLabel(tr("Elaboration:"));
    elaborationLayout->addWidget(elabLabel);
    // elaboration list
    elabList = new QComboBox();
    elabList->addItem("daily value");
    elabList->addItem("average");
    elabList->addItem("sum");
    elabList->addItem("max value");
    elabList->addItem("min value");
    elaborationLayout->addWidget(elabList);

    QLabel *startDateLabel = new QLabel(tr("Start date:"));
    startDate = new QDateEdit();
    QLabel *endDateLabel = new QLabel(tr("End date:"));
    endDate = new QDateEdit();
    dateLayout->addWidget(startDateLabel);
    dateLayout->addWidget(startDate);
    dateLayout->addWidget(endDateLabel);
    dateLayout->addWidget(endDate);

    climateComp = new QCheckBox();
    climateComp->setText("Climate Computation");
    climateComp->setChecked(false);
    climateElabLayout->addWidget(climateComp);
    // climate computation list
    climateCompList = new QComboBox();
    climateCompList->addItem("percentile");
    climateCompList->setVisible(false);
    climateElabLayout->addWidget(climateCompList);
    timeWindowLabel = new QLabel(tr("TimeWindow:"));
    timeWindowLabel->setVisible(false);
    timeWindow = new QLineEdit();
    timeWindow->setValidator(new QIntValidator(0, 100));
    timeWindow->setVisible(false);
    thresholdLabel = new QLabel(tr("Threshold:"));
    thresholdLabel->setVisible(false);
    threshold = new QLineEdit();
    threshold->setValidator(new QDoubleValidator(0, 100, 2));
    threshold->setVisible(false);
    climateElabLayout->addWidget(timeWindowLabel);
    climateElabLayout->addWidget(timeWindow);
    climateElabLayout->addWidget(thresholdLabel);
    climateElabLayout->addWidget(threshold);

    QLabel *fileNameLabel = new QLabel(tr("Output File Name:"));
    fileNameEdit = new QLineEdit();
    outputNameLayout->addWidget(fileNameLabel);
    outputNameLayout->addWidget(fileNameEdit);

    connect(climateComp, &QCheckBox::stateChanged, [=](int state){ this->climateComputation(state); });

    mainLayout->addLayout(varLayout);
    mainLayout->addLayout(elaborationLayout);
    mainLayout->addLayout(dateLayout);
    mainLayout->addLayout(climateElabLayout);
    mainLayout->addLayout(outputNameLayout);

    setLayout(mainLayout);

}

void TabMap::climateComputation(int state)
{
    if (state!= 0)
    {
        climateCompList->setVisible(true);
        timeWindowLabel->setVisible(true);
        timeWindow->setVisible(true);
        thresholdLabel->setVisible(true);
        threshold->setVisible(true);
    }
    else
    {
        climateCompList->setVisible(false);
        timeWindowLabel->setVisible(false);
        timeWindow->setVisible(false);
        thresholdLabel->setVisible(false);
        threshold->setVisible(false);
    }
}
