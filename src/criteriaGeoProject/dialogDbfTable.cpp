#include "dialogDbfTable.h"
#include "shapeUtilities.h"


DialogDbfTable::DialogDbfTable(Crit3DShapeHandler* shapeHandler, const QString& fileName)
    :shapeHandler(shapeHandler)
{
    if (shapeHandler == nullptr)
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("Invalid shapefile handler."));
        return;
    }

    // Create a temporary copy of the DBF file
    const QFileInfo filepathInfo(QString::fromStdString(shapeHandler->getFilepath()));

    const QString fileOrigin = filepathInfo.absolutePath() + "/"
                               + filepathInfo.baseName() + ".dbf";

    const QString fileTemp = filepathInfo.absolutePath() + "/"
                             + filepathInfo.baseName() + "_temp.dbf";

    if (QFile::exists(fileTemp) && ! QFile::remove(fileTemp))
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("Unable to remove temporary DBF file."));
        return;
    }

    if (! QFile::copy(fileOrigin, fileTemp))
    {
        QMessageBox::critical( nullptr, tr("Error"), tr("Unable to create temporary DBF."));
        return;
    }

    setWindowTitle(fileName);
    resize(800, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout;

    // set menu bar
    menuBar = new QMenuBar;
    editMenu = new QMenu(tr("&Edit"), this);
    addRow = editMenu->addAction(tr("Insert row"));
    deleteRow = editMenu->addAction(tr("Delete row"));
    editMenu->addSeparator();
    addCol = editMenu->addAction(tr("Insert column"));
    deleteCol = editMenu->addAction(tr("Delete column"));
    editMenu->addSeparator();
    copyAll = editMenu->addAction(tr("Copy all"));
    editMenu->addSeparator();
    save = editMenu->addAction(tr("Save changes"));
    menuBar->addMenu(editMenu);

    mainLayout->setMenuBar(menuBar);

    // DBF table
    m_DBFTableWidget = new TableDbf();
    mainLayout->addWidget(m_DBFTableWidget);

    const int colNumber = shapeHandler->getFieldNumbers();
    const int rowNumber = shapeHandler->getDBFRecordCount();

    m_DBFTableWidget->blockSignals(true);
    m_DBFTableWidget->setUpdatesEnabled(false);

    m_DBFTableWidget->setRowCount(rowNumber);
    m_DBFTableWidget->setColumnCount(colNumber);

    labels.clear();
    m_DBFTableHeader.clear();

    // store deleted rows
    std::vector<bool> isDeleted(rowNumber, false);

    for (int row = 0; row < rowNumber; ++row)
        isDeleted[row] = shapeHandler->isDBFRecordDeleted(row);

    // Read DBF fields.
    for (int col = 0; col < colNumber; ++col)
    {
        const std::string nameField = shapeHandler->getFieldName(col);

        const DBFFieldType typeField = shapeHandler->getFieldType(col);

        m_DBFTableHeader << QString::fromStdString(nameField);

        for (int row = 0; row < rowNumber; ++row)
        {
            QTableWidgetItem* item = nullptr;

            switch (typeField)
            {
            case FTString:
                item = new QTableWidgetItem(
                    QString::fromStdString(shapeHandler->readStringAttribute(row, col)));
                break;

            case FTInteger:
                item = new QTableWidgetItem(
                    QString::number(shapeHandler->readIntAttribute(row, col)));
                break;

            case FTDouble:
                item = new QTableWidgetItem(
                    QString::number(shapeHandler->readDoubleAttribute(row, col)));
                break;

            default:
                item = new QTableWidgetItem(tr("Not supported"));
                break;
            }

            if (isDeleted[row])
                item->setBackground(Qt::yellow); // mark as DELETED records

            m_DBFTableWidget->setItem(row, col, item);
        }
    }

    // Vertical header labels
    labels.reserve(rowNumber);

    for (int row = 0; row < rowNumber; ++row)
        labels << QString::number(row);

    m_DBFTableWidget->setVerticalHeaderLabels(labels);
    m_DBFTableWidget->setHorizontalHeaderLabels(m_DBFTableHeader);
    m_DBFTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_DBFTableWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);
    m_DBFTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_DBFTableWidget->setShowGrid(true);

    m_DBFTableWidget->setStyleSheet(
        "QTableView { selection-background-color: red; }");

    m_DBFTableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QLabel* labelLengend = new QLabel();
    labelLengend->setText("Deleted rows: yellow");

    mainLayout->addWidget(labelLengend);

    // Connections
    connect(m_DBFTableWidget, &QTableWidget::cellChanged, [this](int row, int column){ this->cellChanged(row, column); });
    connect(m_DBFTableWidget, &QTableWidget::customContextMenuRequested, [this](const QPoint point){ this->menuRequested(point); });
    connect(addRow, &QAction::triggered, [this](){ this->addRowClicked(); });
    connect(deleteRow, &QAction::triggered, [this](){ this->removeRowClicked(); });
    connect(addCol, &QAction::triggered, [this](){ this->addColClicked(); });
    connect(deleteCol, &QAction::triggered, [this](){ this->removeColClicked(); });
    connect(copyAll, &QAction::triggered, [this](){ this->copyAllClicked(); });
    connect(save, &QAction::triggered, [this](){ this->saveChangesClicked(); });

    connect(m_DBFTableWidget->horizontalHeader(), &QHeaderView::sectionClicked, [this](int index){ this->horizontalHeaderClick(index); });
    connect(m_DBFTableWidget->verticalHeader(), &QHeaderView::sectionClicked, [this](int index){ this->verticalHeaderClick(index); });

    m_DBFTableWidget->setUpdatesEnabled(true);
    m_DBFTableWidget->blockSignals(false);

    setLayout(mainLayout);
}


