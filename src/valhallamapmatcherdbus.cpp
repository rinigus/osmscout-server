#include "valhallamapmatcherdbus.h"

#include <QDebug>

ValhallaMapMatcherDBus::ValhallaMapMatcherDBus(QObject *parent):
  QDBusAbstractAdaptor(parent)
{

}

ValhallaMapMatcherDBus::~ValhallaMapMatcherDBus()
{
}

QString ValhallaMapMatcherDBus::update(int mode, double lat, double lon, double accuracy, const QDBusMessage &message)
{
  const QString caller = message.service();
  QSharedPointer<ValhallaMapMatcher> &c = m_matchers[caller][mode];
  if (c.isNull())
    {
      c.reset(new ValhallaMapMatcher(mode));
      if ( c.isNull() ||
          !c->start() )
        return "{\"error\": \"Failed to allocate or start Map Matcher\"}";
    }
  return c->update(lat, lon, accuracy);
}

bool ValhallaMapMatcherDBus::reset(int mode, const QDBusMessage &message)
{
  const QString caller = message.service();
  QSharedPointer<ValhallaMapMatcher> c = m_matchers[caller][mode];
  return !c.isNull() && c->start();
}

bool ValhallaMapMatcherDBus::stop(int mode, const QDBusMessage &message)
{
  const QString caller = message.service();
  if (m_matchers[caller].contains(mode))
    m_matchers[caller].remove(mode);
  return true;
}

bool ValhallaMapMatcherDBus::stop(const QDBusMessage &message)
{
  const QString caller = message.service();
  if (m_matchers.contains(caller))
    m_matchers.remove(caller);
  return true;
}
