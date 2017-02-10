#include "mapmanager.h"

#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QMutexLocker>
#include <QDirIterator>
#include <QDir>

MapManager::MapManager(QObject *parent) : QObject(parent)
{
  // first scan is done when loadSettings is called
}


MapManager::~MapManager()
{
}

void MapManager::loadSettings()
{
  onSettingsChanged();
}

void MapManager::onSettingsChanged()
{
  {
    QMutexLocker lk(&m_mutex);
    AppSettings settings;

    m_root_dir = settings.valueString(MAPMANAGER_SETTINGS "root");
  }

  scanDirectories();
}

void MapManager::scanDirectories()
{
  QMutexLocker lk(&m_mutex);

  // scan for libosmscout databases
  QStringList osmscout_dirs;

  QDirIterator dir_it(m_root_dir, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
  while (dir_it.hasNext())
    {
      QDir d(dir_it.next());
      if (d.exists() && d.exists("types.dat"))
        osmscout_dirs.append(d.absolutePath());
    }

  osmscout_dirs.sort();
  if (osmscout_dirs != m_osmscout_dirs)
    {
      m_osmscout_dirs = osmscout_dirs;
      emit databaseOsmScoutChanged(m_osmscout_dirs);
    }
}
