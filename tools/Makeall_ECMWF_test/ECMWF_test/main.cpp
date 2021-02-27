#include <QCoreApplication>
#include <QFile>
//#include <QDir>
#include "dbMeteoGrid.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    Crit3DMeteoGridDbHandler meteoGridLami;
    QString myError;
    Logger logger;

    //logger.writeInfo(QDir::currentPath());
    //QString dbMeteoName = "../../../../DATA/METEOGRID/DBGridXML_ERG5_ecmwf_test.xml";
/*
    QString dbLamiMeteoName = "/home/laura/PRAGA/DATA/METEOGRID/DBGridXML_ERG5_LamiForecast.xml";
    logger.writeInfo ("Meteo DB: " + dbLamiMeteoName);

    if (! QFile(dbLamiMeteoName).exists())
    {
        logger.writeInfo ("Meteo points DB file doesn't exist");
        return 1;
    }

    if (! meteoGridLami.parseXMLGrid(dbLamiMeteoName, &myError))
    {
        logger.writeInfo ("parseXMLGrid error");
        return 1;
    }
    meteoGridLami.openDatabase(&myError, "gridLami");
    meteoGridLami.loadCellProperties(&myError);
*/
    // load data LAMI
    std::string id;
    QDate firstDate(2020,06,01);
    QDate lastDate(2020,06,30);
/*
    QDateTime firstDateTime = QDateTime(firstDate, QTime(1,0));
    QDateTime lastDateTime = QDateTime(lastDate.addDays(1), QTime(0,0));
    for (int row = 0; row < meteoGridLami.gridStructure().header().nrRows; row++)
    {
        for (int col = 0; col < meteoGridLami.gridStructure().header().nrCols; col++)
        {
            if (meteoGridLami.meteoGrid()->getMeteoPointActiveId(row, col, &id))
            {
                meteoGridLami.loadGridDailyData(&myError, QString::fromStdString(id), firstDate, lastDate); //DONE
            }
        }
    }
    meteoGridLami.closeDatabase();
    // copy data
    */
    QString dbMeteoName = "../../../../DATA/METEOGRID/DBGridXML_ERG5_ecmwf_test.xml";
    Crit3DMeteoGridDbHandler ecmwfMeteoGrid;
    //ecmwfMeteoGrid = meteoGridLami;

    logger.writeInfo ("Meteo DB: " + dbMeteoName);

    if (! QFile(dbMeteoName).exists())
    {
        logger.writeInfo ("Meteo points DB file doesn't exist");
        return 1;
    }

    if (! ecmwfMeteoGrid.parseXMLGrid(dbMeteoName, &myError))
    {
        logger.writeInfo ("parseXMLGrid error");
        return 1;
    }
    ecmwfMeteoGrid.openDatabase(&myError, "gridECMWF");
    ecmwfMeteoGrid.loadCellProperties(&myError);
    QList<meteoVariable> meteoVariableList = {dailyAirTemperatureMin,dailyAirTemperatureMax,dailyAirTemperatureAvg,dailyPrecipitation};
/*
    for (int row = 0; row < meteoGridLami.gridStructure().header().nrRows; row++)
    {
        for (int col = 0; col < meteoGridLami.gridStructure().header().nrCols; col++)
        {
            if (meteoGridLami.meteoGrid()->getMeteoPointActiveId(row, col, &id))
            {
                ecmwfMeteoGrid.loadGridDailyData(&myError, QString::fromStdString(id), firstDate, lastDate);
            }
        }
    }


    for (int row = 0; row < ecmwfMeteoGrid.meteoGrid()->gridStructure().header().nrRows; row++)
    {
        for (int col = 0; col < ecmwfMeteoGrid.meteoGrid()->gridStructure().header().nrCols; col++)
        {
            if (ecmwfMeteoGrid.meteoGrid()->getMeteoPointActiveId(row, col, &id))
            {
                ecmwfMeteoGrid.saveFirstEnsable(&myError, QString::fromStdString(id), row, col, firstDate, lastDate, meteoVariableList);
            }
        }
    }
*/
    return true;
}
