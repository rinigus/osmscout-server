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
  explicit ValhallaMaster(QObject *parent = 0);
  virtual ~ValhallaMaster();

  bool route(QString uri, QByteArray &result);
  void start();

signals:

public slots:
  void onSettingsChanged();
  void onValhallaChanged(QString valhalla_directory, QStringList countries);

protected:
  void generateConfig();
  void stop();
  void start_process(); ///< Called internally when there is no process running and its free to start a new one

  void onProcessStarted();
  void onProcessStopped(int exitCode, QProcess::ExitStatus exitStatus); ///< Called on error while starting or when process has stopped
  void onProcessStateChanged(QProcess::ProcessState newState); ///< Called when state of the process has changed
  void onProcessRead();
  void onProcessReadError();

protected:
  std::mutex m_mutex;

  std::unique_ptr< valhalla::tyr::actor_t > m_actor;

  QString m_config_fname;
  QString m_dirname;
  QStringList m_countries;
  std::string m_config_json;

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
