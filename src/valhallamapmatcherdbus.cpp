#include "valhallamapmatcherdbus.h"
#include "infohub.h"

#include <QDebug>

ValhallaMapMatcherDBus::ValhallaMapMatcherDBus(QObject *parent):
  QObject(parent)
{
}

ValhallaMapMatcherDBus::~ValhallaMapMatcherDBus()
{
}

void ValhallaMapMatcherDBus::activate()
{
  emit activeChanged(true);
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

void ValhallaMapMatcherDBus::onNameOwnerChanged(QString name, QString /*old_owner*/, QString new_owner)
{
  if (new_owner.length() < 1 &&
      m_matchers.contains(name))
    {
      InfoHub::logInfo(tr("Closing map matching service for DBus client %1").arg(name));
      m_matchers.remove(name);
    }
}
