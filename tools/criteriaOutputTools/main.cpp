#include <QCoreApplication>
#include <QDir>

#include "commonConstants.h"
#include "criteriaOutputProject.h"
#include "utilities.h"
#include <iostream>

#define TEST

void usage()
{
    std::cout << "\n" << "USAGE: criteriaOutput.exe PRECOMPUTE_DTX|CSV|SHAPEFILE|AGGREGATION project.ini [date]\n";
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

                settingsFileName = myProject.dataPath + "PROJECT/INCOLTO/nitrati.ini";
                dateComputationStr = "2020-08-13";
                //dateComputationStr = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
                operation = "AGGREGATION";
        #else
                usage();
                return 1;
        #endif
    }
    else
    {
        operation = argv[1];
        operation = operation.toUpper();
        if (operation != "PRECOMPUTE_DTX" && operation != "CSV"
            && operation != "SHAPEFILE" && operation != "AGGREGATION")
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

    // check date
    QDate dateComputation = QDate::fromString(dateComputationStr, "yyyy-MM-dd");
    if (! dateComputation.isValid())
    {
        myProject.logger.writeError("Wrong date format. Requested format is: YYYY-MM-DD");
        return ERROR_WRONGDATE;
    }

    if (settingsFileName.left(1) == ".")
        settingsFileName = appPath + settingsFileName;

    int myResult = myProject.initializeProject(settingsFileName, dateComputation);
    if (myResult != CRIT3D_OK)
    {
        myProject.logger.writeError(myProject.projectError);
        return myResult;
    }
    myProject.logger.writeInfo("computation date: " + dateComputationStr);


    if (operation == "PRECOMPUTE_DTX")
    {
        myResult = myProject.precomputeDtx();
    }
    else if (operation == "CSV")
    {
        myResult = myProject.createCsvFile();
    }
    else if (operation == "SHAPEFILE")
    {
        myResult = myProject.createShapeFile();
    }
    else if (operation == "AGGREGATION")
    {
        myResult = myProject.createAggregationFile();
    }
    else
    {
        myProject.logger.writeError("Wrong operation: " + operation);
        usage();
        return 1;
    }

    if (myResult != CRIT3D_OK)
    {
        myProject.logger.writeError(myProject.projectError);
        return myResult;
    }
    else
    {
        myProject.logger.writeInfo("END");
        return CRIT3D_OK;
    }
}

