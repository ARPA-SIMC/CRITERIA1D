#include "dbfTableDialog.h"
#include "shapeUtilities.h"

DbfTableDialog::DbfTableDialog(Crit3DShapeHandler* shapeHandler, QString fileName)
    :shapeHandler(shapeHandler)
{
    // make a copy of shapefile
    QFileInfo filepathInfo(QString::fromStdString(shapeHandler->getFilepath()));
    QString file_temp = filepathInfo.absolutePath() + "/" + filepathInfo.baseName() + "_temp.dbf";
    QString file_origin = filepathInfo.absolutePath() + "/" + filepathInfo.baseName() + ".dbf";
    QFile::copy(file_origin, file_temp);

    this->setWindowTitle(fileName);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    this->resize(800, 600);

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

    m_DBFTableWidget = new TableDbf();
    mainLayout->addWidget(m_DBFTableWidget);

    int colNumber = shapeHandler->getFieldNumbers();
    int rowNumber = shapeHandler->getDBFRecordCount();

    m_DBFTableWidget->setRowCount(rowNumber);
    m_DBFTableWidget->setColumnCount(colNumber);

    std::string nameField;
    DBFFieldType typeField;

    labels.clear();
    m_DBFTableHeader.clear();

    for (int i=0; i < colNumber; i++)
    {
        nameField = shapeHandler->getFieldName(i);
        typeField = shapeHandler->getFieldType(i);
        m_DBFTableHeader << QString::fromStdString(nameField);

        for (int j = 0; j < rowNumber; j++)
        {  
            if (typeField == FTString)
            {
                m_DBFTableWidget->setItem(j, i, new QTableWidgetItem( QString::fromStdString(shapeHandler->readStringAttribute(j,i) )));
            }
            else if (typeField == FTInteger)
            {
                m_DBFTableWidget->setItem(j, i, new QTableWidgetItem( QString::number(shapeHandler->readIntAttribute(j,i) )));
            }
            else if (typeField == FTDouble)
            {
                m_DBFTableWidget->setItem(j, i, new QTableWidgetItem( QString::number(shapeHandler->readDoubleAttribute(j,i) )));
            }
            if (shapeHandler->isDBFRecordDeleted(j))
            {
                m_DBFTableWidget->item(j,i)->setBackground(Qt::yellow);    // mark as DELETED records
            }
        }
    }

    for (int j = 0; j < rowNumber; j++)
    {
        labels << QString::number(j);
    }

    m_DBFTableWidget->setVerticalHeaderLabels(labels);
    m_DBFTableWidget->setHorizontalHeaderLabels(m_DBFTableHeader);
    m_DBFTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_DBFTableWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);
    m_DBFTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_DBFTableWidget->setShowGrid(true);
    m_DBFTableWidget->setStyleSheet("QTableView {selection-background-color: red;}");

    int offset = 100;
    m_DBFTableWidget->setMinimumHeight(this->height() - offset);
    m_DBFTableWidget->setMaximumHeight(this->height() - offset);

    QLabel* labelLengend = new QLabel();
    labelLengend->setText("Deleted rows: yellow");

    mainLayout->addWidget(labelLengend);

    connect(m_DBFTableWidget, &QTableWidget::cellChanged, [=](int row, int column){ this->cellChanged(row, column); });
    connect(m_DBFTableWidget, &QTableWidget::customContextMenuRequested, [=](const QPoint point){ this->menuRequested(point); });
    connect(addRow, &QAction::triggered, [=](){ this->addRowClicked(); });
    connect(deleteRow, &QAction::triggered, [=](){ this->removeRowClicked(); });
    connect(addCol, &QAction::triggered, [=](){ this->addColClicked(); });
    connect(deleteCol, &QAction::triggered, [=](){ this->removeColClicked(); });
    connect(copyAll, &QAction::triggered, [=](){ this->copyAllClicked(); });
    connect(save, &QAction::triggered, [=](){ this->saveChangesClicked(); });

    connect(m_DBFTableWidget->horizontalHeader(), &QHeaderView::sectionClicked, [=](int index){ this->horizontalHeaderClick(index); });
    connect(m_DBFTableWidget->verticalHeader(), &QHeaderView::sectionClicked, [=](int index){ this->verticalHeaderClick(index); });

    setLayout(mainLayout);
    exec();
}


