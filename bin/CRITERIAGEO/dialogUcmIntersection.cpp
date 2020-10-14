#include "dialogUcmIntersection.h"

DialogUcmIntersection::DialogUcmIntersection(std::vector<Crit3DShapeHandler*> shapeObjList)
    :shapeObjList(shapeObjList)
{

    this->setWindowTitle("Unit Crop Map intersection");
    this->setFixedSize(600,500);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* boxCropLayout = new QHBoxLayout;
    QHBoxLayout* boxSoilLayout = new QHBoxLayout;
    QHBoxLayout* boxMeteoLayout = new QHBoxLayout;

    QGridLayout* cropShapeLayout = new QGridLayout;
    QGridLayout* soilShapeLayout = new QGridLayout;
    QGridLayout* meteoShapeLayout = new QGridLayout;
    cropShapeLayout->setAlignment(Qt::AlignTop);
    soilShapeLayout->setAlignment(Qt::AlignTop);
    meteoShapeLayout->setAlignment(Qt::AlignTop);

    cropFieldLayout = new QGridLayout;
    soilFieldLayout = new QGridLayout;
    meteoFieldLayout = new QGridLayout;
    cropFieldLayout->setAlignment(Qt::AlignTop);
    soilFieldLayout->setAlignment(Qt::AlignTop);
    meteoFieldLayout->setAlignment(Qt::AlignTop);

    boxCropLayout->setAlignment(Qt::AlignLeft);
    boxSoilLayout->setAlignment(Qt::AlignLeft);
    boxMeteoLayout->setAlignment(Qt::AlignLeft);
    cropShape = new QListWidget();
    soilShape = new QListWidget();
    meteoShape = new QListWidget();

    QLabel *cropLabel = new QLabel();
    cropLabel->setText("Select shape CROP or enter ID_CROP key:");
    idCropEdit = new QLineEdit();
    idCropEdit->setFixedSize(300, 20);
    QLabel *soilLabel = new QLabel();
    idSoilEdit = new QLineEdit();
    idSoilEdit->setFixedSize(300, 20);
    soilLabel->setText("Select shape SOIL or enter ID_SOIL key:");
    QLabel *meteoLabel = new QLabel();
    idMeteoEdit = new QLineEdit();
    idMeteoEdit->setFixedSize(300, 20);
    meteoLabel->setText("Select shape METEO or enter ID_METEO key:");
    cropShapeLayout->addWidget(cropLabel);
    cropShapeLayout->addWidget(idCropEdit);
    cropShapeLayout->addWidget(cropShape);

    soilShapeLayout->addWidget(soilLabel);
    soilShapeLayout->addWidget(idSoilEdit);
    soilShapeLayout->addWidget(soilShape);

    meteoShapeLayout->addWidget(meteoLabel);
    meteoShapeLayout->addWidget(idMeteoEdit);
    meteoShapeLayout->addWidget(meteoShape);

    boxCropLayout->addLayout(cropShapeLayout);
    boxSoilLayout->addLayout(soilShapeLayout);
    boxMeteoLayout->addLayout(meteoShapeLayout);

    QStringList shapeLabel;

    for (unsigned int i = 0; i < shapeObjList.size(); i++)
    {
        QString filepath = QString::fromStdString(shapeObjList.at(i)->getFilepath());
        QFileInfo file(filepath);
        shapeLabel << file.baseName();
    }
    cropShape->addItems(shapeLabel);
    soilShape->addItems(shapeLabel);
    meteoShape->addItems(shapeLabel);

    cropShape->setMaximumWidth(300);
    soilShape->setMaximumWidth(300);
    meteoShape->setMaximumWidth(300);

    cropField = new QListWidget();
    soilField = new QListWidget();
    meteoField = new QListWidget();

    cropField->setFixedHeight(100);
    soilField->setFixedHeight(100);
    soilField->setFixedHeight(100);

    QLabel *cropFieldLabel = new QLabel();
    cropFieldLabel->setText("Select ID CROP:");
    QLabel *soilFieldLabel = new QLabel();
    soilFieldLabel->setText("Select ID SOIL:");
    QLabel *meteoFieldLabel = new QLabel();
    meteoFieldLabel->setText("Select ID METEO:");

    cropFieldLabel->setFixedHeight(20);
    soilFieldLabel->setFixedHeight(20);
    meteoFieldLabel->setFixedHeight(20);

    cropFieldLayout->addWidget(cropFieldLabel);
    cropFieldLayout->addWidget(cropField);
    soilFieldLayout->addWidget(soilFieldLabel);
    soilFieldLayout->addWidget(soilField);
    meteoFieldLayout->addWidget(meteoFieldLabel);
    meteoFieldLayout->addWidget(meteoField);

    boxCropLayout->addLayout(cropFieldLayout);
    boxSoilLayout->addLayout(soilFieldLayout);
    boxMeteoLayout->addLayout(meteoFieldLayout);

    // hide field selection
    showChildren(cropFieldLayout, false);
    showChildren(soilFieldLayout, false);
    showChildren(meteoFieldLayout, false);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                             | QDialogButtonBox::Cancel);

    connect(cropShape, &QListWidget::itemClicked, [=](QListWidgetItem* item){ this->shapeCropClicked(item); });
    connect(soilShape, &QListWidget::itemClicked, [=](QListWidgetItem* item){ this->shapeSoilClicked(item); });
    connect(meteoShape, &QListWidget::itemClicked, [=](QListWidgetItem* item){ this->shapeMeteoClicked(item); });
    connect(buttonBox, &QDialogButtonBox::accepted, [=](){ this->ucm(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [=](){ QDialog::done(QDialog::Rejected); });

    connect(idCropEdit, &QLineEdit::textEdited, [=](QString text){ this->idCropEditing(text); });
    connect(idSoilEdit, &QLineEdit::textEdited, [=](QString text){ this->idSoilEditing(text); });
    connect(idMeteoEdit, &QLineEdit::textEdited, [=](QString text){ this->idMeteoEditing(text); });

    mainLayout->addLayout(boxCropLayout);
    mainLayout->addLayout(boxSoilLayout);
    mainLayout->addLayout(boxMeteoLayout);

    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    exec();
}


void DialogUcmIntersection::shapeCropClicked(QListWidgetItem* item)
{
    cropField->clear();
    idCropEdit->clear();
    idCrop.clear();
    showChildren(cropFieldLayout, true);
    unsigned int pos = cropShape->row(item);
    crop = shapeObjList.at(pos);

    QStringList fieldLabel;
    for (int i = 0; i < crop->getFieldNumbers(); i++)
    {
        std::string nameField =  crop->getFieldName(i);
        fieldLabel << QString::fromStdString(nameField);
    }
    cropField->addItems(fieldLabel);
}

void DialogUcmIntersection::shapeSoilClicked(QListWidgetItem* item)
{
    soilField->clear();
    idSoilEdit->clear();
    idSoil.clear();
    showChildren(soilFieldLayout, true);
    unsigned int pos = soilShape->row(item);
    soil = shapeObjList.at(pos);

    QStringList fieldLabel;
    for (int i = 0; i < soil->getFieldNumbers(); i++)
    {
        std::string nameField =  soil->getFieldName(i);
        fieldLabel << QString::fromStdString(nameField);
    }
    soilField->addItems(fieldLabel);
}

void DialogUcmIntersection::shapeMeteoClicked(QListWidgetItem* item)
{
    meteoField->clear();
    idMeteoEdit->clear();
    idMeteo.clear();
    showChildren(meteoFieldLayout, true);
    unsigned int pos = meteoShape->row(item);
    meteo = shapeObjList.at(pos);

    QStringList fieldLabel;
    for (int i = 0; i < meteo->getFieldNumbers(); i++)
    {
        std::string nameField =  meteo->getFieldName(i);
        fieldLabel << QString::fromStdString(nameField);
    }
    meteoField->addItems(fieldLabel);
}

void DialogUcmIntersection::idCropEditing(QString newText)
{
    Q_UNUSED(newText);

    cropShape->clearSelection();
    cropField->clear();
    if (idCropEdit->text().isEmpty())
    {
        idCrop.clear();
        showChildren(cropFieldLayout, true);
    }
    else
    {
        showChildren(cropFieldLayout, false);
        idCrop = idCropEdit->text();
    }
}

void DialogUcmIntersection::idSoilEditing(QString newText)
{
    Q_UNUSED(newText);

    soilShape->clearSelection();
    soilField->clear();
    if (idSoilEdit->text().isEmpty())
    {
        idSoil.clear();
        showChildren(soilFieldLayout, true);
    }
    else
    {
        showChildren(soilFieldLayout, false);
        idSoil = idSoilEdit->text();
    }
}

void DialogUcmIntersection::idMeteoEditing(QString newText)
{
    Q_UNUSED(newText);

    meteoShape->clearSelection();
    meteoField->clear();
    if (idMeteoEdit->text().isEmpty())
    {
        idMeteo.clear();
        showChildren(meteoFieldLayout, true);
    }
    else
    {
        showChildren(meteoFieldLayout, false);
        idMeteo = idMeteoEdit->text();
    }
}


void DialogUcmIntersection::ucm()
{
    QListWidgetItem* itemSelected;

    // check shape selection
    itemSelected = cropShape->currentItem();
    unsigned int pos;
    if (itemSelected == nullptr && idCropEdit->text().isEmpty())
    {
        QMessageBox::information(nullptr, "No crop shape selected", "Select a shape or enter ID");
        return;
    }
    if (itemSelected != nullptr)
    {
        pos = cropShape->row(itemSelected);
        crop = shapeObjList.at(pos);
    }
    else
    {
        crop = nullptr;
    }

    itemSelected = soilShape->currentItem();
    if (itemSelected == nullptr && idSoilEdit->text().isEmpty())
    {
        QMessageBox::information(nullptr, "No soil shape selected", "Select a shape or enter ID");
        return;
    }
    if (itemSelected != nullptr)
    {
        pos = soilShape->row(itemSelected);
        soil = shapeObjList.at(pos);
    }
    else
    {
        soil = nullptr;
    }

    itemSelected = meteoShape->currentItem();
    if (itemSelected == nullptr && idMeteoEdit->text().isEmpty())
    {
        QMessageBox::information(nullptr, "No meteo shape selected", "Select a shape or enter ID");
        return;
    }
    if (itemSelected != nullptr)
    {
        pos = meteoShape->row(itemSelected);
        meteo = shapeObjList.at(pos);
    }
    else
    {
        meteo = nullptr;
    }

    unsigned int nShapeSelected = 0;
    // check field selection
    if (idCropEdit->text().isEmpty())
    {
        nShapeSelected = nShapeSelected + 1;
        itemSelected = cropField->currentItem();
        if (itemSelected == nullptr)
        {
            QMessageBox::information(nullptr, "No id crop selected", "Select a field");
            return;
        }
        idCrop = itemSelected->text();
    }

    if (idSoilEdit->text().isEmpty())
    {
        nShapeSelected = nShapeSelected + 1;
        itemSelected = soilField->currentItem();
        if (itemSelected == nullptr)
        {
            QMessageBox::information(nullptr, "No id soil selected", "Select a field");
            return;
        }
        idSoil = itemSelected->text();
    }

    if (idMeteoEdit->text().isEmpty())
    {
        nShapeSelected = nShapeSelected + 1;
        itemSelected = meteoField->currentItem();
        if (itemSelected == nullptr)
        {
            QMessageBox::information(nullptr, "No id meteo selected", "Select a field");
            return;
        }
        idMeteo = itemSelected->text();
    }

    if (nShapeSelected < 2)
    {
        QMessageBox::information(nullptr, "Too many constant ID entered", "Select at least 2 shapes");
        return;
    }

    QDialog::done(QDialog::Accepted);
}

void DialogUcmIntersection::showChildren(const QGridLayout* layout, bool show)
{
  QLayoutItem *item = 0;
  QWidget *widget = 0;

  for(int i = 0; i < layout->rowCount(); ++i)
  {
    for(int j = 0; j < layout->columnCount(); ++j)
    {
      item = layout->itemAtPosition(i,j);
      widget=item?item->widget():0;
      if(widget)
        widget->setVisible(show);
    }
  }
}

QString DialogUcmIntersection::getIdCrop() const
{
    return idCrop;
}

QString DialogUcmIntersection::getIdSoil() const
{
    return idSoil;
}

QString DialogUcmIntersection::getIdMeteo() const
{
    return idMeteo;
}

Crit3DShapeHandler *DialogUcmIntersection::getCrop() const
{
    return crop;
}

Crit3DShapeHandler *DialogUcmIntersection::getSoil() const
{
    return soil;
}

Crit3DShapeHandler *DialogUcmIntersection::getMeteo() const
{
    return meteo;
}

