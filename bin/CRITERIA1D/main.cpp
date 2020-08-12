#include <QCoreApplication>
#include <QDir>
#include <QDateTime>

#include "criteria1DProject.h"
#include "commonConstants.h"
#include "utilities.h"

//#define TEST_HISTORICAL
#define TEST_TODAY
//#define TEST_PAST
//#define TEST_KIWIFRUIT


int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);

    Criteria1DProject myProject;

    QString appPath = myApp.applicationDirPath() + "/";
    QString settingsFileName, dateOfForecast;

    if (argc > 1)
        settingsFileName = argv[1];
    else
    {
        QString path;
        if (! searchDataPath(&path)) return -1;
        #ifdef TEST_KIWIFRUIT
            settingsFileName = path + "PROJECT/kiwifruit/kiwifruit.ini";
        #else
            #ifdef TEST_HISTORICAL
                //settingsFileName = path + "PROJECT/INCOLTO/Incolto_cut_historical_xml.ini";
                settingsFileName = path + "PROJECT/INCOLTO/Incolto_historical_xml.ini";
            #else
                #if defined(TEST_TODAY) || defined(TEST_PAST)
                    //settingsFileName = path + "PROJECT/INCOLTO/Incolto_cut_xml.ini";
                    settingsFileName = path + "PROJECT/INCOLTO/Incolto.ini";
                #else
                    myProject.logInfo("USAGE: CRITERIA1D project.ini [date]\n");
                    return ERROR_SETTINGS_MISSING;
                #endif
            #endif
        #endif
    }

    if (argc > 2)
    {
        dateOfForecast = argv[2];
    }
    else
    {
        #ifdef TEST_HISTORICAL
            dateOfForecast = "2020-01-01";
        #else
            #ifdef TEST_PAST
                dateOfForecast = "2020-07-01";
            #else
                dateOfForecast = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
            #endif
        #endif
    }

    // check date
    QDate forecastDate = QDate::fromString(dateOfForecast, "yyyy-MM-dd");
    if (! forecastDate.isValid())
    {
        myProject.logError("Wrong date format: " + dateOfForecast +"\nRequested format is: YYYY-MM-DD");
        return ERROR_WRONGDATE;
    }

    if (settingsFileName.left(1) == ".")
        settingsFileName = appPath + settingsFileName;

    // initialize project
    int myResult = myProject.initializeProject(settingsFileName);
    if (myResult != CRIT3D_OK)
    {
        myProject.logError();
        return myResult;
    }
    // date of last observed data: yesterday
    myProject.criteriaSimulation.lastObservedDate = forecastDate.addDays(-1);

    // computation unit list
    if (! loadUnitList(myProject.dbUnitsName, myProject.unitList, myProject.projectError))
    {
        myProject.logError();
        return ERROR_READ_UNITS;
    }
    myProject.logInfo("\nQuery result: " + QString::number(myProject.unitList.size()) + " distinct computation units.\n");

    // initialize output (seasonal forecast)
    if (myProject.criteriaSimulation.isSeasonalForecast)
    {
        if (!myProject.initializeCsvOutputFile())
            return ERROR_DBOUTPUT;
    }

    // COMPUTE
    myResult = myProject.compute();
    myProject.logInfo("\nEND");

    return myResult;
}

