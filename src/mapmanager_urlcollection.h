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

#ifndef MAPMANAGER_URLCOLLECTION_H
#define MAPMANAGER_URLCOLLECTION_H

#include <QJsonObject>
#include <QStringList>
#include <QQueue>

namespace MapManager {

  /// \brief Collection of URLs used to store addresses of the servers
  ///
  /// URLs aree loaded from JSON object and are kept in the stack. All URLs
  /// are partitioned according to their priority and can be iterated through
  /// by asking for the next url. By asking one-by-one url, the collection will get
  /// depleted in the end.
  class UrlCollection
  {
  public:
    UrlCollection();

    void load(const QJsonObject &object);

    const QString& url() const ///< fetch the current URL
    { return m_current; }

    bool isEmpty() const { return m_current.isEmpty(); }

    /// \brief iterate to the next url
    /// \return true if the next url exists
    bool next();

    void reload(); ///< restores url collection to the state where it can be iterated through again

  protected:
    QString m_current;
    QQueue<QStringList> m_collection_current;
    QQueue<QStringList> m_collection_full;
  };

}

#endif // MAPMANAGER_URLCOLLECTION_H
