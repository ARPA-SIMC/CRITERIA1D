#include "dbfNewColDialog.h"


DbfNewColDialog::DbfNewColDialog()
{

    insertOK = false;
    this->setFixedSize(400,300);
    this->setWindowTitle("New Column");
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QVBoxLayout *vbox = new QVBoxLayout;

    name = new QLineEdit();
    name->setPlaceholderText("Insert name: ");

    QGroupBox *groupBox = new QGroupBox(tr("Insert type"));
    stringButton = new QRadioButton(tr("FTString"));
    intButton = new QRadioButton(tr("FTInteger"));
    doubleButton = new QRadioButton(tr("FTDouble"));
    stringButton->setChecked(true);


    nWidth = new QLineEdit();
    nWidth->setPlaceholderText("Insert the width of the field: ");
    nWidth->setValidator(new QIntValidator(0,100));

    mainLayout->addWidget(name);

    vbox->addWidget(stringButton);
    vbox->addWidget(intButton);
    vbox->addWidget(doubleButton);


    groupBox->setLayout(vbox);
    mainLayout->addWidget(groupBox);

    mainLayout->addWidget(nWidth);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                             | QDialogButtonBox::Cancel);

    connect(stringButton, &QRadioButton::clicked, [=](){ this->showWidthEdit(); });
    connect(intButton, &QRadioButton::clicked, [=](){ this->hideWidthEdit(); });
    connect(doubleButton, &QRadioButton::clicked, [=](){ this->hideWidthEdit(); });
    connect(buttonBox, &QDialogButtonBox::accepted, [=](){ this->insertCol(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [=](){ QDialog::done(QDialog::Rejected); });

    mainLayout->addWidget(buttonBox);
    this->setLayout(mainLayout);
    exec();
}

DbfNewColDialog::~DbfNewColDialog()
{
    close();
}

bool DbfNewColDialog::insertCol()
{
    if (!checkValidData())
    {
        insertOK = false;
        return false;
    }
    insertOK = true;
    QDialog::done(QDialog::Accepted);
    return true;

}

void DbfNewColDialog::showWidthEdit()
{
    nWidth->setVisible(true);
    return;
}

void DbfNewColDialog::hideWidthEdit()
{
    nWidth->setVisible(false);
    return;
}

bool DbfNewColDialog::checkValidData()
{
    if (name->text().isEmpty())
    {
        QMessageBox::information(nullptr, "Missing name", "Insert name");
        return false;
    }
    if (stringButton->isChecked() && nWidth->text().isEmpty())
    {
        QMessageBox::information(nullptr, "Missing width", "Insert the width of the field");
        return false;
    }


    if (nWidth->text().contains(",") || nWidth->text().contains("+") || nWidth->text().contains("-"))
    {
        QMessageBox::information(nullptr, "Invalid width", "Insert only digits");
        return false;
    }
    return true;
}


bool DbfNewColDialog::getInsertOK() const
{
    return insertOK;
}

QString DbfNewColDialog::getName()
{
    return name->text();
}

int DbfNewColDialog::getType()
{
    if (stringButton->isChecked())
    {
       return FTString;
    }
    else if (intButton->isChecked())
    {
       return FTInteger;
    }
    else
       return FTDouble;
}

int DbfNewColDialog::getWidth()
{
    return nWidth->text().toInt();
}


