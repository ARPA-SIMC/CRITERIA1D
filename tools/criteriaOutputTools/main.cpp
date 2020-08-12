#include <QCoreApplication>
#include <QDir>

#include "commonConstants.h"
#include "criteriaOutputProject.h"
#include "utilities.h"
#include <iostream>

#define TEST

void usage()
{
    std::cout << "\n" << "Usage: criteriaOutput.exe CSV|SHAPEFILE project.ini [date]\n";
}


int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);
    CriteriaOutputProject myProject;

    QString appPath = myApp.applicationDirPath() + "/";
    QString settingsFileName, dateComputationStr, operation;

    if (argc <= 2)
    {
        #ifdef TEST
                if (! searchDataPath(&myProject.dataPath)) return -1;
                settingsFileName = myProject.dataPath + "PROJECT/INCOLTO/incolto_output.ini";
                dateComputationStr = "2020-08-11";
                //dateComputationStr = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
                operation = "CSV";
        #else
                usage();
                return 1;
        #endif
    }
    else
    {
        operation = argv[1];
        operation = operation.toUpper();
        if (operation != "CSV" && operation != "SHAPEFILE")
        {
            myProject.logger.writeError("Wrong parameter: " + operation);
            usage();
            return 1;
        }

        settingsFileName = argv[2];
        if (settingsFileName.right(3) != "ini")
        {
            myProject.logger.writeError("Wrong file .ini: " + settingsFileName);
            usage();
            return 1;
        }

        if (argc > 3)
        {
            dateComputationStr = argv[3];
        }
        else
        {
            dateComputationStr = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
        }
    }

    bool isCsv = (operation == "CSV");

    // check date
    QDate dateComputation = QDate::fromString(dateComputationStr, "yyyy-MM-dd");
    if (! dateComputation.isValid())

    {
        myProject.logger.writeError("Wrong date format. Requested format is: YYYY-MM-DD");
        return ERROR_WRONGDATE;
    }

    if (settingsFileName.left(1) == ".")
        settingsFileName = appPath + settingsFileName;

    int myResult = myProject.initializeProject(settingsFileName, dateComputation, isCsv);
    if (myResult != CRIT3D_OK)
    {
        myProject.logger.writeError(myProject.projectError);
        return myResult;
    }

    myProject.logger.writeInfo("computation date: " + dateComputationStr);

    if (operation == "CSV")
    {
        // computation unit list
        if (! loadUnitList(myProject.dbUnitsName, myProject.unitList, myProject.projectError))
        {
            myProject.logger.writeError(myProject.projectError);
            return ERROR_READ_UNITS;
        }
        myProject.logger.writeInfo("Query result: " + QString::number(myProject.unitList.size()) + " distinct computation units.");

        // initialize output
        if (!myProject.initializeCsvOutputFile())
        {
            myProject.logger.writeError(myProject.projectError);
            return ERROR_PARSERCSV;
        }

        // initialize output
        myResult = myProject.writeCsvOutput();
        if (myResult != CRIT3D_OK)
        {
            myProject.logger.writeError(myProject.projectError);
            return myResult;
        }
    }

    if (operation == "SHAPEFILE")
    {
        if (! myProject.createShapeFile())
            return ERROR_SHAPEFILE;
    }

    myProject.logger.writeInfo("END");

    return CRIT3D_OK;
}

