#ifndef VALHALLAMAPMATCHERDBUSADAPTOR_H
#define VALHALLAMAPMATCHERDBUSADAPTOR_H

#include "valhallamapmatcherdbus.h"
#include "config.h"

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusMessage>

class ValhallaMapMatcherDBusAdaptor : public QDBusAbstractAdaptor
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", DBUS_INTERFACE_MAPMATCHING)
  Q_PROPERTY(bool active READ active)

public:
  ValhallaMapMatcherDBusAdaptor(ValhallaMapMatcherDBus *parent);

  bool active() const { return true; }

public slots:
  QString update(int mode, double lat, double lon, double accuracy, const QDBusMessage &message);

  bool reset(int mode, const QDBusMessage &message);
  bool stop(int mode, const QDBusMessage &message);
  bool stop(const QDBusMessage &message);

private:
  ValhallaMapMatcherDBus *m;
};

#endif // VALHALLAMAPMATCHERDBUSADAPTOR_H
