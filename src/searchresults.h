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
