#include <QCoreApplication>
#include <QDateTime>
#include <iostream>

#include "commonConstants.h"
#include "criteria1DError.h"
#include "criteria1DProject.h"
#include "utilities.h"

// uncomment to execute test
//#define TEST_SQLITE
//#define TEST_FIRSTRUN
//#define TEST_RESTART


void usage()
{
    std::cout << "CRITERIA-1D soil water balance" << std::endl
              << "\nUsage:" << std::endl
              << "CRITERIA1D <project.ini> [lastDate]" << std::endl
              << "CRITERIA1D <project.ini> [firstDate] [lastDate]" << std::endl
              << "\ndate must be in YYYY-MM-DD format" << std::endl
              << "Point projects (SQLite): default are the first and last date in the meteo table" << std::endl
              << "Grid projects (MySQL): default lastDate is yesterday" << std::endl
              << "firstDate and lastDate can be also defined in project.ini" << std::endl << std::endl;
}


int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);

    Crit1DProject myProject;

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

        #ifdef TEST_FIRSTRUN
            myProject.firstSimulationDate = QDate::fromString("1995-01-01", "yyyy-MM-dd");
            myProject.lastSimulationDate = QDate::fromString("2008-05-31", "yyyy-MM-dd");
            //myProject.lastSimulationDate = QDate::fromString("2007-12-31", "yyyy-MM-dd");
        #endif

        #ifdef TEST_RESTART
            //myProject.firstSimulationDate = QDate::fromString("2008-01-01", "yyyy-MM-dd");
            myProject.firstSimulationDate = QDate::fromString("2008-06-01", "yyyy-MM-dd");
            myProject.lastSimulationDate = QDate::fromString("2008-12-31", "yyyy-MM-dd");
        #endif
    }


    if (argc == 3)
    {
        // last simulation date
        QString dateStr = argv[2];

        // check
        myProject.lastSimulationDate = QDate::fromString(dateStr, "yyyy-MM-dd").addDays(-1);
        if (! myProject.lastSimulationDate.isValid())
        {
            myProject.logger.writeError("Wrong date format: " + dateStr +"\nRequested format is: YYYY-MM-DD");
            return ERROR_WRONGDATE;
        }
    }

    if (argc == 4)
    {
        // first simulation date
        QString dateStr = argv[2];

        // check
        myProject.firstSimulationDate = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (! myProject.firstSimulationDate.isValid())
        {
            myProject.logger.writeError("Wrong date format: " + dateStr +"\nRequested format is: YYYY-MM-DD");
            return ERROR_WRONGDATE;
        }

        // last simulation date
        dateStr = argv[3];

        // check
        myProject.lastSimulationDate = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (! myProject.lastSimulationDate.isValid())
        {
            myProject.logger.writeError("Wrong date format: " + dateStr +"\nRequested format is: YYYY-MM-DD");
            return ERROR_WRONGDATE;
        }
    }

    if (settingsFileName.at(0) == '.')
        settingsFileName = appPath + settingsFileName;

    int myResult = myProject.initializeProject(settingsFileName);
    if (myResult != CRIT1D_OK)
    {
        myProject.logger.writeError(myProject.projectError);
        return myResult;
    }

    myProject.logger.writeInfo("COMPUTE...");

    myResult = myProject.computeAllUnits();

    myProject.logger.writeInfo("END");

    return myResult;
}

