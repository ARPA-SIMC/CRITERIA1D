#include "dialogOutputMap.h"
#include "tabMap.h"
#include "tabSymbology.h"

DialogOutputMap::DialogOutputMap(QStringList varList)
    :varList(varList)
{

    this->setWindowTitle("Output Map");
    this->setFixedSize(550,300);
    tabWidget = new QTabWidget;
    tabWidget->addTab(new TabMap(this->varList), tr("Map"));
    tabWidget->addTab(new TabSymbology(), tr("Symbology"));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, [=](){ QDialog::done(QDialog::Accepted); });
    connect(buttonBox, &QDialogButtonBox::rejected, [=](){ QDialog::done(QDialog::Rejected); });
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
    }
    else    // cancel, close or exc was pressed
    {
        QDialog::done(QDialog::Rejected);
        return;
    }
}
