/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
 * 
 * This file is part of OSM Scout Server.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef USE_MAPNIK

#ifndef MAPNIKMASTER_H
#define MAPNIKMASTER_H

#include <QObject>
#include <QStringList>
#include <QString>
#include <QHash>

#include <mapnik/map.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <deque>

/// \brief Access to Mapnik rendering
///
/// This is a thread safe object used to render maps through Mapnik
class MapnikMaster : public QObject
{
  Q_OBJECT

protected:
  explicit MapnikMaster(QObject *parent = 0);

public:
  static MapnikMaster* instance();
  virtual ~MapnikMaster();

  bool renderMap(const QString &style, bool daylight,
                 int width, int height,
                 double lat0, double lon0,
                 double lat1, double lon1,
                 QByteArray &result);

signals:

public slots:
    void onSettingsChanged();
    void onMapnikChanged(QString world_directory, QStringList country_dirs);

protected:

    /// \brief Checks whether new configuration is requested and loads it if needed
    ///
    /// This method allows to delay loading on start and dynamically change styles if needed. Its
    /// called every time a new tile is requested.
    /// This method should be called with the mutex locked by the caller
    void checkForSettingsChanges();

    /// \brief Regenerates XML configuration and allocates new map objects, if needed
    ///
    /// This method should be called with the mutex locked by the caller
    void reloadMapnik(const QString &world_directory, const QStringList &country_dirs, bool config_changed);

    /// \brief Sets configuration directory and local configuration
    ///
    /// Used internally to set variables determining configuration directory
    /// and its local copy location
    void setConfigDir(const QString &style, bool daylight);

protected:
    std::mutex m_mutex;
    bool m_available{false};

    //mapnik::Map m_map;
    mapnik::projection m_projection_longlat;
    mapnik::projection m_projection_merc;
    mapnik::proj_transform m_projection_transform;

    std::atomic<float> m_scale{1.0};
    std::atomic<int>   m_buffer_size{128}; ///< Size of a buffer around a tile in pixels for scale 1
    QString m_styles_dir;
    QString m_configuration_dir;
    QString m_local_dir_offset;
    QString m_local_xml;

    QString m_old_config_style;
    QString m_config_path_world;
    QStringList m_config_countries;
    size_t m_config_maps_counter{0};
    size_t m_old_config_maps_counter{0};

    QString m_preferred_language;

    QHash<QString, QString> m_styles_cache;

    // pool of mapnik maps
    std::deque< std::shared_ptr< mapnik::Map > > m_pool_maps;
    int m_pool_maps_generation{0}; ///< Increased when the new settings are loaded
    std::condition_variable m_pool_maps_cv;
    int m_ncpus{0};

    const QString const_xml{"mapnik.xml"};
    const QString const_dir{"mapnik"};

    static MapnikMaster *s_instance;
};

#endif // MAPNIKMASTER_H

#endif
