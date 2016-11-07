/**
  @file
  @author Stefan Frings
*/

#include "requestmapper.h"
#include "dbmaster.h"
#include "infohub.h"

#include "microhttpconnectionstore.h"

#include <microhttpd.h>

#include <QTextStream>
#include <QUrl>
#include <QRunnable>
#include <QThreadPool>
#include <QDir>

#include <QDebug>

#include <functional>


extern DBMaster *osmScoutMaster;

RequestMapper::RequestMapper()
{
#ifdef IS_SAILFISH_OS
    // In Sailfish, CPUs could be switched off one by one. As a result,
    // "ideal thread count" set by Qt could be off.
    // In other systems, this procedure is not needed and the defaults can be used
    //
    int cpus = 0;
    QDir dir;
    while ( dir.exists(QString("/sys/devices/system/cpu/cpu") + QString::number(cpus)) )
        ++cpus;

    m_pool.setMaxThreadCount(cpus);
#endif

    qDebug() << "Number of threads used to render tiles: " << m_pool.maxThreadCount();
}


RequestMapper::~RequestMapper()
{
}


//////////////////////////////////////////////////////////////////////
/// Helper functions to get tile coordinates
//////////////////////////////////////////////////////////////////////

//static int long2tilex(double lon, int z)
//{
//    return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z)));
//}

//static int lat2tiley(double lat, int z)
//{
//    return (int)(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z)));
//}

static double tilex2long(int x, int z)
{
    return x / pow(2.0, z) * 360.0 - 180;
}

