#include "dbustracker.h"

#include <QDBusConnection>

DBusTracker* DBusTracker::s_instance = nullptr;

DBusTracker::DBusTracker(QObject *parent) : QObject(parent)
{
  QDBusConnection dbusconnection = QDBusConnection::sessionBus();
  dbusconnection.connect(QString(),
                         "/org/freedesktop/DBus", "org.freedesktop.DBus", "NameOwnerChanged",
                         this, SLOT(onNameOwnerChanged(QString,QString,QString)));
}

DBusTracker* DBusTracker::instance()
{
  if (!s_instance) s_instance = new DBusTracker();
  return s_instance;
}

void DBusTracker::track(const QString &service)
{
  QMutexLocker lk(&m_mutex);
  m_tracked.insert(service);
}

void DBusTracker::onNameOwnerChanged(QString name, QString /*old_owner*/, QString new_owner)
{
  QMutexLocker lk(&m_mutex);
  if (new_owner.length() < 1 && m_tracked.contains(name))
    {
      m_tracked.remove(name);
      emit serviceDisappeared(name);
    }
}
