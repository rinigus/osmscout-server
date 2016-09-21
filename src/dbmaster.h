#ifndef DBMASTER_H
#define DBMASTER_H

#include <osmscout/Database.h>
#include <osmscout/MapService.h>

#include <QMutex>
#include <QByteArray>

#define OSM_SETTINGS "libosmscout/"

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

protected:
    QMutex m_mutex;
    bool m_error_flag=false;
    std::string m_icons_dir;

    osmscout::DatabaseParameter databaseParameter;
    osmscout::DatabaseRef database;
    osmscout::MapServiceRef mapService;
    osmscout::StyleConfigRef styleConfig;
};

#endif // DBMASTER_H