static double tiley2lat(int y, int z)
{
    double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
    return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

//////////////////////////////////////////////////////////////////////
/// Helper functions to get extract values from query
//////////////////////////////////////////////////////////////////////

template <typename T>
T qstring2value(const QString &, bool &)
{
    // dummy function, implement the specific version for each type separately
    T v;
    return v;
}

template <> int qstring2value(const QString &s, bool &ok)
{
    return s.toInt(&ok);
}

template <> size_t qstring2value(const QString &s, bool &ok)
{
    return s.toUInt(&ok);
}

template <> bool qstring2value(const QString &s, bool &ok)
{
    return (s.toInt(&ok) > 0);
}

template <> double qstring2value(const QString &s, bool &ok)
{
    return s.toDouble(&ok);
}

template <> QString qstring2value(const QString &s, bool &ok)
{
    ok = true;
    return s;
}

template <typename T>
T q2value(const char *key, T default_value, MHD_Connection *q, bool &ok)
{
    const char *vstr = MHD_lookup_connection_value(q, MHD_GET_ARGUMENT_KIND, key);
    if (vstr == NULL)
        return default_value;

    bool this_ok = true;
    T v = qstring2value<T>(vstr,this_ok);
    if (!this_ok)
        v = default_value;
    ok = (ok && this_ok);
    return v;
}

//////////////////////////////////////////////////////////////////////
/// Default error function
//////////////////////////////////////////////////////////////////////
static void errorText(MHD_Response *response, MicroHTTP::Connection::keytype connection_id, const char *txt)
{
    QByteArray data;
    {
        QTextStream output(&data, QIODevice::WriteOnly);
        output << txt;
    }

    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html; charset=UTF-8");
    MicroHTTP::ConnectionStore::setData(connection_id, data, false);
}

static void makeEmptyJson(QByteArray &result)
{
    QTextStream output(&result, QIODevice::WriteOnly);
    output << "{ }";
}

void RequestMapper::loguri(const char *uri)
{
    InfoHub::logInfo("Request: " + QString(uri));
}

/////////////////////////////////////////////////////////////////////////////
/// Runnable classes used to solve the tasks
/////////////////////////////////////////////////////////////////////////////

class Task: public QRunnable
{
public:
    Task(MicroHTTP::Connection::keytype key,
         std::function<bool(QByteArray &)> caller,
         QString error_message):
        QRunnable(),
        m_key(key),
        m_caller(caller),
        m_error_message(error_message)
    {}

    virtual ~Task() {}

    virtual void run()
    {
        QByteArray data;
        if ( !m_caller(data) )
        {
            QByteArray err;
            {
                QTextStream output(&err, QIODevice::WriteOnly);
                output << m_error_message;
            }

            MicroHTTP::ConnectionStore::setData(m_key, err, false);
        }

        MicroHTTP::ConnectionStore::setData(m_key, data, false);
    }

protected:
    MicroHTTP::Connection::keytype m_key;
    std::function<bool(QByteArray &)> m_caller;
    QString m_error_message;
};

//class TaskTile:
//{
//public:
//    TaskTile(MicroHTTP::Connection::keytype key,)

//};

/////////////////////////////////////////////////////////////////////////////
/// Request mapper main service function
/////////////////////////////////////////////////////////////////////////////
unsigned int RequestMapper::service(const char *url_c,
                                    MHD_Connection *connection, MHD_Response *response,
                                    MicroHTTP::Connection::keytype connection_id)
{
    QUrl url(url_c);
    QString path(url.path());

    //////////////////////////////////////////////////////////////////////
    /// TILES
    if (path == "/v1/tile")
    {
        bool ok = true;
        bool daylight = q2value<bool>("daylight", true, connection, ok);
        int shift = q2value<int>("shift", 0, connection, ok);
        int scale = q2value<int>("scale", 1, connection, ok);
        int x = q2value<int>("x", 0, connection, ok);
        int y = q2value<int>("y", 0, connection, ok);
        int z = q2value<int>("z", 0, connection, ok);

        if (!ok)
        {
            InfoHub::logWarning("Error in HTTP query");
            errorText(response, connection_id, "Error while reading tile query parameters");
            return MHD_HTTP_BAD_REQUEST;
        }

        int ntiles = 1 << shift;

        Task *task = new Task(connection_id,
                  std::bind(&DBMaster::renderMap, osmScoutMaster,
                            daylight, 96*scale/ntiles, z + shift, 256*scale, 256*scale,
                            (tiley2lat(y, z) + tiley2lat(y+1, z))/2.0,
                            (tilex2long(x, z) + tilex2long(x+1, z))/2.0, std::placeholders::_1),
                  "Error while rendering a tile" );

        m_pool.start(task);

        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "image/png");
        return MHD_HTTP_OK;
    }

    //    //////////////////////////////////////////////////////////////////////
    //    /// SEARCH
    //    else if (path == "/v1/search")
    //    {
    //        bool ok = true;
    //        size_t limit = q2value<size_t>("limit", 25, connection, ok);
    //        QString search = q2value<QString>("search", "", connection, ok);

    //        search = search.simplified();

    //        if (!ok || search.length() < 1)
    //        {
    //            returnError(response);
    //            InfoHub::logWarning("Error in HTTP query");
    //            return;
    //        }

    //        QByteArray bytes;
    //        if ( !osmScoutMaster->search(search, bytes, limit) )
    //        {
    //            returnError(response);
    //            return;
    //        }

    //        response.setHeader("Content-Type", "text/plain; charset=UTF-8");
    //        response.write(bytes, true);
    //    }

    //    //////////////////////////////////////////////////////////////////////
    //    /// GUIDE: LOOKUP POIs NEAR REFERENCE POINT
    //    else if (path == "/v1/guide")
    //    {
    //        bool ok = true;
    //        double radius = q2value<double>("radius", 1000.0, connection, ok);
    //        size_t limit = q2value<size_t>("limit", 50, connection, ok);
    //        QString poitype = q2value<QString>("poitype", "", connection, ok);
    //        QString search = q2value<QString>("search", "", connection, ok);
    //        double lon = q2value<double>("lng", 0, connection, ok);
    //        double lat = q2value<double>("lat", 0, connection, ok);

    //        if (!ok)
    //        {
    //            returnError(response);
    //            InfoHub::logWarning("Error in HTTP query");
    //            return;
    //        }

    //        search = search.simplified();

    //        QByteArray bytes;
    //        bool res = false;

    //        if ( query.hasQueryItem("lng") && query.hasQueryItem("lat") )
    //        {
    //            res = osmScoutMaster->guide(poitype, lat, lon, radius, limit, bytes);
    //        }

    //        else if ( query.hasQueryItem("search") && search.length() > 0 )
    //        {
    //            if (osmScoutMaster->search(search, lat, lon))
    //                res = osmScoutMaster->guide(poitype, lat, lon, radius, limit, bytes);
    //            else
    //            {
    //                res = true;
    //                makeEmptyJson(bytes);
    //            }
    //        }

    //        if (!res)
    //        {
    //            returnError(response);
    //            return;
    //        }

    //        response.setHeader("Content-Type", "text/plain; charset=UTF-8");
    //        response.write(bytes, true);
    //    }

    //    //////////////////////////////////////////////////////////////////////
    //    /// LIST AVAILABLE POI TYPES
    //    else if (path == "/v1/poi_types")
    //    {
    //        QByteArray bytes;
    //        if (!osmScoutMaster->poiTypes(bytes))
    //        {
    //            returnError(response);
    //            return;
    //        }

    //        response.setHeader("Content-Type", "text/plain; charset=UTF-8");
    //        response.write(bytes, true);
    //    }

    //    //////////////////////////////////////////////////////////////////////
    //    /// ROUTING
    //    else if (path == "/v1/route")
    //    {
    //        bool ok = true;
    //        QString type = q2value<QString>("type", "car", connection, ok);
    //        double radius = q2value<double>("radius", 1000.0, connection, ok);
    //        bool gpx = q2value<int>("gpx", 0, connection, ok);

    //        std::vector<osmscout::GeoCoord> points;

    //        bool points_done = false;
    //        for (int i=0; !points_done && ok; ++i)
    //        {
    //            QString prefix = "p[" + QString::number(i) + "]";
    //            if ( query.hasQueryItem(prefix + "[lng]") && query.hasQueryItem(prefix + "[lat]") )
    //            {
    //                double lon = q2value<double>(prefix + "[lng]", 0, connection, ok);
    //                double lat = q2value<double>(prefix + "[lat]", 0, connection, ok);
    //                osmscout::GeoCoord c(lat,lon);
    //                points.push_back(c);
    //            }

    //            else if ( query.hasQueryItem(prefix + "[search]") )
    //            {
    //                QString search = q2value<QString>(prefix + "[search]", "", connection, ok);
    //                search = search.simplified();
    //                if (search.length()<1)
    //                {
    //                    returnError(response);
    //                    return;
    //                }

    //                double lat, lon;
    //                if (osmScoutMaster->search(search, lat, lon))
    //                {
    //                    osmscout::GeoCoord c(lat,lon);
    //                    points.push_back(c);
    //                }
    //                else
    //                    ok = false;
    //            }

    //            else points_done = true;
    //        }

    //        if (!ok || points.size() < 2)
    //        {
    //            returnError(response);
    //            InfoHub::logWarning("Error in HTTP query");
    //            return;
    //        }

    //        osmscout::Vehicle vehicle;
    //        if (type == "car") vehicle = osmscout::vehicleCar;
    //        else if (type == "bicycle") vehicle = osmscout::vehicleBicycle;
    //        else if (type == "foot") vehicle = osmscout::vehicleFoot;
    //        else
    //        {
    //            returnError(response);
    //            InfoHub::logWarning("Error in HTTP query: unknown vehicle");
    //            return;
    //        }

    //        for (auto i: points)
    //            std::cout << i.GetLat() << " " << i.GetLon() << "\n";

    //        QByteArray bytes;
    //        bool res = osmScoutMaster->route(vehicle, points, radius, gpx, bytes);

    //        if (!res)
    //        {
    //            returnError(response);
    //            return;
    //        }

    //        if (!gpx) response.setHeader("Content-Type", "text/plain; charset=UTF-8");
    //        else response.setHeader("Content-Type", "text/xml; charset=UTF-8");
    //        response.write(bytes, true);
    //    }

    else // command unidentified. return help string
    {
        InfoHub::logWarning("Unknown URL path");
        errorText(response, connection_id, "Unknow URL path");
        return MHD_HTTP_BAD_REQUEST;
    }
}
