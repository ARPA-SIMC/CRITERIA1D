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
    //elaborationLayout->addWidget(elabList);

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
    //climateElabLayout->addWidget(computationList);
    QLabel *timeWindowLabel = new QLabel(tr("TimeWindow:"));
    timeWindowLabel->setVisible(false);
    timeWindow = new QLineEdit();
    timeWindow->setVisible(false);
    QLabel *thresholdLabel = new QLabel(tr("Threshold:"));
    thresholdLabel->setVisible(false);
    threshold = new QLineEdit();
    threshold->setVisible(false);
    climateElabLayout->addWidget(timeWindowLabel);
    climateElabLayout->addWidget(timeWindow);
    climateElabLayout->addWidget(thresholdLabel);
    climateElabLayout->addWidget(threshold);

    QLabel *fileNameLabel = new QLabel(tr("Output File Name:"));
    fileNameEdit = new QLineEdit();
    outputNameLayout->addWidget(fileNameLabel);
    outputNameLayout->addWidget(fileNameEdit);

    mainLayout->addLayout(varLayout);
    mainLayout->addLayout(elaborationLayout);
    mainLayout->addLayout(dateLayout);
    mainLayout->addLayout(climateElabLayout);
    mainLayout->addLayout(outputNameLayout);

    setLayout(mainLayout);

}
