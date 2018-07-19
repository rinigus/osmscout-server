#include "valhallamapmatcherdbus.h"

#include <QDebug>

ValhallaMapMatcherDBus::ValhallaMapMatcherDBus(QObject *parent):
  QDBusAbstractAdaptor(parent)
{

}

ValhallaMapMatcherDBus::~ValhallaMapMatcherDBus()
{
}

QString ValhallaMapMatcherDBus::update(double lat, double lon, double accuracy, const QDBusMessage &message)
{
  const QString caller = message.service();
  QPointer<ValhallaMapMatcher> c = m_matchers[caller];
  if (c.isNull()) return "{\"error\": \"unallocated\"}";
  return c->update(lat, lon, accuracy);
}

bool ValhallaMapMatcherDBus::start(int mode, const QDBusMessage &message)
{
  const QString caller = message.service();
  if (m_matchers.contains(caller))
    {
      delete m_matchers[caller];
      m_matchers.remove(caller);
    }
  m_matchers.insert(caller, QPointer<ValhallaMapMatcher>(new ValhallaMapMatcher()));
  QPointer<ValhallaMapMatcher> c = m_matchers[caller];
  if (c.isNull()) return false;
  return c->start( ValhallaMapMatcher::int2mode(mode) );
}



//bool ValhallaMapMatcherDBus::start(int mode)
//{
//  qDebug() << "Start called: " << mode;
//  return true;
//}

//bool ValhallaMapMatcherDBus::stop(int mode)
//{
//  qDebug() << "Stop called: " << mode;
//  return true;
//}

//bool ValhallaMapMatcherDBus::stop()
//{
//  qDebug() << "Stop called";
//  return true;
//}
