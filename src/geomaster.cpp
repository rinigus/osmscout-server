#include "geomaster.h"
#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QMutexLocker>

#include <QJsonArray>
#include <QJsonDocument>

GeoMaster::GeoMaster(QObject *parent) : QObject(parent)
{
    onSettingsChanged();
}


void GeoMaster::onSettingsChanged()
{
    QMutexLocker lk(&m_mutex);
    AppSettings settings;

    // prepare for new settings
    m_geocoder.drop();
    m_postal.clear_languages();

    // apply new settings
    m_postal.set_initialize_every_call(settings.valueInt(GEOMASTER_SETTINGS "initialize_every_call") > 0);
    m_postal.set_postal_datadir(settings.valueString(GEOMASTER_SETTINGS "postal-main-dir").toStdString(),
                                settings.valueString(GEOMASTER_SETTINGS "postal-country-dir").toStdString());
    m_postal.set_use_primitive(settings.valueInt(GEOMASTER_SETTINGS "use_primitive") > 0);

    QString geopath = settings.valueString(GEOMASTER_SETTINGS "geocoder-path");
    if (!m_geocoder.load(geopath.toStdString()))
    {
        InfoHub::logError(tr("Cannot open geocoding database") + ": " + geopath);
        return;
    }

    InfoHub::logInfo(tr("Opened geocoding database") + " " + geopath, true);

    m_geocoder.set_max_queries_per_hierarchy(settings.valueInt(GEOMASTER_SETTINGS "max_queries_per_hierarchy"));
}


bool GeoMaster::search(const QString &searchPattern, QJsonObject &result, size_t limit,
                       double &lat, double &lon, size_t &number_of_results)
{
    if (!m_geocoder && !m_geocoder.load())
    {
        InfoHub::logError(tr("Cannot open geocoding database"));
        return false;
    }

    // parsing with libpostal
    std::vector< GeoNLP::Postal::ParseResult > parsed_query;
    GeoNLP::Postal::ParseResult nonorm;

    if ( !m_postal.parse( searchPattern.toStdString(),
                          parsed_query, nonorm) )
    {
        InfoHub::logError(tr("Error parsing by libpostal, maybe libpostal databases are not available"));
        return false;
    }

    // search
    m_geocoder.set_max_results(limit);
    std::vector<GeoNLP::Geocoder::GeoResult> search_result;
    if ( !m_geocoder.search(parsed_query, search_result) )
    {
        InfoHub::logError(tr("Error while searching with geocoder-nlp"));
        return false;
    }

    // record results
    result.insert("query", searchPattern);

    {
        QJsonObject r;
        for (auto a: nonorm)
            r.insert(QString::fromStdString(a.first), QString::fromStdString(a.second));
        result.insert("parsed", r);
    }

    {
        QJsonArray arr;
        for (const GeoNLP::Postal::ParseResult &pr: parsed_query)
        {
            QJsonObject r;
            for (auto a: pr)
                r.insert(QString::fromStdString(a.first), QString::fromStdString(a.second));
            arr.push_back(r);
        }
        result.insert("parsed_normalized", arr);
    }

    {
        QJsonArray arr;
        for (const GeoNLP::Geocoder::GeoResult &sr: search_result)
          {
            QJsonObject r;
            r.insert("admin_region", QString::fromStdString(sr.address));
            r.insert("title", QString::fromStdString(sr.title));
            r.insert("lat", sr.latitude);
            r.insert("lng", sr.longitude);
            r.insert("object_id", sr.id);
            r.insert("type", QString::fromStdString(sr.type));
            r.insert("levels_resolved", (int)sr.levels_resolved);

            arr.push_back(r);
          }

        result.insert("result", arr);
    }

    number_of_results = search_result.size();
    if (number_of_results > 0)
    {
        lat = search_result[0].latitude;
        lon = search_result[0].longitude;
    }

    return true;
}

bool GeoMaster::search(const QString &searchPattern, double &lat, double &lon)
{
    QJsonObject obj;
    size_t number_of_results;

    if ( !search(searchPattern, obj, 1, lat, lon, number_of_results ) )
    {
        InfoHub::logWarning("Search for reference point failed");
        return false;
    }

    if ( number_of_results > 0 )
        return true;

    InfoHub::logWarning(tr("Search for reference point failed: cannot find") + " " + searchPattern);
    return false;
}

bool GeoMaster::searchExposed(const QString &searchPattern, QByteArray &result, size_t limit, bool full_result)
{
    QJsonObject sres;
    double lat, lon;
    size_t number_of_results;

    if ( !search(searchPattern, sres, limit, lat, lon, number_of_results ) )
        return false;

    if (!full_result)
    {
        QJsonDocument document(sres.value("result").toArray());
        result = document.toJson();
    }
    else
    {
        QJsonDocument document(sres);
        result = document.toJson();
    }

    return true;
}
