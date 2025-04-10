#include "dialogShapeAnomaly.h"


DialogShapeAnomaly::DialogShapeAnomaly(std::vector<Crit3DShapeHandler*> shapeObjList)
    :shapeObjList(shapeObjList)
{
    this->setWindowTitle("Compute anomaly");
    this->setFixedSize(800,500);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* boxLayout = new QHBoxLayout;
    QVBoxLayout* shapeLayout = new QVBoxLayout;
    QVBoxLayout* idLayout = new QVBoxLayout;
    QVBoxLayout* fieldLayout = new QVBoxLayout;

    // add SHAPE widgets
    shape1ListWidget = new QListWidget();
    shape2ListWidget = new QListWidget();

    QLabel *climateLabel = new QLabel();
    climateLabel->setText("Select shape CLIMATE:");
    QLabel *forecastLabel = new QLabel();
    forecastLabel->setText("Select shape FORECAST:");
    shapeLayout->addWidget(climateLabel);
    shapeLayout->addWidget(shape1ListWidget);
    shapeLayout->addWidget(forecastLabel);
    shapeLayout->addWidget(shape2ListWidget);

    QList<QString> shapeLabel;
    for (unsigned int i = 0; i < shapeObjList.size(); i++)
    {
        QString filepath = QString::fromStdString(shapeObjList.at(i)->getFilepath());
        QFileInfo file(filepath);
        shapeLabel << file.baseName();
    }

    shape1ListWidget->addItems(shapeLabel);
    shape2ListWidget->addItems(shapeLabel);

    // add ID widgets
    id1ListWidget = new QListWidget();
    id2ListWidget = new QListWidget();

    QLabel *climateIdLabel = new QLabel();
    climateIdLabel->setText("Select ID:");
    QLabel *forecastIdLabel = new QLabel();
    forecastIdLabel->setText("Select ID:");

    idLayout->addWidget(climateIdLabel);
    idLayout->addWidget(id1ListWidget);
    idLayout->addWidget(forecastIdLabel);
    idLayout->addWidget(id2ListWidget);

    // add FIELD widgets
    field1ListWidget = new QListWidget();
    field2ListWidget = new QListWidget();

    QLabel *climateFieldLabel = new QLabel();
    climateFieldLabel->setText("Select field:");
    QLabel *forecastFieldLabel = new QLabel();
    forecastFieldLabel->setText("Select field:");

    fieldLayout->addWidget(climateFieldLabel);
    fieldLayout->addWidget(field1ListWidget);
    fieldLayout->addWidget(forecastFieldLabel);
    fieldLayout->addWidget(field2ListWidget);

    boxLayout->addLayout(shapeLayout);
    boxLayout->addLayout(idLayout);
    boxLayout->addLayout(fieldLayout);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                             | QDialogButtonBox::Cancel);

    connect(shape1ListWidget, &QListWidget::itemClicked, [=](QListWidgetItem* item){ this->shape1Clicked(item); });
    connect(shape2ListWidget, &QListWidget::itemClicked, [=](QListWidgetItem* item){ this->shape2Clicked(item); });

    connect(buttonBox, &QDialogButtonBox::accepted, [=](){ this->computeAnomaly(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [=](){ QDialog::done(QDialog::Rejected); });

    mainLayout->addLayout(boxLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    exec();
}


void DialogShapeAnomaly::shape1Clicked(QListWidgetItem* item)
{
    unsigned int pos = shape1ListWidget->row(item);
    shapeClimate = shapeObjList.at(pos);

    QList<QString> fieldList;
    for (int i = 0; i < shapeClimate->getFieldNumbers(); i++)
    {
        std::string fieldName = shapeClimate->getFieldName(i);
        fieldList << QString::fromStdString(fieldName);
    }

    id1ListWidget->clear();
    id1ListWidget->addItems(fieldList);

    field1ListWidget->clear();
    field1ListWidget->addItems(fieldList);
}


void DialogShapeAnomaly::shape2Clicked(QListWidgetItem* item)
{
    unsigned int pos = shape2ListWidget->row(item);
    shapeForecast = shapeObjList.at(pos);

    QList<QString> fieldList;
    for (int i = 0; i < shapeForecast->getFieldNumbers(); i++)
    {
        std::string fieldName =  shapeForecast->getFieldName(i);
        fieldList << QString::fromStdString(fieldName);
    }

    id2ListWidget->clear();
    id2ListWidget->addItems(fieldList);

    field2ListWidget->clear();
    field2ListWidget->addItems(fieldList);
}


void DialogShapeAnomaly::computeAnomaly()
{
    QListWidgetItem* itemSelected;
    unsigned int pos;

    // check shape selection
    itemSelected = shape1ListWidget->currentItem();
    if (itemSelected == nullptr)
    {
        QMessageBox::information(nullptr, "Warning!", "Select a climate shape");
        return;
    }
    pos = shape1ListWidget->row(itemSelected);
    shapeClimate = shapeObjList.at(pos);

    itemSelected = shape2ListWidget->currentItem();
    if (itemSelected == nullptr)
    {
        QMessageBox::information(nullptr, "Warning!", "Select a forecast shape");
        return;
    }
    pos = shape2ListWidget->row(itemSelected);
    shapeForecast = shapeObjList.at(pos);

    // check shape (improve)
    ShapeObject firstShapeClimate, firstShapeForecast;
    shapeClimate->getShape(0, firstShapeClimate);
    shapeForecast->getShape(0, firstShapeForecast);
    if (firstShapeClimate.getVertex(0).x != firstShapeForecast.getVertex(0).x)
    {
        QMessageBox::information(nullptr, "Warning!", "Shapes are different.");
        return;
    }

    // check id selection
    itemSelected = id1ListWidget->currentItem();
    if (itemSelected == nullptr)
    {
        QMessageBox::information(nullptr, "Warning!", "Select climate ID.");
        return;
    }
    idClimate = itemSelected->text();

    itemSelected = id2ListWidget->currentItem();
    if (itemSelected == nullptr)
    {
        QMessageBox::information(nullptr, "Warning!", "Select forecast ID.");
        return;
    }
    idForecast = itemSelected->text();

    // check IDs
    if (idClimate != idForecast)
    {
        QMessageBox::information(nullptr, "Warning!", "IDs cannot be different.");
        return;
    }

    // check field selection
    itemSelected = field1ListWidget->currentItem();
    if (itemSelected == nullptr)
    {
        QMessageBox::information(nullptr, "Warning!", "Select a climate field");
        return;
    }
    pos = field1ListWidget->row(itemSelected);
    DBFFieldType type = shapeClimate->getFieldType(pos);
    if (type != FTInteger && type != FTDouble)
    {
        QMessageBox::information(nullptr, "Warning!", "Select a numeric field (climate).");
        return;
    }
    fieldClimate = itemSelected->text();

    itemSelected = field2ListWidget->currentItem();
    if (itemSelected == nullptr)
    {
        QMessageBox::information(nullptr, "Warning!", "Select a forecast field");
        return;
    }
    pos = field2ListWidget->row(itemSelected);
    type = shapeForecast->getFieldType(pos);
    if (type != FTInteger && type != FTDouble)
    {
        QMessageBox::information(nullptr, "Warning!", "Select a numeric field (forecast).");
        return;
    }
    fieldForecast = itemSelected->text();

    QDialog::done(QDialog::Accepted);
}
