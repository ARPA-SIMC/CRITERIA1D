#include <QCoreApplication>
#include <QDateTime>

#include "criteria1DProject.h"
#include "commonConstants.h"
#include "utilities.h"
#include <iostream>

// uncomment to execute test
#define TEST_SQLITE


void usage()
{
    std::cout << "CRITERIA-1D water balance" << std::endl
              << "Usage: CRITERIA1D project.ini [firstDate] [lastDate]" << std::endl;
}


int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);

    Criteria1DProject myProject;

    QString appPath = myApp.applicationDirPath() + "/";
    QString settingsFileName;

    if (argc > 1)
    {
        // settings file
        settingsFileName = argv[1];
    }
    else
    {
        QString path;
        if (! searchDataPath(&path)) return -1;
        #ifdef TEST_SQLITE
            settingsFileName = path + "PROJECT/test/test.ini";
        #else
            usage();
            return 1;
        #endif
    }

    if (argc > 2)
    {
        // first simulation date
        QString dateStr = argv[2];

        // check
        myProject.criteriaSimulation.firstSimulationDate = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (! myProject.criteriaSimulation.firstSimulationDate.isValid())
        {
            myProject.logger.writeError("Wrong date format: " + dateStr +"\nRequested format is: YYYY-MM-DD");
            return ERROR_WRONGDATE;
        }
    }

    if (argc > 3)
    {
        // last simulation date
        QString dateStr = argv[3];

        // check
        myProject.criteriaSimulation.lastSimulationDate = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (! myProject.criteriaSimulation.lastSimulationDate.isValid())
        {
            myProject.logger.writeError("Wrong date format: " + dateStr +"\nRequested format is: YYYY-MM-DD");
            return ERROR_WRONGDATE;
        }
    }

    if (settingsFileName.at(0) == ".")
        settingsFileName = appPath + settingsFileName;

    int myResult = myProject.initializeProject(settingsFileName);
    if (myResult != CRIT1D_OK)
    {
        myProject.logger.writeError(myProject.projectError);
        return myResult;
    }

    // computation unit list
    if (! readUnitList(myProject.dbUnitsName, myProject.unitList, myProject.projectError))
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

    myProject.logger.writeInfo("COMPUTE...");

    myResult = myProject.compute();

    myProject.logger.writeInfo("END");

    return myResult;
}

