#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include <QObject>
#include <QMutex>
#include <QString>
#include <QStringList>

/// \brief Map Manager
///
/// Map Manager keeps the list of available maps, geocoder and
/// libpostal databases as well as tracks dependencies between
/// them
class MapManager : public QObject
{
  Q_OBJECT
public:
  explicit MapManager(QObject *parent = 0);
  virtual ~MapManager();

  void loadSettings();

signals:
    void databaseOsmScoutChanged(QStringList databaseDirectories);

public slots:
  void onSettingsChanged();

protected:
  void scanDirectories();

protected:
  QMutex m_mutex;

  QString m_root_dir;

  QStringList m_osmscout_dirs;
};

#endif // MAPMANAGER_H
