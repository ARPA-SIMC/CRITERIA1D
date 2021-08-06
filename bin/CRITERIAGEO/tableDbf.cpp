#include "tableDbf.h"
#include <QClipboard>
#include <QApplication>

TableDbf::TableDbf()
{

}

void TableDbf::keyPressEvent(QKeyEvent *event){
    QModelIndexList selectedRows = selectionModel()->selectedRows();
    // at least one entire row selected
    if(!selectedRows.isEmpty()){
        if(event->key() == Qt::Key_Insert)
            model()->insertRows(selectedRows.at(0).row(),
                                selectedRows.size());
        else if(event->key() == Qt::Key_Delete)
            model()->removeRows(selectedRows.at(0).row(),
                                selectedRows.size());
    }
    // at least one cell selected
    if(!selectedIndexes().isEmpty()){
        if(event->key() == Qt::Key_Delete){
            foreach (QModelIndex index, selectedIndexes())
                model()->setData(index, QString());
        }
        else if(event->matches(QKeySequence::Copy))
        {
            copySelection();
        }
        else if(event->matches(QKeySequence::Paste))
        {

            QString text = QApplication::clipboard()->text();
            #if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                QList<QString> rowContents = text.split("\n", Qt::SkipEmptyParts);
            #else
                QList<QString> rowContents = text.split("\n", QString::SkipEmptyParts);
            #endif

            QModelIndex initIndex = selectedIndexes().at(0);
            auto initRow = initIndex.row();
            auto initCol = initIndex.column();

            for (auto i = 0; i < rowContents.size(); ++i)
            {
                QStringList columnContents = rowContents.at(i).split("\t");
                if (columnContents.size() == 1)
                {
                    model()->setData(model()->index(initRow + i, initCol), columnContents[0]);
                }
                else
                {
                    for (auto j = 0; j < columnContents.size(); ++j)
                    {
                        this->setSortingEnabled(false);
                        this->blockSignals(true);
                        model()->setData(model()->index(initRow + i, initCol + j), columnContents[j]);
                    }
                    this->blockSignals(false);
                    this->setSortingEnabled(true);

                }
            }
         }
         else

            QTableView::keyPressEvent(event);
    }
}

void TableDbf::copySelection()
{
    QString text;
    QItemSelectionRange range = selectionModel()->selection().first();
    QStringList header;
    for (auto j = range.left(); j <= range.right(); ++j)
    {
        header << this->horizontalHeaderItem(j)->text();
    }
    text += header.join("\t");
    text += "\n";
    for (auto i = range.top(); i <= range.bottom(); ++i)
    {
        QStringList rowContents;
        for (auto j = range.left(); j <= range.right(); ++j)
            rowContents << model()->index(i,j).data().toString();
        text += rowContents.join("\t");
        text += "\n";
    }
    QApplication::clipboard()->setText(text);
}

