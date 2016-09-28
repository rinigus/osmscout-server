#ifndef DBMASTER_H
#define DBMASTER_H

#include <osmscout/Database.h>
#include <osmscout/MapService.h>

#include <QMutex>
#include <QByteArray>
#include <QObject>

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

    bool search(QString search, QByteArray &result, size_t limit);

    /// \brief checks if DBMaster object is ready for operation
    ///
    operator bool() const { return !m_error_flag; }

    void loadSettings();

public slots:
    void onSettingsChanged();

protected:

    bool loadStyle(bool daylight);


protected:
    QMutex m_mutex;

    bool m_error_flag=false;

    std::string m_icons_dir;
    std::string m_style_name;
    bool m_render_sea = true;
    bool m_draw_background = true;
    float m_font_size = 3.0;
    bool m_daylight = true;

    osmscout::DatabaseParameter m_database_parameter;
    osmscout::DatabaseRef m_database;
    osmscout::MapServiceRef m_map_service;
    osmscout::StyleConfigRef m_style_config;
};

#endif // DBMASTER_H
