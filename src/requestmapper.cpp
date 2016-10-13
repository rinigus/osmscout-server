/**
  @file
  @author Stefan Frings
*/

#include "requestmapper.h"
#include "dbmaster.h"
#include "infohub.h"

#include <QUrl>
#include <QUrlQuery>

extern DBMaster *osmScoutMaster;

RequestMapper::RequestMapper(QObject* parent)
    :HttpRequestHandler(parent)
{
    qDebug("RequestMapper: created");
}


RequestMapper::~RequestMapper()
{
    qDebug("RequestMapper: deleted");
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
T q2value(const QString &key, T default_value, QUrlQuery &q, bool &ok)
{
    if (!q.hasQueryItem(key))
        return default_value;

    bool this_ok = true;
    T v = qstring2value<T>(q.queryItemValue(key),this_ok);
    if (!this_ok)
        v = default_value;
    ok = (ok && this_ok);
    return v;
}

//////////////////////////////////////////////////////////////////////
/// Default error function
//////////////////////////////////////////////////////////////////////
static void returnError(HttpResponse &response)
{
    response.setStatus(500, "unknown command");
    response.write("There was either error in server configuration or error in given URL", true);
}


void RequestMapper::service(HttpRequest& request, HttpResponse& response)
{
    QUrl url(request.getPath());
    QString path(url.path());
    QUrlQuery query(url.query());

    InfoHub::logInfo("Request: " + url.toString());

    if (path == "/v1/tile") // Tile
    {
        bool ok = true;
        bool daylight = q2value<bool>("daylight", true, query, ok);
        int shift = q2value<int>("shift", 0, query, ok);
        int scale = q2value<int>("scale", 1, query, ok);
        int x = q2value<int>("x", 0, query, ok);
        int y = q2value<int>("y", 0, query, ok);
        int z = q2value<int>("z", 0, query, ok);

        if (!ok)
        {
            returnError(response);
            InfoHub::logWarning("Error in HTTP query");
            return;
        }

        int ntiles = 1 << shift;

        QByteArray bytes;
        if ( !osmScoutMaster->renderMap(daylight, 96*scale/ntiles, z + shift, 256*scale, 256*scale,
                                        (tiley2lat(y, z) + tiley2lat(y+1, z))/2.0,
                                        (tilex2long(x, z) + tilex2long(x+1, z))/2.0, bytes ) )
        {
            returnError(response);
            return;
        }

        response.setHeader("Content-Type", "image/png");
        response.write(bytes, true);
    }

    else if (path == "/v1/search")
    {
        bool ok = true;
        size_t limit = q2value<size_t>("limit", 25, query, ok);
        QString search = q2value<QString>("search", "", query, ok);

        search = search.simplified();

        if (!ok || search.length() < 1)
        {
            returnError(response);
            InfoHub::logWarning("Error in HTTP query");
            return;
        }

        QByteArray bytes;
        if ( !osmScoutMaster->search(search, bytes, limit) )
        {
            returnError(response);
            return;
        }

        response.setHeader("Content-Type", "text/plain; charset=UTF-8");
        response.write(bytes, true);
    }

    else if (path == "/v1/guide")
    {
        bool ok = true;
        double radius = q2value<double>("radius", 1000.0, query, ok);
        size_t limit = q2value<size_t>("limit", 50, query, ok);
        QString poitype = q2value<QString>("poitype", "", query, ok);
        QString search = q2value<QString>("search", "", query, ok);
        double lon = q2value<double>("lng", 0, query, ok);
        double lat = q2value<double>("lat", 0, query, ok);

        if (!ok)
        {
            returnError(response);
            InfoHub::logWarning("Error in HTTP query");
            return;
        }

        search = search.simplified();

        QByteArray bytes;
        bool res = false;

        if ( query.hasQueryItem("lng") && query.hasQueryItem("lat") )
        {
            res = osmScoutMaster->guide(poitype, lat, lon, radius, limit, bytes);
        }

        else if ( query.hasQueryItem("search") && search.length() > 0 )
        {
            res = osmScoutMaster->guide(poitype, search, radius, limit, bytes);
        }

        if (!res)
        {
            returnError(response);
            return;
        }

        response.setHeader("Content-Type", "text/plain; charset=UTF-8");
        response.write(bytes, true);
    }

    else if (path == "/v1/poi_types")
    {
        QByteArray bytes;
        if (!osmScoutMaster->poiTypes(bytes))
        {
            returnError(response);
            return;
        }

        response.setHeader("Content-Type", "text/plain; charset=UTF-8");
        response.write(bytes, true);
    }

    else // command unidentified. return help string
    {
        returnError(response);
        InfoHub::logWarning("Uknown URL path");
        return;
    }
}
