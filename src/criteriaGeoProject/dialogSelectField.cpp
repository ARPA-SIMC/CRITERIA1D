#include "dialogSelectField.h"

DialogSelectField::DialogSelectField(Crit3DShapeHandler* shapeHandler, QString fileName, bool isOnlyNumeric, dialogType dialogType)
    :_shapeHandler(shapeHandler)
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
    _listFields = new QListWidget();
    mainLayout->addWidget(_listFields);

    if (dialogType == RASTERIZE || dialogType == GDALRASTER)
    {
        _numericValue = new QLineEdit();
        _numericValue->setPlaceholderText("cell size [m]");
        _numericValue->setValidator(new QDoubleValidator(0, 9999, 2));
        mainLayout->addWidget(_numericValue);
        if (dialogType == RASTERIZE)
        {
            _stringValue = new QLineEdit();
            _stringValue->setPlaceholderText("Output Name");
            mainLayout->addWidget(_stringValue);
        }
    }
    else if (dialogType == RASTERIZE_WITHBASE)
    {
        _stringValue = new QLineEdit();
        _stringValue->setPlaceholderText("Output Name");
        mainLayout->addWidget(_stringValue);
    }
    else if (dialogType == PREVAILING)
    {
        _numericValue = new QLineEdit();
        _numericValue->setPlaceholderText("Threshold ratio [-]");
        _numericValue->setValidator(new QDoubleValidator(0, 1, 3));
        _numericValue->setText("0.33");
        mainLayout->addWidget(_numericValue);

        _stringValue = new QLineEdit();
        _stringValue->setPlaceholderText("New field (numeric)");
        mainLayout->addWidget(_stringValue);

        _checkBox = new QCheckBox();
        _checkBox->setText("Proportional distribution");
        mainLayout->addWidget(_checkBox);
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
    _listFields->addItems(fields);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    if (dialogType == RASTERIZE || dialogType == RASTERIZE_WITHBASE)
        connect(buttonBox, &QDialogButtonBox::accepted, [=](){ this->acceptRasterize(); });
    else if (dialogType == PREVAILING)
        connect(buttonBox, &QDialogButtonBox::accepted, [=](){ this->acceptPrevailing(); });
    else
        connect(buttonBox, &QDialogButtonBox::accepted, [=](){ this->acceptSelection(); });

    connect(buttonBox, &QDialogButtonBox::rejected, [=](){ QDialog::done(QDialog::Rejected); });

    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    exec();
}


void DialogSelectField::acceptRasterize()
{
    QListWidgetItem * itemSelected = _listFields->currentItem();
    if (itemSelected == nullptr)
    {
        QMessageBox::information(nullptr, "No items selected", "Select a field");
        return;
    }
    if (_stringValue->text().isEmpty())
    {
        QMessageBox::information(nullptr, "Empty name", "Insert output name");
        return;
    }

    QDialog::done(QDialog::Accepted);
}


void DialogSelectField::acceptPrevailing()
{
    if (_stringValue->text().isEmpty())
    {
        QListWidgetItem * itemSelected = _listFields->currentItem();
        if (itemSelected == nullptr)
        {
            QMessageBox::information(nullptr, "No items selected", "Select a field or enter a new field.");
            return;
        }
    }

    QDialog::done(QDialog::Accepted);
}


void DialogSelectField::acceptSelection()
{
    QListWidgetItem *itemSelected = _listFields->currentItem();
    if (itemSelected == nullptr)
    {
        QMessageBox::information(nullptr, "No items selected", "Select a field");
        return;
    }

    QDialog::done(QDialog::Accepted);
}
