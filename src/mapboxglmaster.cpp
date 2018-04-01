#include "mapboxglmaster.h"

#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QFileInfo>
#include <QDir>

#include <QDebug>

MapboxGLMaster::MapboxGLMaster(QObject *parent) : QObject(parent)
{
  AppSettings settings;

  QString host = settings.valueString(HTTP_SERVER_SETTINGS "host");
  int port = settings.valueInt(HTTP_SERVER_SETTINGS "port");

  m_hostname_port = host + ":" + QString::number(port);
}

MapboxGLMaster::~MapboxGLMaster()
{
}

void MapboxGLMaster::onSettingsChanged()
{
}

void MapboxGLMaster::onMapboxGLChanged(QString world_database, QString glyphs_database, QSet<QString> country_databases)
{
  std::unique_lock<std::mutex> lk(m_mutex);

  if (m_world_fname == world_database && m_glyphs_fname == glyphs_database && m_country_fnames == country_databases)
    return; // already using these settings

  m_world_fname = world_database;
  m_glyphs_fname = glyphs_database;
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

  // glyphs
  if (!glyphs_database.isEmpty())
    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", const_conn_glyphs);
      db.setDatabaseName(glyphs_database);
      m_db_connections.insert(const_conn_glyphs);
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

bool MapboxGLMaster::getGlyphs(QString stackstr, QString range, QByteArray &result, bool &compressed, bool &found)
{
  std::unique_lock<std::mutex> lk(m_mutex);

  compressed = false; // maybe would be flexible in future, for now just assume its not compressed

  const QString connection = const_conn_glyphs;
  if (!m_db_connections.contains(connection))
    {
      found = false;
      return true;
    }

  QSqlDatabase db = QSqlDatabase::database(connection, true);

  if (!db.isOpen()) return false;

  QStringList stacks = stackstr.split(",");

  // Add Noto Sans Regular as a backup for:
  // * default MapboxGL fonts
  // * Noto Italic (many scripts are missing)
  if ( (stackstr.contains("Open Sans Regular") && stackstr.contains("Arial Unicode MS Regular")) ||
       stackstr.contains("Noto Sans Italic") )
    stacks.append("Noto Sans Regular");

  for (QString stack: stacks)
    {
      QSqlQuery query(db);
      query.setForwardOnly(true);
      query.prepare("SELECT pbf FROM fonts WHERE (stack=:stack AND range=:range)");
      query.bindValue(":stack", stack);
      query.bindValue(":range", range);

      if (!query.exec())
        {
          InfoHub::logWarning(tr("Failed to run query in Mapbox GL fonts database"));
          return false;
        }

      while (query.next())
        {
          // will be called only once since there is only one tile matching it
          result = query.value(0).toByteArray();
          found = true;
          return true;
        }
    }

  found = false;
  return true;
}


QString MapboxGLMaster::getFilePath(const QString &dname, const QString &fname)
{
  QDir dir(MAPBOXGL_STYLEDIR); dir.cd(dname);
  QFileInfo fi(dir.absoluteFilePath(fname));
  QString fpath = fi.canonicalFilePath();

  // expected to serve files from subfolder(s)
  if (!fpath.startsWith(dir.absolutePath()))
    {
      InfoHub::logWarning(tr("Malformed Mapbox GL file request: %1/%2 [%3] [%4]").arg(dname).arg(fname).arg(fpath).arg(MAPBOXGL_STYLEDIR));
      return QString();
    }

  if (!fi.exists())
    {
      InfoHub::logWarning(tr("Requested Mapbox GL file does not exist: %1 [%2]").arg(fname).arg(fpath));
      return QString();
    }

  return fpath;
}


bool MapboxGLMaster::getStyle(const QString &stylename, QByteArray &result)
{
  QString fpath = getFilePath("styles", stylename + ".json");
  if (fpath.isEmpty())
    return false;

  // load style file
  QFile fin(fpath);

  if (!fin.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      InfoHub::logWarning(tr("Error opening Mapbox GL style template %1").arg(fpath));
      return false;
    }

  QTextStream tin(&fin);
  QString style = tin.readAll();
  if (tin.status() != QTextStream::Ok)
    {
      InfoHub::logWarning(tr("Error reading Mapbox GL style template %1").arg(fpath));
      return false;
    }

  style.replace(const_tag_hostname_port, m_hostname_port);
  result = style.toUtf8();
  return true;
}

bool MapboxGLMaster::getSpriteJson(const QString &fname, QByteArray &result)
{
  QString fpath = getFilePath("sprites", fname);
  if (fpath.isEmpty())
    return false;

  QFile fin(fpath);

  if (!fin.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      InfoHub::logWarning(tr("Error opening Mapbox GL sprite JSON"));
      return false;
    }

  QTextStream tin(&fin);
  QString txt = tin.readAll();
  if (tin.status() != QTextStream::Ok)
    {
      InfoHub::logWarning(tr("Error reading Mapbox GL sprite JSON"));
      return false;
    }

  result = txt.toUtf8();
  return true;
}

bool MapboxGLMaster::getSpriteImage(const QString &fname, QByteArray &result)
{
  QString fpath = getFilePath("sprites", fname);
  if (fpath.isEmpty())
    return false;

  QFile fin(fpath);

  if (!fin.open(QIODevice::ReadOnly))
    {
      InfoHub::logWarning(tr("Error opening Mapbox GL sprite image"));
      return false;
    }

  result = fin.readAll();
  if (result.size() == 0)
    {
      InfoHub::logWarning(tr("Error reading Mapbox GL sprite image"));
      return false;
    }

  return true;
}
