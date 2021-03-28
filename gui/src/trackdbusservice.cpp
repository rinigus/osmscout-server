#include "trackdbusservice.h"

#include "config-common.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>

TrackDBusService::TrackDBusService(QObject *parent) :
  QObject(parent),
  m_watcher(DBUS_SERVICE, QDBusConnection::sessionBus())
{
  connect(&m_watcher, &QDBusServiceWatcher::serviceRegistered,
          this, &TrackDBusService::serviceUpdate);
  connect(&m_watcher, &QDBusServiceWatcher::serviceUnregistered,
          this, &TrackDBusService::serviceUpdate);
  serviceUpdate(QStringLiteral());
}

void TrackDBusService::serviceUpdate(const QString &/*service*/)
{
  QDBusReply<QString> r = QDBusConnection::sessionBus().interface()->serviceOwner(DBUS_SERVICE);
  m_available = r.isValid();
  // trigger signals to reload data before notification of
  // availibility for QML
  if (m_available)
    emit serviceAppeared();
  emit availableChanged(m_available);
}
