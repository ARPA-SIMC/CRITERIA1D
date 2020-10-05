#include "tabMap.h"
#include "commonConstants.h"

TabMap::TabMap(QStringList varList, QDate firstDbDate, QDate lastDbDate)
    :varList(varList), firstDbDate(firstDbDate), lastDbDate(lastDbDate)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *varLayout = new QHBoxLayout;
    QHBoxLayout *elaborationLayout = new QHBoxLayout;
    QHBoxLayout *dateLayout = new QHBoxLayout;
    dateLayout->setAlignment(Qt::AlignLeft);
    QHBoxLayout *climateElabLayout = new QHBoxLayout;
    QHBoxLayout *outputNameLayout = new QHBoxLayout;
    outputNameLayout->setAlignment(Qt::AlignLeft);

    QLabel *variableLabel = new QLabel(tr("Variable:"));
    varLayout->addWidget(variableLabel);
    variableList = new QComboBox();
    variableList->setFixedWidth(200);
    for (int i = 0; i<varList.size(); i++)
    {
        variableList->addItem(varList[i]);
    }
    varLayout->addWidget(variableList);

    QLabel *elabLabel = new QLabel(tr("Elaboration:"));
    elaborationLayout->addWidget(elabLabel);
    // elaboration list
    elabList = new QComboBox();
    elabList->addItem("daily value");
    elabList->addItem("average");
    elabList->addItem("sum");
    elabList->addItem("max value");
    elabList->addItem("min value");
    elabList->setFixedWidth(200);
    elaborationLayout->addWidget(elabList);

    startDateLabel = new QLabel(tr("Start date:"));
    startDateLabel->setFixedWidth(100);
    startDate = new QDateEdit();
    startDate->setFixedWidth(100);
    startDate->setDisplayFormat("MM/dd/yyyy");
    startDate->setDate(firstDbDate);
    startDate->setMinimumDate(firstDbDate);
    startDate->setMaximumDate(lastDbDate);
    endDateLabel = new QLabel(tr("End date:"));
    endDateLabel->setFixedWidth(100);
    endDate = new QDateEdit();
    endDate->setFixedWidth(100);
    endDate->setDisplayFormat("MM/dd/yyyy");
    endDate->setDate(lastDbDate);
    endDate->setMinimumDate(firstDbDate);
    endDate->setMaximumDate(lastDbDate);
    dateLabel = new QLabel(tr("Date:"));
    dateLabel->setFixedWidth(100);
    date = new QDateEdit();
    date->setFixedWidth(100);
    date->setDisplayFormat("MM/dd/yyyy");
    date->setDate(lastDbDate);
    date->setMinimumDate(firstDbDate);
    date->setMaximumDate(lastDbDate);
    if (elabList->currentText() == "daily value")
    {
        startDateLabel->setVisible(false);
        startDate->setVisible(false);
        endDateLabel->setVisible(false);
        endDate->setVisible(false);
        dateLabel->setVisible(true);
        date->setVisible(true);
    }
    else
    {
        startDateLabel->setVisible(true);
        startDate->setVisible(true);
        endDateLabel->setVisible(true);
        endDate->setVisible(true);
        dateLabel->setVisible(false);
        date->setVisible(false);
    }
    dateLayout->addWidget(dateLabel);
    dateLayout->addWidget(date);
    dateLayout->addWidget(startDateLabel);
    dateLayout->addWidget(startDate);
    dateLayout->addSpacing(20);
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
    timeWindow->setFixedWidth(40);
    thresholdLabel = new QLabel(tr("Threshold:"));
    thresholdLabel->setVisible(false);
    threshold = new QLineEdit();
    threshold->setValidator(new QDoubleValidator(0, 100, 2));
    threshold->setVisible(false);
    threshold->setFixedWidth(40);
    climateElabLayout->addWidget(timeWindowLabel);
    climateElabLayout->addWidget(timeWindow);
    climateElabLayout->addWidget(thresholdLabel);
    climateElabLayout->addWidget(threshold);

    QLabel *fileNameLabel = new QLabel(tr("Output File Name:"));
    fileNameEdit = new QLineEdit();
    fileNameEdit->setFixedWidth(200);
    outputNameLayout->addWidget(fileNameLabel);
    outputNameLayout->addWidget(fileNameEdit);

    connect(climateComp, &QCheckBox::stateChanged, [=](int state){ this->climateComputation(state); });
    connect(elabList, &QComboBox::currentTextChanged, [=](const QString &newVar){ this->listElaboration(newVar); });

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

void TabMap::listElaboration(const QString value)
{
    if (value == "daily value")
    {
        startDateLabel->setVisible(false);
        startDate->setVisible(false);
        endDateLabel->setVisible(false);
        endDate->setVisible(false);
        dateLabel->setVisible(true);
        date->setVisible(true);
    }
    else
    {
        startDateLabel->setVisible(true);
        startDate->setVisible(true);
        endDateLabel->setVisible(true);
        endDate->setVisible(true);
        dateLabel->setVisible(false);
        date->setVisible(false);
    }
}

QDate TabMap::getStartDate() const
{
    return startDate->date();
}

QDate TabMap::getEndDate() const
{
    return endDate->date();
}

QDate TabMap::getDate() const
{
    return date->date();
}

QString TabMap::getVariable() const
{
    return variableList->currentText();
}

QString TabMap::getElab() const
{
    return elabList->currentText();
}

bool TabMap::isClimateComputation() const
{
    return climateComp->isChecked();
}

QString TabMap::getClimateComputation() const
{
    return climateCompList->currentText();
}

int TabMap::getTimeWindow() const
{
    if (timeWindow->text().isEmpty())
    {
        return NODATA;
    }
    else
    {
        return timeWindow->text().toInt();
    }
}

double TabMap::getThreshold() const
{
    if (threshold->text().isEmpty())
    {
        return NODATA;
    }
    else
    {
        return threshold->text().toDouble();
    }
}

QString TabMap::getOutputName() const
{
    return fileNameEdit->text();
}
