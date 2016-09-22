#ifndef DBMASTER_H
#define DBMASTER_H

#include <osmscout/Database.h>
#include <osmscout/MapService.h>

#include <QMutex>
#include <QByteArray>

/// \brief Access to all OSM Scout functionality
///
/// This is a thread safe object used to render maps, search for locations, and calculate routing.
class DBMaster
{    
public:
    DBMaster();
    ~DBMaster();

    bool renderMap(double dpi, int zoom_level, int width, int height, double lat, double lon, QByteArray &result);

    operator bool() const { return !m_error_flag; }

    void loadSettings();

protected:
    QMutex m_mutex_settings;
    QMutex m_mutex_database;

    bool m_error_flag=false;

    std::string m_icons_dir;
    std::string m_style_name;
    bool m_render_sea = true;
    bool m_draw_background = true;
    float m_font_size = 3.0;

    osmscout::DatabaseParameter m_database_parameter;
    osmscout::DatabaseRef m_database;
    osmscout::MapServiceRef m_map_service;
    osmscout::StyleConfigRef m_style_config;
};

#endif // DBMASTER_H
