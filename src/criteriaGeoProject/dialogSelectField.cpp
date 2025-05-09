#include "dialogSelectField.h"

DialogSelectField::DialogSelectField(Crit3DShapeHandler* shapeHandler, QString fileName, bool isOnlyNumeric, dialogType dialogType)    :shapeHandler(shapeHandler)
{
    if (isOnlyNumeric)
    {
        setWindowTitle("Choose a numeric field of " + fileName);
    }
    else
    {
        setWindowTitle("Choose a field of " + fileName);
    }
    setFixedSize(400, 300);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    listFields = new QListWidget();
    mainLayout->addWidget(listFields);

    if (dialogType == RASTERIZE)
    {
        cellSize = new QLineEdit();
        cellSize->setPlaceholderText("cell size [m]");
        cellSize->setValidator(new QDoubleValidator(0, 9999, 2)); //LC accetta double con 2 cifre decimali da 0 a 9999
        outputName = new QLineEdit();
        outputName->setPlaceholderText("Output Name");
        mainLayout->addWidget(cellSize);
        mainLayout->addWidget(outputName);
    }
    else if (dialogType == GDALRASTER)
    {
        cellSize = new QLineEdit();
        cellSize->setPlaceholderText("cell size [m]");
        cellSize->setValidator(new QDoubleValidator(0, 9999, 2)); //LC accetta double con 2 cifre decimali da 0 a 9999
        mainLayout->addWidget(cellSize);
    }
    else if (dialogType == RASTERIZE_WITHBASE)
    {
        outputName = new QLineEdit();
        outputName->setPlaceholderText("Output Name");
        mainLayout->addWidget(outputName);
    }

    DBFFieldType typeField;
    QList<QString> fields;

    if (dialogType == RASTERIZE || dialogType == RASTERIZE_WITHBASE)
    {
        fields << "Shape ID";
    }

    if (dialogType == SHAPESTYLE)
    {
        fields << "Edges only";
    }


    for (int i = 0; i < shapeHandler->getFieldNumbers(); i++)
    {
        typeField = shapeHandler->getFieldType(i);
        if (isOnlyNumeric)
        {
            if (typeField == FTInteger || typeField == FTDouble)
            {
                fields << QString::fromStdString(shapeHandler->getFieldName(i));
            }
        }
        else fields << QString::fromStdString(shapeHandler->getFieldName(i));
    }
    listFields->addItems(fields);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    if (dialogType == RASTERIZE || dialogType == RASTERIZE_WITHBASE)
        connect(buttonBox, &QDialogButtonBox::accepted, [=](){ this->acceptRasterize(); });
    else
        connect(buttonBox, &QDialogButtonBox::accepted, [=](){ this->acceptSelection(); });

    connect(buttonBox, &QDialogButtonBox::rejected, [=](){ QDialog::done(QDialog::Rejected); });

    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    exec();
}


void DialogSelectField::acceptRasterize()
{
    QListWidgetItem * itemSelected = listFields->currentItem();
    if (itemSelected == nullptr)
    {
        QMessageBox::information(nullptr, "No items selected", "Select a field");
        return;
    }
    if (outputName->text().isEmpty())
    {
        QMessageBox::information(nullptr, "Empty name", "Insert output name");
        return;
    }
    QDialog::done(QDialog::Accepted);
}


void DialogSelectField::acceptSelection()
{
    QListWidgetItem * itemSelected = listFields->currentItem();
    if (itemSelected == nullptr)
    {
        QMessageBox::information(nullptr, "No items selected", "Select a field");
        return;
    }

    QDialog::done(QDialog::Accepted);
}


QString DialogSelectField::getOutputName()
{
    return outputName->text();
}

double DialogSelectField::getCellSize() const
{
    QString cellString = cellSize->text();
    cellString.replace(",", ".");
    return cellString.toDouble();
}

QString DialogSelectField::getFieldSelected()
{
    return listFields->currentItem()->text();
}
