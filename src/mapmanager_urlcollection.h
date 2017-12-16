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
