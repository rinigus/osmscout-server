#include "mapboxglmaster.h"

#include "infohub.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include <QDebug>

MapboxGLMaster::MapboxGLMaster(QObject *parent) : QObject(parent)
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", const_conn_world);
  db.setDatabaseName("tiles-world.sqlite");
  db.open();
}

MapboxGLMaster::~MapboxGLMaster()
{

}

bool MapboxGLMaster::getTile(int x, int y, int z, QByteArray &result, bool &compressed, bool &found)
{
  compressed = true; /// maybe would be flexible in future, for now just assume its compressed

  QSqlDatabase db_world = QSqlDatabase::database(const_conn_world, true);

  if (!db_world.isOpen()) return false;

  QSqlQuery query(db_world);
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

void MapboxGLMaster::onMapboxGLChanged(QString world_database, QStringList country_databases)
{
  //std::unique_lock<std::mutex> lk(m_mutex);
  qDebug() << world_database << " " << country_databases;
}