DialogDbfTable::~DialogDbfTable()
{ }

void DialogDbfTable::addRowClicked()
{
    m_DBFTableWidget->insertRow(m_DBFTableWidget->rowCount());
    labels << QString::number(labels.size());
    m_DBFTableWidget->setVerticalHeaderLabels(labels);
    m_DBFTableWidget->scrollToBottom();

}

void DialogDbfTable::removeRowClicked()
{
    if (m_DBFTableWidget->selectionBehavior() == QAbstractItemView::SelectColumns)
    {
        m_DBFTableWidget->clearSelection();
        m_DBFTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    }
    QItemSelectionModel *select = m_DBFTableWidget->selectionModel();

    if (select->hasSelection())
    {
        QModelIndexList indexList = select->selectedRows();
        int row = indexList.at(0).row();

        QMessageBox::StandardButton confirm = QMessageBox::Yes;
        confirm = QMessageBox::question( this, "Delete confirmation", "Delete shape number " + QString::number(row),
                         QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);

        if (confirm == QMessageBox::Yes)
        {
            qDebug() << "removeRowClicked ";
            if (m_DBFTableWidget->selectionBehavior() == QAbstractItemView::SelectColumns)
            {
                m_DBFTableWidget->clearSelection();
                m_DBFTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
            }

            if (shapeHandler->deleteRecord(row) || !(indexList.at(0).data(Qt::DisplayRole).isValid()) )
            {
                qDebug() << "deleteRecord = " << row;
                for (int i = 0; i < shapeHandler->getFieldNumbers(); i++)
                {
                    if (m_DBFTableWidget->item(row,i) != nullptr)
                    {
                        m_DBFTableWidget->item(row,i)->setBackground(Qt::yellow);
                    }
                    else
                    {
                        m_DBFTableWidget->removeRow(row);
                    }
                }
            }
        }
        else
        {
            qDebug() << "cancel operation ";
            return;
        }
    }
    else
    {
        QMessageBox::information(nullptr, "Select a row", "no row selected");
    }
}


void DialogDbfTable::addColClicked()
{
    newColDialog = new DialogDbfNewCol();
    if (newColDialog->getInsertOK())
    {
        QString name = newColDialog->getName();
        int typeField = newColDialog->getType();
        int width;
        int decimals;

        if (typeField == FTString)
        {
            width = newColDialog->getWidth();
            decimals = 0;
        }
        else if (typeField == FTInteger)
        {
            width = DEFAULT_INTEGER_MAX_DIGITS;
            decimals = 0;
        }
        else if (typeField == FTDouble)
        {
            width = DEFAULT_DOUBLE_MAX_DIGITS;
            decimals = DEFAULT_DOUBLE_MAX_DECIMALS;
        }
        else
        {
            qDebug() << "addition of field failed";
            return;
        }

        if (shapeHandler->addField(name.toStdString().c_str(), typeField, width, decimals))
        {
            m_DBFTableWidget->insertColumn(m_DBFTableWidget->columnCount());
            m_DBFTableHeader << name;
            m_DBFTableWidget->setHorizontalHeaderLabels(m_DBFTableHeader);
        }
        else
        {
            qDebug() << "addition of field failed";
        }

    }
}

