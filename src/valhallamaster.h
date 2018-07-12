#ifndef VALHALLAMASTER_H
#define VALHALLAMASTER_H

#ifdef USE_VALHALLA

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>

#include <cstdint>
#include <mutex>
#include <atomic>

#include <valhalla/tyr/actor.h>

class ValhallaMaster : public QObject
{
  Q_OBJECT

public:
  enum ActorType { Height, Isochrone, Locate, Matrix, OptimizedRoute, Route, TraceAttributes, TraceRoute };

public:
  explicit ValhallaMaster(QObject *parent = 0);
  virtual ~ValhallaMaster();

  /// \brief Call Valhalla's action
  ///
  /// Used to call route, trace_attributes and other actor actions
  bool callActor(ActorType atype, const QString &json, QByteArray &result);

  void start();

signals:

public slots:
  void onSettingsChanged();
  void onValhallaChanged(QString valhalla_directory, QStringList countries);

protected:
  void generateConfig();
  void stop();

protected:
  std::mutex m_mutex;

  std::unique_ptr< valhalla::tyr::actor_t > m_actor;

  QString m_config_fname;
  QString m_dirname;
  QStringList m_countries;
  std::string m_config_json;

  bool m_idle_mode{true}; ///< Used to delay start while loading settings

  int m_cache;

  float m_limit_max_distance_auto;
  float m_limit_max_distance_bicycle;
  float m_limit_max_distance_pedestrian;

  const QString const_tag_cache{"MAXIMAL_CACHE_SIZE"};
  const QString const_tag_dirname{"VALHALLA_TILE_DIRECTORY"};
  const QString const_tag_route_port{"TCPIP_ROUTE_PORT"};
  const QString const_tag_limit_max_distance_auto{"LIMIT_MAX_DISTANCE_AUTO"};
  const QString const_tag_limit_max_distance_bicycle{"LIMIT_MAX_DISTANCE_BICYCLE"};
  const QString const_tag_limit_max_distance_pedestrian{"LIMIT_MAX_DISTANCE_PEDESTRIAN"};
};

#endif // USE_VALHALLA

#endif // VALHALLAMASTER_H
