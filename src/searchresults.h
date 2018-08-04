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

#ifndef SEARCHRESULTS_H
#define SEARCHRESULTS_H

#include <osmscout/Database.h>

#include <QVector>
#include <QMap>
#include <QString>
#include <QSet>

///////////////////////////////////////////////////////////////////////////////////////
/// \brief The helper class to keep SearchResults
///
class SearchResults
{
public:
    bool contains(osmscout::FileOffset id) const
    {
        return m_elements.contains(id);
    }

    bool contains(const osmscout::ObjectFileRef &object) const
    {
        return contains(object.GetFileOffset());
    }

    void add(osmscout::FileOffset id, QMap<QString, QString> &curr_result)
    {
        m_results.push_back(curr_result);
        m_elements.insert(id);
    }

    void add(const osmscout::ObjectFileRef &object, QMap<QString, QString> &curr_result)
    {
        add( object.GetFileOffset(), curr_result);
    }

    QVector< QMap<QString, QString> >& results() { return m_results; }

    size_t length() const { return m_results.length(); }

protected:
    QSet<osmscout::FileOffset> m_elements;
    QVector< QMap<QString, QString> > m_results;
};

#endif // SEARCHRESULTS_H
