#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDate>
#include <QUuid>

#include "commonConstants.h"
#include "crit3dDate.h"
#include "criteria1DdbMeteo.h"
#include "utilities.h"
#include "meteoPoint.h"


bool openDbMeteo(QString dbName, QSqlDatabase* dbMeteo, QString* error)
{

    *dbMeteo = QSqlDatabase::addDatabase("QSQLITE", QUuid::createUuid().toString());
    dbMeteo->setDatabaseName(dbName);

    if (!dbMeteo->open())
    {
       *error = "Connection with database fail";
       return false;
    }

    return true;
}


bool getMeteoPointList(QSqlDatabase* dbMeteo, QStringList* idMeteoList, QString* error)
{
    // query id_meteo list
    QString queryString = "SELECT id_meteo FROM meteo_locations";
    QSqlQuery query = dbMeteo->exec(queryString);

    query.first();
    if (! query.isValid())
    {
        *error = query.lastError().text();
        return false;
    }

    QString idMeteo;
    do
    {
        getValue(query.value("id_meteo"), &idMeteo);
        if (idMeteo != "")
        {
            idMeteoList->append(idMeteo);
        }
    }
    while(query.next());

    return true;
}


bool getLatLonFromIdMeteo(QSqlDatabase* dbMeteo, QString idMeteo, QString* lat, QString* lon, QString *error)
{
    *error = "";
    QString queryString = "SELECT * FROM meteo_locations WHERE id_meteo='" + idMeteo +"'";

    QSqlQuery query = dbMeteo->exec(queryString);
    query.last();

    if (! query.isValid())
    {
        *error = query.lastError().text();
        return false;
    }

    getValue(query.value("latitude"), lat);
    getValue(query.value("longitude"), lon);

    return true;
}

bool updateLatLonFromIdMeteo(QSqlDatabase* dbMeteo, QString idMeteo, QString lat, QString lon, QString *error)
{
    QSqlQuery qry(*dbMeteo);
    *error = "";
    if (idMeteo.isEmpty())
    {
        *error = "id_meteo missing";
        return false;
    }
    qry.prepare( "UPDATE meteo_locations SET longitude = :longitude, "
                 "latitude = :latitude WHERE id_meteo = :id_meteo");

    qry.bindValue(":longitude", lon);
    qry.bindValue(":latitude", lat);

    qry.bindValue(":id_meteo", idMeteo);

    if( !qry.exec() )
    {
        *error = qry.lastError().text();
        return false;
    }
    return true;
}

bool updateLatFromIdMeteo(QSqlDatabase* dbMeteo, QString idMeteo, QString lat, QString *error)
{
    QSqlQuery qry(*dbMeteo);
    *error = "";
    if (idMeteo.isEmpty())
    {
        *error = "id_meteo missing";
        return false;
    }
    qry.prepare( "UPDATE meteo_locations SET "
                 "latitude = :latitude WHERE id_meteo = :id_meteo");

    qry.bindValue(":latitude", lat);

    qry.bindValue(":id_meteo", idMeteo);

    if( !qry.exec() )
    {
        *error = qry.lastError().text();
        return false;
    }
    return true;
}


QString getTableNameFromIdMeteo(QSqlDatabase* dbMeteo, QString idMeteo, QString *error)
{
    *error = "";
    QString queryString = "SELECT * FROM meteo_locations WHERE id_meteo='" + idMeteo +"'";

    QSqlQuery query = dbMeteo->exec(queryString);
    query.last();

    if (! query.isValid())
    {
        *error = query.lastError().text();
        return "";
    }

    QString table_name;
    getValue(query.value("table_name"), &table_name);

    return table_name;
}


bool getYearList(QSqlDatabase* dbMeteo, QString table, QStringList* yearList, QString *error)
{
    *error = "";
    QString queryString = "SELECT date, strftime('%Y',date) as Year FROM '" + table +"'";

    QSqlQuery query = dbMeteo->exec(queryString);

    query.first();
    if (! query.isValid())
    {
        *error = query.lastError().text();
        return false;
    }

    QString year;
    do
    {
        getValue(query.value("Year"), &year);
        if (year != "" && !yearList->contains(year))
        {
            yearList->append(year);
        }
    }
    while(query.next());

    return true;
}


