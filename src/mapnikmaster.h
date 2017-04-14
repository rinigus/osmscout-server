#ifndef MAPNIKMASTER_H
#define MAPNIKMASTER_H

#include <QObject>
#include <QStringList>
#include <QString>

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

public:
  explicit MapnikMaster(QObject *parent = 0);
  virtual ~MapnikMaster();

  bool renderMap(bool daylight, int width, int height,
                 double lat0, double lon0,
                 double lat1, double lon1,
                 QByteArray &result);

signals:

public slots:
    void onSettingsChanged();
    void onMapnikChanged(QString root_directory, QStringList country_files);

protected:
    std::mutex m_mutex;

    //mapnik::Map m_map;
    mapnik::projection m_projection_longlat;
    mapnik::projection m_projection_merc;
    mapnik::proj_transform m_projection_transform;

    std::atomic<float> m_scale{1.0};
    std::atomic<int>  m_buffer_size{128}; ///< Size of a buffer around a tile in pixels for scale 1
    QString m_configuration_dir;
    QString m_local_xml;

    // pool of mapnik maps
    std::deque< std::shared_ptr< mapnik::Map > > m_pool_maps;
    int m_pool_maps_generation{0}; ///< Increased when the new settings are loaded
    std::condition_variable m_pool_maps_cv;

    const QString const_xml{"mapnik.xml"};
    const QString const_dir{"mapnik"};
};

#endif // MAPNIKMASTER_H
