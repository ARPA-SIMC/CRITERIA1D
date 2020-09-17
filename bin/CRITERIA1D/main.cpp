#include <QCoreApplication>
#include <QDateTime>

#include "criteria1DProject.h"
#include "commonConstants.h"
#include "utilities.h"
#include <iostream>

//#define TEST_HISTORICAL
//#define TEST_TODAY
//#define TEST_PAST
//#define TEST_SQLITE


int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);

    Criteria1DProject myProject;

    QString appPath = myApp.applicationDirPath() + "/";
    QString settingsFileName, computationDateStr;

    if (argc > 1)
        settingsFileName = argv[1];
    else
    {
        QString path;
        if (! searchDataPath(&path)) return -1;
        #ifdef TEST_SQLITE
            settingsFileName = path + "PROJECT/kiwifruit/kiwifruit.ini";
        #else
            #ifdef TEST_HISTORICAL
                settingsFileName = path + "PROJECT/INCOLTO/Incolto_storico.ini";
            #else
                #if defined(TEST_TODAY) || defined(TEST_PAST)
                    settingsFileName = path + "PROJECT/INCOLTO/Incolto.ini";
                #else
                    std::cout << "USAGE: CRITERIA1D project.ini [date]\n";
                    return ERROR_SETTINGS_MISSING;
                #endif
            #endif
        #endif
    }

    if (argc > 2)
    {
        computationDateStr = argv[2];
    }
    else
    {
        #ifdef TEST_HISTORICAL
            computationDateStr = "2020-01-01";
        #else
            #ifdef TEST_PAST
                computationDateStr = "2020-08-13";
            #else
                computationDateStr = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
            #endif
        #endif
    }

    // check date
    QDate computationDate = QDate::fromString(computationDateStr, "yyyy-MM-dd");
    if (! computationDate.isValid())
    {
        myProject.logger.writeError("Wrong date format: " + computationDateStr +"\nRequested format is: YYYY-MM-DD");
        return ERROR_WRONGDATE;
    }

    if (settingsFileName.left(1) == ".")
        settingsFileName = appPath + settingsFileName;

    // initialize project
    int myResult = myProject.initializeProject(settingsFileName);
    if (myResult != CRIT3D_OK)
    {
        myProject.logger.writeError(myProject.projectError);
        return myResult;
    }

    myProject.logger.writeInfo("Computation date: " + computationDateStr);

    // date of last observed data: yesterday
    myProject.criteriaSimulation.lastObservedDate = computationDate.addDays(-1);

    // computation unit list
    if (! loadUnitList(myProject.dbUnitsName, myProject.unitList, myProject.projectError))
    {
        myProject.logger.writeError(myProject.projectError);
        return ERROR_READ_UNITS;
    }
    myProject.logger.writeInfo("Query result: " + QString::number(myProject.unitList.size()) + " distinct computation units.");

    // initialize output (seasonal forecast)
    if (myProject.criteriaSimulation.isSeasonalForecast)
    {
        if (!myProject.initializeCsvOutputFile())
            return ERROR_DBOUTPUT;
    }

    // Computation
    myProject.logger.writeInfo("Computation...");

    myResult = myProject.compute();

    myProject.logger.writeInfo("END");

    return myResult;
}

