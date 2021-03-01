#include <QCoreApplication>
#include <QFile>
//#include <QDir>
#include "dbMeteoGrid.h"
#include "logger.h"
#include "utilities.h"
#include <stdio.h>
#include <random>
#include <QRandomGenerator>

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
    int nDays = firstDate.daysTo(lastDate)+1;

    for (int row = 0; row < ecmwfMeteoGrid.gridStructure().header().nrRows; row++)
    {
        for (int col = 0; col < ecmwfMeteoGrid.gridStructure().header().nrCols; col++)
        {
            if (ecmwfMeteoGrid.meteoGrid()->getMeteoPointActiveId(row, col, &id))
            {
                ecmwfMeteoGrid.loadGridDailyDataEnsemble(&myError, QString::fromStdString(id), 1, firstDate, lastDate);
                for (int j = 0; j<nDays; j++)
                {
                    float tmin = ecmwfMeteoGrid.meteoGrid()->meteoPointPointer(row,col)->getMeteoPointValueD(getCrit3DDate(firstDate.addDays(j)), dailyAirTemperatureMin);
                    float tmax = ecmwfMeteoGrid.meteoGrid()->meteoPointPointer(row,col)->getMeteoPointValueD(getCrit3DDate(firstDate.addDays(j)), dailyAirTemperatureMax);
                    float tavg = ecmwfMeteoGrid.meteoGrid()->meteoPointPointer(row,col)->getMeteoPointValueD(getCrit3DDate(firstDate.addDays(j)), dailyAirTemperatureAvg);
                    float trange = tmax-tmin;
                    int deltaTemp = 2 + j/3;
                    float prec = ecmwfMeteoGrid.meteoGrid()->meteoPointPointer(row,col)->getMeteoPointValueD(getCrit3DDate(firstDate.addDays(j)), dailyPrecipitation);
                    float deltaPrec = 0.5 + j*0.05;
                    QRandomGenerator* gen = QRandomGenerator::system();
                    for (int memberNr=2; memberNr<=51; memberNr++)
                    {
                        // temp
                        double lowest = -1;
                        double highest = 1;
                        double randTavg = gen->generateDouble()*2*highest + lowest;
                        if (randTavg > highest)
                        {
                            randTavg = highest;
                        }
                        tavg = tavg + randTavg*deltaTemp;
                        lowest = -0.5;
                        highest = 0.5;
                        double randTrange = gen->generateDouble()*2*highest + lowest;
                        if (randTrange > highest)
                        {
                            randTrange = highest;
                        }
                        trange = trange + randTrange*deltaTemp;
                        if (trange < 0)
                        {
                            trange = 0.5;
                        }
                        tmin = tavg - trange/2;
                        tmax = tavg+trange/2;
                        ecmwfMeteoGrid.meteoGrid()->meteoPointPointer(row,col)->setMeteoPointValueD(getCrit3DDate(firstDate.addDays(j)), dailyAirTemperatureMin, tmin);
                        ecmwfMeteoGrid.meteoGrid()->meteoPointPointer(row,col)->setMeteoPointValueD(getCrit3DDate(firstDate.addDays(j)), dailyAirTemperatureMax, tmax);
                        ecmwfMeteoGrid.meteoGrid()->meteoPointPointer(row,col)->setMeteoPointValueD(getCrit3DDate(firstDate.addDays(j)), dailyAirTemperatureAvg, tavg);

                        // prec
                        if (prec>0.2)
                        {
                            // rainy day
                            lowest = -deltaPrec;
                            highest = deltaPrec;
                            double randPrec = gen->generateDouble()*2*highest + lowest;
                            if (randTrange > highest)
                            {
                                randTrange = highest;
                            }
                            prec = prec * (1 + randPrec);
                        }
                        else
                        {
                            // not rainy day
                            lowest = -4*deltaPrec;
                            highest = deltaPrec;
                            double randPrec = gen->generateDouble()*5*highest + lowest;
                            if (randTrange > highest)
                            {
                                randTrange = highest;
                            }
                            prec = 10*randPrec;
                        }
                        ecmwfMeteoGrid.meteoGrid()->meteoPointPointer(row,col)->setMeteoPointValueD(getCrit3DDate(firstDate.addDays(j)), dailyPrecipitation, prec);

                        ecmwfMeteoGrid.saveCellGridDailyDataEnsemble(&myError, QString::fromStdString(id), row, col, firstDate, lastDate, meteoVariableList, memberNr);
                    }
                }
            }
        }
    }
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
    ecmwfMeteoGrid.closeDatabase();
    return true;
}