DbfTableDialog::~DbfTableDialog()
{
}

void DbfTableDialog::addRowClicked()
{

    m_DBFTableWidget->insertRow(m_DBFTableWidget->rowCount());
    labels << QString::number(labels.size());
    m_DBFTableWidget->setVerticalHeaderLabels(labels);
    m_DBFTableWidget->scrollToBottom();

}

void DbfTableDialog::removeRowClicked()
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

void DbfTableDialog::addColClicked()
{
    newColDialog = new DbfNewColDialog();
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

void DbfTableDialog::removeColClicked()
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


void DbfTableDialog::cellChanged(int row, int column)
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

void DbfTableDialog::closeEvent(QCloseEvent *event)
{
    shapeHandler->close();

    QString filepath = QString::fromStdString(shapeHandler->getFilepath());
    QFileInfo filepathInfo(filepath);
    QString file_temp = filepathInfo.absolutePath()+"/"+filepathInfo.baseName()+"_temp.dbf";

    // dbf
    QFile::remove(filepathInfo.absolutePath()+"/"+filepathInfo.baseName()+".dbf");
    QFile::copy(file_temp, filepathInfo.absolutePath()+"/"+filepathInfo.baseName()+".dbf");    // file temp to .dbf
    QFile::remove(file_temp);
    // shp
    QString shp_temp = filepathInfo.absolutePath()+"/"+filepathInfo.baseName()+"_temp.shp";
    QString shx_temp = filepathInfo.absolutePath()+"/"+filepathInfo.baseName()+"_temp.shx";
    if (QFile::exists(shp_temp) && QFile::exists(shx_temp))
    {
        QFile::remove(filepathInfo.absolutePath()+"/"+filepathInfo.baseName()+".shp");
        QFile::copy(shp_temp, filepathInfo.absolutePath()+"/"+filepathInfo.baseName()+".shp");
        QFile::remove(shp_temp);

        QFile::remove(filepathInfo.absolutePath()+"/"+filepathInfo.baseName()+".shx");
        QFile::copy(shx_temp, filepathInfo.absolutePath()+"/"+filepathInfo.baseName()+".shx");
        QFile::remove(shx_temp);
    }

    // re-open shapefile
    shapeHandler->open(shapeHandler->getFilepath());

    QDialog::closeEvent(event);
}


void DbfTableDialog::copyAllClicked()
{
    m_DBFTableWidget->selectAll();
    m_DBFTableWidget->copySelection();
}


void DbfTableDialog::saveChangesClicked()
{
    QString filepath = QString::fromStdString(shapeHandler->getFilepath());
    QFileInfo filepathInfo(filepath);
    QString file_temp = filepathInfo.absolutePath() + "/" + filepathInfo.baseName() + "_temp.dbf";

    QFile::remove(file_temp);   // remove old file_temp

    if (shapeHandler->existRecordDeleted())
    {
        shapeHandler->packSHP(file_temp.toStdString());
        shapeHandler->packDBF(file_temp.toStdString());
        shapeHandler->close();
    }
    else
    {
        shapeHandler->close();
        // copy modified file to file_temp
        QFile::copy(filepathInfo.absolutePath()+"/"+filepathInfo.baseName()+".dbf", file_temp);
    }
    shapeHandler->open(shapeHandler->getFilepath());
}


void DbfTableDialog::horizontalHeaderClick(int index)
{
    m_DBFTableWidget->setSelectionBehavior(QAbstractItemView::SelectColumns);
    m_DBFTableWidget->setCurrentCell(0, index);
}

void DbfTableDialog::verticalHeaderClick(int index)
{
    m_DBFTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_DBFTableWidget->setCurrentCell(index, 0);
}

void DbfTableDialog::menuRequested(const QPoint point)
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


