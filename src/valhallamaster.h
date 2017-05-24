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

class ValhallaMaster : public QObject
{
  Q_OBJECT
public:
  explicit ValhallaMaster(QObject *parent = 0);
  virtual ~ValhallaMaster();

signals:

public slots:
  void onSettingsChanged();
  void onValhallaChanged(QString valhalla_directory, QStringList countries);

protected:
  void generateConfig();
  void start();
  void stop();
  void start_process(); ///< Called internally when there is no process running and its free to start a new one

  void onProcessStarted();
  void onProcessStopped(int exitCode, QProcess::ExitStatus exitStatus); ///< Called on error while starting or when process has stopped
  void onProcessStateChanged(QProcess::ProcessState newState); ///< Called when state of the process has changed
  void onProcessRead();
  void onProcessReadError();

protected:
  std::mutex m_mutex;

  QString m_config_fname;
  QString m_dirname;
  QStringList m_countries;

  int m_cache;

  QProcess *m_process{nullptr};
  std::atomic<bool> m_process_ready{false};
  bool m_process_start_when_ready{false};
  bool m_process_killed{false};

  const QString const_conf{"valhalla.json"};
  const QString const_dir{"valhalla"};

  const QString const_tag_cache{"MAXIMAL_CACHE_SIZE"};
  const QString const_tag_dirname{"VALHALLA_TILE_DIRECTORY"};
};

#endif // USE_VALHALLA

#endif // VALHALLAMASTER_H
