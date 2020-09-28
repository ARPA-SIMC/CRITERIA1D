#ifndef CRITERIAOUTPUTMAP_H
#define CRITERIAOUTPUTMAP_H

#include <QString>
#include <QSqlDatabase>
//#include "criteria1DUnit.h"

class CriteriaOutputMap
{
public:
    CriteriaOutputMap();
    bool isProjectLoaded;

    QString path;
    QString configFileName;
    QString projectName;
    QString ucmFileName;
    QString fieldListFileName;

    QString dbUnitsName;
    QString dbDataName;
    QString dbCropName;
    QString dbDataHistoricalName;

    QSqlDatabase dbCrop;
    QSqlDatabase dbData;
    QSqlDatabase dbDataHistorical;

    int nrUnits;
    //std::vector<Crit1DUnit> unitList;
    //CriteriaOutputVariable outputVariable;

    QString error;
    QString outputFile;

    void initialize();
    void closeProject();
    bool initializeProject(QString settingsFileName);
    bool readSettings();
};

#endif // CRITERIAOUTPUTMAP_H
