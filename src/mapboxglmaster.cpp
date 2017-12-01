#include "mapboxglmaster.h"

#include "infohub.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QFileInfo>

#include <QDebug>

MapboxGLMaster::MapboxGLMaster(QObject *parent) : QObject(parent)
{
}

MapboxGLMaster::~MapboxGLMaster()
{

}

bool MapboxGLMaster::getTile(int x, int y, int z, QByteArray &result, bool &compressed, bool &found)
{
  std::unique_lock<std::mutex> lk(m_mutex);

  compressed = true; /// maybe would be flexible in future, for now just assume its compressed

  QString connection;
  if (z < const_section_level)
    connection = const_conn_world;
  else
    {
      int xx = x >> (z-const_section_level);
      int yy = y >> (z-const_section_level);
      connection = const_conn_prefix + QString("7-%1-%2").arg(xx).arg(yy);
    }

  if (!m_db_connections.contains(connection))
    {
      found = false;
      return true;
    }

  QSqlDatabase db = QSqlDatabase::database(connection, true);

  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  query.setForwardOnly(true);
  query.prepare("SELECT tile_data FROM tiles WHERE (zoom_level=:z AND tile_column=:x AND tile_row=:y)");
  query.bindValue(":x", x);
  query.bindValue(":y", (1 << z) - 1 - y); // conversion between XYZ and TMS
  query.bindValue(":z", z);

  if (!query.exec())
    {
      InfoHub::logWarning(tr("Failed to run query in Mapbox GL database"));
      return false;
    }

  while (query.next())
    {
      // will be called only once since there is only one tile matching it
      result = query.value(0).toByteArray();
      found = true;
      return true;
    }

  found = false;
  return true;
}

void MapboxGLMaster::onSettingsChanged()
{
  //std::unique_lock<std::mutex> lk(m_mutex);
  //AppSettings settings;
}

void MapboxGLMaster::onMapboxGLChanged(QString world_database, QSet<QString> country_databases)
{
  std::unique_lock<std::mutex> lk(m_mutex);

  if (m_world_fname == world_database && m_country_fnames == country_databases)
    return; // already using these settings

  m_world_fname = world_database;
  m_country_fnames = country_databases;

  // close all previous connections
  for (const QString &c: m_db_connections)
    QSqlDatabase::removeDatabase(c);
  m_db_connections.clear();

  ////////////////////////////////////
  /// open database connections

  // world
  if (!world_database.isEmpty())
    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", const_conn_world);
      db.setDatabaseName(world_database);
      m_db_connections.insert(const_conn_world);
    }

  // sections
  for (const QString &current: country_databases)
    {
      /// process the database name for sections in form
      /// /dir/dir.../tiles-section-7-71-38.sqlite
      QFileInfo fi(current);
      QString connection = const_conn_prefix + fi.baseName().mid(14);

      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connection);
      db.setDatabaseName(current);
      m_db_connections.insert(connection);
    }
}

