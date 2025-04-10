#ifndef DIALOGSHAPEPROPERTIES_H
#define DIALOGSHAPEPROPERTIES_H

#include <QtWidgets>
#ifndef SHAPEHANDLER_H
    #include "shapeHandler.h"
#endif

    class DialogShapeProperties : public QDialog
    {
        Q_OBJECT

    public:
        explicit DialogShapeProperties(Crit3DShapeHandler* shapeHandler, QString filename);

    private:
        Crit3DShapeHandler* shapeHandler;
        QLineEdit* shapeType;
        QLineEdit* shapeEntityCount;
        QLineEdit* fieldsCount;
        QTextEdit* shapeData;
        QTreeWidget* treeWidget;

    private slots:
        void onSelectShape(QTreeWidgetItem *item, int column);
    };

#endif // DIALOGSHAPEPROPERTIES_H
