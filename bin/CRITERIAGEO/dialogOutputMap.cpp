#include "dialogOutputMap.h"

DialogOutputMap::DialogOutputMap(QStringList varList)
    :varList(varList)
{

    this->setWindowTitle("Output Map");
    this->setFixedSize(550,300);
    tabWidget = new QTabWidget;
    tabMap = new TabMap(this->varList);
    tabSymbology = new TabSymbology();
    tabWidget->addTab(tabMap, tr("Map"));
    tabWidget->addTab(tabSymbology, tr("Symbology"));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, [=](){ this->done(true); });
    connect(buttonBox, &QDialogButtonBox::rejected, [=](){ this->done(false); });
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    exec();

}

DialogOutputMap::~DialogOutputMap()
{
    close();
}

void DialogOutputMap::done(bool res)
{

    if(res)  // ok was pressed
    {
        // check valid date
        if (tabMap->getElab() != "daily value")
        {
            if (tabMap->getStartDate() > tabMap->getEndDate())
            {
                QMessageBox::information(nullptr, "Error!", "Invalid date ");
                return;
            }
        }
        if (tabMap->getOutputName().isEmpty())
        {
            QMessageBox::information(nullptr, "Error!", "Insert output file name");
            return;
        }
        QDialog::done(QDialog::Accepted);
        return;
    }
    else    // cancel, close or exc was pressed
    {
        QDialog::done(QDialog::Rejected);
        return;
    }
}
