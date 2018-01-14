#include "mapmanager_urlcollection.h"

#include <QJsonArray>

#include <algorithm>
#include <random>

#include <QDebug>

using namespace MapManager;

UrlCollection::UrlCollection()
{
}


void UrlCollection::load(const QJsonObject &object)
{
  m_collection_full.clear();

  std::random_device rd;
  std::mt19937 g(rd());

  QJsonArray servers = object.value("servers").toArray();
  for (QJsonArray::const_iterator iter_servers = servers.constBegin();
       iter_servers != servers.constEnd(); ++iter_servers)
    {
      QStringList list;
      const QJsonArray prio = (*iter_servers).toArray();
      for (QJsonArray::const_iterator iter_prio = prio.constBegin();
           iter_prio != prio.constEnd(); ++iter_prio)
        list.append( (*iter_prio).toString() );

      std::shuffle(list.begin(), list.end(), g);
      m_collection_full.enqueue(list);
    }

  reload();
}


void UrlCollection::reload()
{
  m_collection_current = m_collection_full;
  next();
}


bool UrlCollection::next()
{
  while (!m_collection_current.empty() && m_collection_current.first().empty())
    m_collection_current.dequeue();

  if (m_collection_current.empty())
    {
      m_current.clear();
      return false;
    }

  m_current = m_collection_current.first().first();
  m_collection_current.first().pop_front();
  return true;
}