bool checkYear(QSqlDatabase* dbMeteo, QString table, QString year, QString *error)
{
    *error = "";

    QString TMIN_MIN = "-50.0";
    QString TMIN_MAX = "40.0";

    QString TMAX_MIN = "-40.0";
    QString TMAX_MAX = "50.0";

    QString PREC_MIN = "0.0";

    // count valid temp and prec
    QString queryString = "SELECT COUNT(date) FROM '" + table +"'" + " WHERE strftime('%Y',date) = '" + year+"'";
    queryString = queryString + " AND tmin NOT LIKE '' AND tmax NOT LIKE '' AND prec NOT LIKE ''";
    queryString = queryString + " AND CAST(tmin AS float) >=" + TMIN_MIN + " AND CAST(tmin AS float) <=" + TMIN_MAX;
    queryString = queryString + " AND CAST(tmax AS float) >= " + TMAX_MIN + " AND CAST(tmax AS float) <= " + TMAX_MAX + " AND CAST(prec AS float) >= " + PREC_MIN;

    QSqlQuery query = dbMeteo->exec(queryString);
    query.first();
    if (! query.isValid())
    {
        *error = query.lastError().text();
        return false;
    }
    int count;


    getValue(query.value(0), &count);
    QDate temp(year.toInt(), 1, 1);
    int daysInYear = temp.daysInYear();

    if (count < (daysInYear-MAX_MISSING_TOT_DAYS))
    {
        *error = "incomplete year, valid data missing more than MAX_MISSING_DAYS";
        return false;
    }

    // check consecutive missing days (1 missing day allowed for temperature)

    queryString = "SELECT * FROM '" + table +"'" + "WHERE strftime('%Y',date) = '" + year +"'";
    query = dbMeteo->exec(queryString);

    query.first();
    if (! query.isValid())
    {
        *error = query.lastError().text();
        return false;
    }

    QDate date;
    QDate previousDate(year.toInt()-1, 12, 31);
    QDate lastDate(year.toInt(), 12, 31);
    float tmin = NODATA;
    float tmax = NODATA;
    float prec = NODATA;
    float tmin_min = TMIN_MIN.toFloat();
    float tmin_max = TMIN_MAX.toFloat();

    float tmax_min = TMAX_MIN.toFloat();
    float tmax_max = TMAX_MAX.toFloat();
    float prec_min = PREC_MIN.toFloat();

    int invalidTemp = 0;
    int invalidPrec = 0;

    do
    {
        getValue(query.value("date"), &date);
        getValue(query.value("tmin"), &tmin);
        getValue(query.value("tmax"), &tmax);
        getValue(query.value("prec"), &prec);
        // 2 days missing
        if (previousDate.daysTo(date) > (MAX_MISSING_CONSECUTIVE_DAYS_T+1))
        {
            *error = "incomplete year, missing more than 1 consecutive days";
            return false;
        }
        // 1 day missing, the next one invalid temp
        if ( (previousDate.daysTo(date) == (MAX_MISSING_CONSECUTIVE_DAYS_T+1)) && (tmin < tmin_min || tmin > tmin_max || tmax < tmax_min || tmax > tmax_max ) )
        {
            *error = "incomplete year, missing valid data (temp) more than 1 consecutive days";
            return false;
        }
        // no day missing, check valid temp
        if (tmin < tmin_min || tmin > tmin_max || tmax < tmax_min || tmax > tmax_max )
        {
            invalidTemp = invalidTemp + 1;
            if (invalidTemp > 1)
            {
                *error = "incomplete year, missing valid data (temp) more than 1 consecutive days";
                return false;
            }
        }
        else
        {
            invalidTemp = 0;
        }

        // check valid prec
        if (prec < prec_min )
        {
            invalidPrec = invalidPrec + 1;
            // 7 day missing, the next one invalid temp
            if ( invalidPrec + previousDate.daysTo(date) > MAX_MISSING_CONSECUTIVE_DAYS_PREC +1 )
            {
                 *error = "incomplete year, missing valid data (prec) more than 7 consecutive days";
                 return false;
            }
        }
        else
        {
            invalidPrec = 0;
        }
        previousDate = date;

    }
    while(query.next());

    // check last day (temp)
    if (date.daysTo(lastDate) > MAX_MISSING_CONSECUTIVE_DAYS_T || (date.daysTo(lastDate) == MAX_MISSING_CONSECUTIVE_DAYS_T && invalidTemp > 0) )
    {
        *error = "incomplete year, missing more than 1 consecutive days (temp)";
        return false;
    }

    // check last day (prec)
    if (date.daysTo(lastDate) > MAX_MISSING_CONSECUTIVE_DAYS_PREC || (date.daysTo(lastDate) == MAX_MISSING_CONSECUTIVE_DAYS_PREC && invalidPrec > 0) )
    {
        *error = "incomplete year, missing more than 1 consecutive days (prec)";
        return false;
    }

    return true;
}


