#include <QCoreApplication>
#include <QDateTime>
#include <iostream>

#include "commonConstants.h"
#include "criteria1DError.h"
#include "criteria1DProject.h"
#include "utilities.h"

// uncomment to execute test
//#define TEST
//#define TEST_GEO
//#define TEST_FIRSTRUN
//#define TEST_RESTART


void usage()
{
    std::cout << "Usage:" << std::endl
              << "CRITERIA1D <project.ini> [firstDate] [lastDate]" << std::endl << std::endl
              << "Date format:" << std::endl
              << "- dates must be in YYYY-MM-DD format" << std::endl
              << "- the default dates are the first and last dates of the weather data tables in the db_meteo file" << std::endl
              << "- in projects with gridded weather data (MySQL), the default lastDate is yesterday" << std::endl
              << "- firstDate and lastDate can be also defined in the project.ini file" << std::endl;

    std::cout << std::flush;
}


int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);
    Crit1DProject myProject;

    std::cout << "CRITERIA-1D agro-hydrological model " << CRITERIA_VERSION << std::endl;

    QString appPath = myApp.applicationDirPath() + "/";
    QString projectFileName;

    if (argc > 1)
    {
        // settings file
        projectFileName = argv[1];
    }
    else
    {
        QString dataPath;
        if (! searchDataPath(&dataPath))
        {
            return -1;
        }
        QString projectPath = dataPath + PATH_PROJECT;

        #ifdef TEST
            projectFileName = projectPath + "test/test.ini";
        #endif

        #ifdef TEST_GEO
            projectFileName = projectPath + "INCOLTO/incolto.ini";
        #endif

        #ifdef TEST_FIRSTRUN
            projectFileName = projectPath + "kiwifruit/kiwifruit.ini";
            myProject.firstSimulationDate = QDate::fromString("1995-01-01", "yyyy-MM-dd");
            myProject.lastSimulationDate = QDate::fromString("2001-05-31", "yyyy-MM-dd");
        #endif

        #ifdef TEST_RESTART
            projectFileName = projectPath + "kiwifruit/kiwifruit_restart.ini";
            myProject.firstSimulationDate = QDate::fromString("2001-06-01", "yyyy-MM-dd");
            myProject.lastSimulationDate = QDate::fromString("2008-12-31", "yyyy-MM-dd");
        #endif

        if (projectFileName.isEmpty())
        {
            usage();
            return 1;
        }
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

    if (projectFileName == "")
    {
        std::cout << "*** WARNING: the project filename is missing\n" << std::endl;
        usage();
        return 1;
    }

    if (projectFileName.at(0) == '.')
        projectFileName = appPath + projectFileName;

    int myResult = myProject.initializeProject(projectFileName);
    if (myResult != CRIT1D_OK)
    {
        myProject.logger.writeError(myProject.projectError);
        return myResult;
    }

    myResult = myProject.computeAllUnits();

    myProject.logger.writeInfo("END");

    return myResult;
}

