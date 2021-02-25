#include <QCoreApplication>
#include <QFile>
//#include <QDir>
#include "dbMeteoGrid.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Crit3DMeteoGridDbHandler meteoGrid;
    QString myError;
    Logger logger;

    //logger.writeInfo(QDir::currentPath());
    QString dbMeteoName = "../../../../DATA/METEOGRID/DBGridXML_ERG5_ecmwf_test.xml";
    logger.writeInfo ("Meteo DB: " + dbMeteoName);

    if (! QFile(dbMeteoName).exists())
    {
        logger.writeInfo ("Meteo points DB file doesn't exist");
        return 1;
    }

    if (! meteoGrid.parseXMLGrid(dbMeteoName, &myError))
    {
        logger.writeInfo ("parseXMLGrid error");
        return 1;
    }
    meteoGrid.openDatabase(&myError, "grid");
    meteoGrid.loadCellProperties(&myError);
    //meteoGrid.addMemberNrColumn(&myError); //DONE

    return true;
}