bool fillDailyTempPrecCriteria1D(QSqlDatabase* dbMeteo, QString table, Crit3DMeteoPoint *meteoPoint, QString validYear, QString *error)
{
    *error = "";

    QString queryString = "SELECT * FROM '" + table +"'" + " WHERE strftime('%Y',date) = '" + validYear +"'";
    QSqlQuery query = dbMeteo->exec(queryString);

    query.first();
    if (! query.isValid())
    {
        *error = query.lastError().text();
        return false;
    }

    QDate date;
    QDate previousDate(validYear.toInt()-1, 12, 31);
    QDate lastDate(validYear.toInt(), 12, 31);
    float tmin = NODATA;
    float tmax = NODATA;
    float tavg = NODATA;
    float prec = NODATA;

    const float tmin_min = -50;
    const float tmin_max = 40;

    const float tmax_min = -40;
    const float tmax_max = 50;


    do
    {
        getValue(query.value("date"), &date);
        getValue(query.value("tmin"), &tmin);
        getValue(query.value("tmax"), &tmax);
        getValue(query.value("prec"), &prec);

        if (tmin < tmin_min || tmin > tmin_max)
        {
            tmin = NODATA;
        }
        if (tmax < tmax_min || tmax > tmax_max)
        {
            tmax = NODATA;
        }
        if (tmin == NODATA || tmax == NODATA)
        {
            tavg = NODATA;
        }
        else
        {
            tavg = (tmin + tmax) * 0.5f;
        }
        if (prec < 0)
        {
            prec = NODATA;
        }
        meteoPoint->setMeteoPointValueD(getCrit3DDate(date), dailyAirTemperatureMin, tmin);
        meteoPoint->setMeteoPointValueD(getCrit3DDate(date), dailyAirTemperatureMax, tmax);
        meteoPoint->setMeteoPointValueD(getCrit3DDate(date), dailyAirTemperatureAvg, tavg);
        meteoPoint->setMeteoPointValueD(getCrit3DDate(date), dailyPrecipitation, prec);
    }
    while(query.next());

    QDate firstDate(validYear.toInt(), 1, 1);
    int daysInYear = firstDate.daysInYear();
    float prevTmin = NODATA;
    float prevTmax = NODATA;
    float nextTmin = NODATA;
    float nextTmax = NODATA;

    // fill NODATA values with average values of day before and next.
    for (int i = 0; i < daysInYear; i++)
    {
        date = firstDate.addDays(i);
        tmin = meteoPoint->getMeteoPointValueD(getCrit3DDate(date), dailyAirTemperatureMin);
        tmax = meteoPoint->getMeteoPointValueD(getCrit3DDate(date), dailyAirTemperatureMax);
        tavg = meteoPoint->getMeteoPointValueD(getCrit3DDate(date), dailyAirTemperatureAvg);
        if (tmin == NODATA)
        {
            if (i!=0 && i!=(daysInYear-1))
            {
                prevTmin = meteoPoint->getMeteoPointValueD(getCrit3DDate(date.addDays(-1)), dailyAirTemperatureMin);
                nextTmin = meteoPoint->getMeteoPointValueD(getCrit3DDate(date.addDays(1)), dailyAirTemperatureMin);
                tmin = (prevTmin + nextTmin) * 0.5f;
            }
            else if (i==0)
            {
                nextTmin = meteoPoint->getMeteoPointValueD(getCrit3DDate(date.addDays(1)), dailyAirTemperatureMin);
                tmin = nextTmin;
            }
            else if (i==(daysInYear-1))
            {
                prevTmin = meteoPoint->getMeteoPointValueD(getCrit3DDate(date.addDays(-1)), dailyAirTemperatureMin);
                tmin = prevTmin;
            }

            meteoPoint->setMeteoPointValueD(getCrit3DDate(date), dailyAirTemperatureMin, tmin);

        }
        if (tmax == NODATA)
        {
            if (i!=0 && i!=(daysInYear-1))
            {
                prevTmax = meteoPoint->getMeteoPointValueD(getCrit3DDate(date.addDays(-1)), dailyAirTemperatureMax);
                nextTmax = meteoPoint->getMeteoPointValueD(getCrit3DDate(date.addDays(1)), dailyAirTemperatureMax);
                tmax = (prevTmin + nextTmin) * 0.5f;
            }
            else if (i==0)
            {
                nextTmax = meteoPoint->getMeteoPointValueD(getCrit3DDate(date.addDays(1)), dailyAirTemperatureMax);
                tmax = nextTmax;
            }
            else if (i==(daysInYear-1))
            {
                prevTmax = meteoPoint->getMeteoPointValueD(getCrit3DDate(date.addDays(-1)), dailyAirTemperatureMax);
                tmax = prevTmax;
            }

            meteoPoint->setMeteoPointValueD(getCrit3DDate(date), dailyAirTemperatureMax, tmax);
        }
        if (tavg == NODATA)
        {
            tavg = (tmin + tmax) * 0.5f;
            meteoPoint->setMeteoPointValueD(getCrit3DDate(date), dailyAirTemperatureAvg, tavg);
        }
        if (prec == NODATA)
        {
            prec = 0;
        }
    }
    return true;
}

