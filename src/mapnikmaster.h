#ifndef MAPNIKMASTER_H
#define MAPNIKMASTER_H

#include <QObject>
#include <QMutex>
#include <QStringList>

#include <mapnik/map.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>

#include <atomic>

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
    void onMapnikChanged(QStringList files);

protected:
    QMutex m_mutex;

    mapnik::Map m_map;
    mapnik::projection m_projection_longlat;
    mapnik::projection m_projection_merc;
    mapnik::proj_transform m_projection_transform;

    std::atomic<float> m_scale{1.0};
    QStringList m_mapnik_files;
};

#endif // MAPNIKMASTER_H
