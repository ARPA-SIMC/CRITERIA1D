#ifndef DBMETEOCRITERIA1D_H
#define DBMETEOCRITERIA1D_H

    #define MAX_MISSING_TOT_DAYS 30
    #define MAX_MISSING_CONSECUTIVE_DAYS_T 1
    #define MAX_MISSING_CONSECUTIVE_DAYS_PREC 7

    #include <QString>

    class QSqlDatabase;
    class QSqlQuery;
    class QDate;
    class Crit3DMeteoPoint;

    bool openDbMeteo(const QString &dbName, QSqlDatabase &dbMeteo, QString &errorStr);
    bool getMeteoPointList(const QSqlDatabase &dbMeteo, QList<QString> &idMeteoList, QString &errorStr);

    bool getYearList(const QSqlDatabase &dbMeteo, const QString &table, QList<QString> &yearList, QString &errorStr);

    bool getLatLonFromIdMeteo(const QSqlDatabase &dbMeteo, const QString &idMeteo,
                              QString &lat, QString &lon, QString &errorStr);

    bool updateLatFromIdMeteo(const QSqlDatabase &dbMeteo, const QString &idMeteo, const QString &lat, QString &errorStr);

    QString getTableNameFromIdMeteo(const QSqlDatabase &dbMeteo, const QString &idMeteo, QString &errorStr);

    bool checkYearDbMeteo(const QSqlDatabase &dbMeteo, const QString &table, const QString &year, QString &errorStr);

    bool checkYearMeteoGridFixedFields(const QSqlDatabase &dbMeteo, const QString &tableD, const QString &fieldTime,
                                       const QString &fieldTmin, const QString &fieldTmax, const QString &fieldPrec,
                                       const QString &yearStr, QString &errorStr);

    bool checkYearMeteoGrid(const QSqlDatabase &dbMeteo, const QString &tableD, const QString &fieldTime,
                            int varCodeTmin, int varCodeTmax, int varCodePrec, const QString &yearStr, QString &error);

    bool getLastDateDbMeteo(const QSqlDatabase &dbMeteo, const QString &table, const QString &year,
                            QDate &lastDate, QString &errorStr);

    bool getLastDateGrid(const QSqlDatabase &dbMeteo, const QString &table, const QString &fieldTime,
                         const QString &year, QDate &lastDate, QString &errorStr);

    bool fillDailyTempPrecCriteria1D(const QSqlDatabase &dbMeteo, const QString &table,
                                     Crit3DMeteoPoint &meteoPoint, int year, QString &errorStr);

    bool readDailyDataCriteria1D(QSqlQuery &query, Crit3DMeteoPoint &meteoPoint, int maxNrDays, QString &errorStr);


#endif // DBMETEOCRITERIA1D_H
