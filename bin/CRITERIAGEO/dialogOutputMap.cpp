#include "dialogOutputMap.h"
#include "tabMap.h"
#include "tabSymbology.h"

DialogOutputMap::DialogOutputMap()
{

    this->setWindowTitle("Output Map");
    tabWidget = new QTabWidget;
    tabWidget->addTab(new TabMap(), tr("Map"));
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
