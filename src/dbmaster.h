#ifndef DBMASTER_H
#define DBMASTER_H

#include <osmscout/Database.h>
#include <osmscout/MapService.h>

#include "searchresults.h"

#include <QMutex>
#include <QByteArray>
#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>

#include <string>

/// \brief Access to all OSM Scout functionality
///
/// This is a thread safe object used to render maps, search for locations, and calculate routing.
class DBMaster: public QObject
{
    Q_OBJECT

public:
    DBMaster();
    virtual ~DBMaster();

    bool renderMap(bool daylight, double dpi, int zoom_level, int width, int height, double lat, double lon, QByteArray &result);

    bool search(const QString &searchPattern, QByteArray &result, size_t limit);

    /////////////////////////////////////////////////////////////
    /// \brief Search for a pattern and return the coordinates of the first found object
    ///
    /// Can be used for finding coordinates of the reference points
    ///
    /// \param searchPattern
    /// \param lat this is a return value, latitude of the reference point
    /// \param lon this is a return value, longitude of the reference point
    /// \return true if a reference point was found
    ///
    bool search(const QString &searchPattern, double &lat, double &lon);

    bool guide(const QString &poitype, double lat, double lon, double radius, size_t limit, QByteArray &result);

    bool poiTypes(QByteArray &result); ///< Fill results with list of supported POI types

    bool route(osmscout::Vehicle &vehicle, std::vector< osmscout::GeoCoord > &coordinates, double radius, QByteArray &result);

    /// \brief checks if DBMaster object is ready for operation
    ///
    operator bool() const { return !m_error_flag; }

    void loadSettings();

public slots:
    void onSettingsChanged();

protected:

    bool loadStyle(bool daylight);

    bool search(const QString &search, SearchResults &result, size_t limit);

protected:
    QMutex m_mutex;

    bool m_error_flag=false;

    std::string m_map_dir;
    std::string m_icons_dir;
    std::string m_style_name;
    std::string m_style_name_loaded;

    bool m_render_sea = true;
    bool m_draw_background = true;
    float m_font_size = 3.0;
    float m_data_lookup_area = 1.5;
    int m_tile_borders_zoom_cutoff = 20;
    bool m_daylight = true;

    osmscout::DatabaseParameter m_database_parameter;
    osmscout::DatabaseRef m_database;
    osmscout::MapServiceRef m_map_service;
    osmscout::StyleConfigRef m_style_config;
};

#endif // DBMASTER_H