void DialogDbfTable::removeColClicked()
{

    qDebug() << "removeColClicked ";
    if (m_DBFTableWidget->selectionBehavior() == QAbstractItemView::SelectRows)
    {
        m_DBFTableWidget->clearSelection();
        m_DBFTableWidget->setSelectionBehavior(QAbstractItemView::SelectColumns);
    }


    QItemSelectionModel *select = m_DBFTableWidget->selectionModel();

    if (select->hasSelection())
    {
        QModelIndexList indexList = select->selectedColumns();
        int col = indexList.at(0).column();

        if (shapeHandler->removeField(col))
        {
            qDebug() << "deleteCol = " << col;
            m_DBFTableWidget->removeColumn(col);
        }
    }
    else
    {
        QMessageBox::information(nullptr, "Select a column", "no column selected");
    }

}


void DialogDbfTable::cellChanged(int row, int column)
{
    qDebug() << "Cell at row: " << QString::number(row) << " column " << QString::number(column)<<" was changed.";
    QString data = m_DBFTableWidget->item(row, column)->text();
    int typeField = shapeHandler->getFieldType(column);
    if (typeField == FTString)
    {
        shapeHandler->writeStringAttribute(row,column,data.toStdString().c_str());
    }
    else if (typeField == FTInteger)
    {
        shapeHandler->writeIntAttribute(row,column, data.toInt());
    }
    else if (typeField == FTDouble)
    {
        shapeHandler->writeDoubleAttribute(row,column, data.toDouble());
    }
}


void DialogDbfTable::closeEvent(QCloseEvent *event)
{
    shapeHandler->close();

    QString filepath = QString::fromStdString(shapeHandler->getFilepath());
    QFileInfo filepathInfo(filepath);

    QString fileName = filepathInfo.absolutePath() + "/"
                       + filepathInfo.baseName();

    QString fileNameTmp = fileName + "_temp";

    // dbf
    QString dbf_backup = fileNameTmp + ".dbf";

    QFile::remove(fileName + ".dbf");
    QFile::copy(dbf_backup, fileName + ".dbf");
    QFile::remove(dbf_backup);

    // shp
    QString shp_backup = fileNameTmp + ".shp";
    QString shx_backup = fileNameTmp + ".shx";

    if (QFile::exists(shp_backup) && QFile::exists(shx_backup))
    {
        QFile::remove(fileName + ".shp");
        QFile::copy(shp_backup, fileName + ".shp");
        QFile::remove(shp_backup);

        QFile::remove(fileName + ".shx");
        QFile::copy(shx_backup, fileName + ".shx");
        QFile::remove(shx_backup);
    }

    // re-open shapefile
    shapeHandler->open(shapeHandler->getFilepath(), false);

    QDialog::closeEvent(event);
}


void DialogDbfTable::copyAllClicked()
{
    m_DBFTableWidget->selectAll();
    m_DBFTableWidget->copySelection();
}


void DialogDbfTable::saveChangesClicked()
{
    const QString filepath = QString::fromStdString(shapeHandler->getFilepath());
    QFileInfo filepathInfo(filepath);

    const QString backupBaseName = filepathInfo.absolutePath() + "/"
                                   + filepathInfo.baseName() + "_temp";

    const QString backupDbf = backupBaseName + ".dbf";

    // remove previous backup
    QFile::remove(backupDbf);

    if (shapeHandler->existRecordDeleted())
    {
        // create packed backup
        shapeHandler->packSHP(backupBaseName.toStdString());
        shapeHandler->packDBF(backupBaseName.toStdString());

        shapeHandler->close();
    }
    else
    {
        shapeHandler->close();

        // copy modified file
        const QString dbfFileName = filepathInfo.absolutePath()
                                    + "/" + filepathInfo.baseName() + ".dbf";

        if (! QFile::copy(dbfFileName, backupDbf))
        {
            QMessageBox::critical(this, tr("Error"), tr("Unable to create DBF backup."));
        }
    }

    // re-open shapefile
    shapeHandler->open(shapeHandler->getFilepath(), false);
}


void DialogDbfTable::horizontalHeaderClick(int index)
{
    m_DBFTableWidget->setSelectionBehavior(QAbstractItemView::SelectColumns);
    m_DBFTableWidget->setCurrentCell(0, index);
}

void DialogDbfTable::verticalHeaderClick(int index)
{
    m_DBFTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_DBFTableWidget->setCurrentCell(index, 0);
}

void DialogDbfTable::menuRequested(const QPoint point)
{
    QPoint itemPoint = m_DBFTableWidget->mapToGlobal(point);

    QMenu submenu;
    submenu.addAction("Copy");
    submenu.addSeparator();

    QAction* rightClickItem = submenu.exec(itemPoint);

    if (rightClickItem)
    {
        if (rightClickItem->text().contains("Copy") )
        {
            m_DBFTableWidget->copySelection();
        }
    }
    return;
}