/*!
 * \brief read daily meteo data from a table in the criteria-1D format
 * \brief (`date`,`tmin`,`tmax`,`tavg`,`prec`,`etp`,`watertable`)
 * \details mandatory: date, tmin, tmax, prec
 * \details not mandatory: tavg, etp, watertable
 * \details date format: "yyyy-mm-dd"
 * \return true if data are correctly loaded
 * \note meteoPoint have to be initialized BEFORE function
 */
bool readDailyDataCriteria1D(QSqlQuery *query, Crit3DMeteoPoint *meteoPoint, QString *myError)
{
    const int MAX_MISSING_DAYS = 3;
    QDate myDate, expectedDate, previousDate;
    Crit3DDate date;

    float tmin = NODATA;
    float tmax = NODATA;
    float tmed = NODATA;
    float prec = NODATA;            // [mm]
    float et0 = NODATA;             // [mm]
    float waterTable = NODATA;      // [m]
    float previousTmin = NODATA;
    float previousTmax = NODATA;
    float previousWaterTable = NODATA;
    int nrMissingData = 0;

    // first date
    query->first();
    myDate = query->value("date").toDate();
    expectedDate = myDate;
    previousDate = myDate.addDays(-1);

    do
    {
        myDate = query->value("date").toDate();

        if (! myDate.isValid())
        {
            *myError = "Wrong date format: " + query->value("date").toString();
            return false;
        }

        if (myDate != previousDate)
        {
            if (myDate != expectedDate)
            {
                if (expectedDate.daysTo(myDate) > MAX_MISSING_DAYS)
                {
                    *myError = "Wrong METEO: too many missing data." + expectedDate.toString();
                    return false;
                }
                else
                {
                    // fill missing data
                    while (myDate != expectedDate)
                    {
                        tmin = previousTmin;
                        tmax = previousTmax;
                        tmed = (tmin + tmax) * 0.5f;
                        prec = 0;
                        et0 = NODATA;
                        waterTable = previousWaterTable;

                        date = getCrit3DDate(expectedDate);
                        meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureMin, tmin);
                        meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureMax, tmax);
                        meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureAvg, tmed);
                        meteoPoint->setMeteoPointValueD(date, dailyPrecipitation, prec);
                        meteoPoint->setMeteoPointValueD(date, dailyReferenceEvapotranspirationHS, et0);
                        meteoPoint->setMeteoPointValueD(date, dailyWaterTableDepth, waterTable);

                        expectedDate = expectedDate.addDays(1);
                    }
                }
            }

            previousTmax = tmax;
            previousTmin = tmin;
            previousWaterTable = waterTable;

            // mandatory variables
            getValue(query->value("tmin"), &tmin);
            getValue(query->value("tmax"), &tmax);
            getValue(query->value("prec"), &prec);

            // check
            if (prec < 0.f) prec = NODATA;
            if (tmin < -50 || tmin > 40) tmin = NODATA;
            if (tmax < -40 || tmax > 50) tmax = NODATA;

            if (int(tmin) == int(NODATA) || int(tmax) == int(NODATA) || int(prec) == int(NODATA))
            {
                if (nrMissingData < MAX_MISSING_DAYS)
                {
                    if (int(tmin) == int(NODATA)) tmin = previousTmin;
                    if (int(tmax) == int(NODATA)) tmax = previousTmax;
                    if (int(prec) == int(NODATA)) prec = 0;
                    nrMissingData++;
                }
                else
                {
                    *myError = "Wrong METEO: too many missing data " + myDate.toString();
                    return false;
                }
            }
            else nrMissingData = 0;

            // NOT mandatory variables

            // TAVG [°C]
            getValue(query->value("tavg"), &tmed);
            if (int(tmed) == int(NODATA) || tmed < -40.f || tmed > 40.f)
                 tmed = (tmin + tmax) * 0.5f;

            // ET0 [mm]
            getValue(query->value("etp"), &et0);
            if (et0 < 0.f || et0 > 10.f)
                et0 = NODATA;

            // Watertable depth [m]
            getValue(query->value("watertable"), &waterTable);
            if (waterTable < 0.f) waterTable = NODATA;

            date = getCrit3DDate(myDate);
            if (meteoPoint->obsDataD[0].date.daysTo(date) < meteoPoint->nrObsDataDaysD)
            {
                meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureMin, float(tmin));
                meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureMax, float(tmax));
                meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureAvg, float(tmed));
                meteoPoint->setMeteoPointValueD(date, dailyPrecipitation, float(prec));
                meteoPoint->setMeteoPointValueD(date, dailyReferenceEvapotranspirationHS, float(et0));
                meteoPoint->setMeteoPointValueD(date, dailyWaterTableDepth, waterTable);
            }
            else
            {
                *myError = "Wrong METEO: index out of range.";
                return false;
            }

            previousDate = myDate;
            expectedDate = myDate.addDays(1);
        }

    } while(query->next());

    return true;
}

