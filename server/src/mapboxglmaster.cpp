/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
 *
 * This file is part of OSM Scout Server.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "mapboxglmaster.h"

#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QVariant>
#include <QFileInfo>
#include <QDir>

#include <QDebug>

MapboxGLMaster* MapboxGLMaster::s_instance = nullptr;

MapboxGLMaster::MapboxGLMaster(QObject *parent) : QObject(parent)
{
  AppSettings settings;

  QString host = settings.valueString(HTTP_SERVER_SETTINGS "host");
  int port = settings.valueInt(HTTP_SERVER_SETTINGS "port");

  m_hostname_port = host + ":" + QString::number(port);
}

MapboxGLMaster* MapboxGLMaster::instance()
{
  if (!s_instance) s_instance = new MapboxGLMaster();
  return s_instance;
}

MapboxGLMaster::~MapboxGLMaster()
{
}

void MapboxGLMaster::onSettingsChanged()
{
}

void MapboxGLMaster::addConnection(const QString &connection, const QString &fname)
{
  m_db_connection[connection] = nullptr;
  m_db_connection_fname[connection] = fname;
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
  m_db_connection.clear();
  m_db_connection_fname.clear();

  ////////////////////////////////////
  /// register database connections
  /// and corresponding file names

  // world
  if (!world_database.isEmpty())
    addConnection(const_conn_world, world_database);

  // glyphs
  if (!glyphs_database.isEmpty())
    addConnection(const_conn_glyphs, glyphs_database);

  // sections
  for (const QString &current: country_databases)
    {
      /// process the database name for sections in form
      /// /dir/dir.../tiles-section-7-71-38.sqlite
      QFileInfo fi(current);
      QString connection = const_conn_prefix + fi.baseName().mid(14);
      addConnection(connection, current);
    }
}

std::shared_ptr<sqlite3pp::database> MapboxGLMaster::getDatabase(const QString &connection)
{
  auto dbc = m_db_connection.find(connection);
  if (dbc == m_db_connection.end()) return nullptr;

  if (dbc.value()) return dbc.value();

  // allocate new database connection
  auto nmc = m_db_connection_fname.find(connection);
  if (nmc == m_db_connection_fname.end()) return nullptr;

  try {
    std::shared_ptr<sqlite3pp::database> db(new sqlite3pp::database(nmc.value().toStdString().c_str(),
                                                                    SQLITE_OPEN_READONLY));
    m_db_connection[connection] = db;
    return db;
  }
  catch (sqlite3pp::database_error &e) {
    InfoHub::logWarning(tr("Failed to open Mapbox GL database: %1").arg(e.what()));
  }

  return nullptr;
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

  std::shared_ptr<sqlite3pp::database> db = getDatabase(connection);
  if (!db)
    {
      found = false;
      return true;
    }

  try
  {
    sqlite3pp::query query(*db,
                           "SELECT tile_data FROM tiles WHERE "
                           "(zoom_level=:z AND tile_column=:x AND tile_row=:y)");
    query.bind(":x", x);
    query.bind(":y", (1 << z) - 1 - y); // conversion between XYZ and TMS
    query.bind(":z", z);

    for (auto v: query)
      {
        // will be called only once since there is only one tile matching it
        void const *blob = v.get<void const*>(0);
        int sz = v.column_bytes(0);
        result = QByteArray((const char*)blob, sz);
        return true;
      }
  }
  catch (sqlite3pp::database_error &e)
  {
    InfoHub::logWarning(tr("Failed to run query in Mapbox GL database: %1").arg(e.what()));
    return false;
  }

  found = false;
  return true;
}

bool MapboxGLMaster::getGlyphs(QString stackstr, QString range, QByteArray &result, bool &compressed, bool &found)
{
  std::unique_lock<std::mutex> lk(m_mutex);

  compressed = false; // maybe would be flexible in future, for now just assume its not compressed

  std::shared_ptr<sqlite3pp::database> db = getDatabase(const_conn_glyphs);
  if (!db)
    {
      found = false;
      return true;
    }

  QStringList stacks = stackstr.split(",");

  // Add Noto Sans Regular as a backup for:
  // * default MapboxGL fonts
  // * Noto Italic (many scripts are missing)
  if ( (stackstr.contains("Open Sans Regular") && stackstr.contains("Arial Unicode MS Regular")) ||
       stackstr.contains("Noto Sans Italic") )
    stacks.append("Noto Sans Regular");

  std::string str_range = range.toStdString();
  for (QString stack: stacks)
    {
      try
      {
        std::string str_stack = stack.toStdString();
        sqlite3pp::query query(*db, "SELECT pbf FROM fonts WHERE (stack=:stack AND range=:range)");
        query.bind(":stack", str_stack, sqlite3pp::nocopy);
        query.bind(":range", str_range, sqlite3pp::nocopy);

        for (auto v: query)
          {
            // will be called only once since there is only one tile matching it
            void const *blob = v.get<void const*>(0);
            int sz = v.column_bytes(0);
            result = QByteArray((const char*)blob, sz);
            return true;
          }
      }
      catch (sqlite3pp::database_error &e)
      {
        InfoHub::logWarning(tr("Failed to run query in Mapbox GL fonts database: %1").arg(e.what()));
        return false;
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
