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

#ifndef MAPBOXGLMASTER_H
#define MAPBOXGLMASTER_H

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QSet>

#include <memory>
#include <mutex>
#include <sqlite3pp.h>

class MapboxGLMaster : public QObject
{
  Q_OBJECT

protected:
  explicit MapboxGLMaster(QObject *parent = nullptr);

public:
  static MapboxGLMaster* instance();

  virtual ~MapboxGLMaster();

  bool getTile(int x, int y, int z, QByteArray &result, bool &compressed, bool &found);
  bool getGlyphs(QString sprite, QString range, QByteArray &result, bool &compressed, bool &found);
  bool getStyle(const QString &stylename, QByteArray &result);
  bool getSpriteJson(const QString &fname, QByteArray &result);
  bool getSpriteImage(const QString &fname, QByteArray &result);

signals:

public slots:
  void onSettingsChanged();
  void onMapboxGLChanged(QString world_database, QString glyphs_database, QSet<QString> country_databases);

protected:
  void addConnection(const QString &connection, const QString &fname);
  std::shared_ptr<sqlite3pp::database> getDatabase(const QString &connection);
  QString getFilePath(const QString &dname, const QString &fname);

protected:
  std::mutex m_mutex;
  QString m_hostname_port;

  QString m_world_fname;
  QString m_glyphs_fname;
  QSet<QString> m_country_fnames;

  QHash<QString, QString> m_db_connection_fname; ///< Map between connection names and filenames
  QHash<QString, std::shared_ptr<sqlite3pp::database> > m_db_connection; ///< Map connection name -> database

  const int const_section_level{7};
  const QString const_conn_world{"mapboxgl: world"};
  const QString const_conn_glyphs{"mapboxgl: glyphs"};
  const QString const_conn_prefix{"mapboxgl: "};

  const QString const_tag_hostname_port{"HOSTNAMEPORT"};

  static MapboxGLMaster *s_instance;
};

#endif // MAPBOXGLMASTER_H
