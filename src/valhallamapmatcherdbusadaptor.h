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
  Q_PROPERTY(bool Active READ active NOTIFY ActiveChanged)

public:
  ValhallaMapMatcherDBusAdaptor(ValhallaMapMatcherDBus *parent);

  bool active() const { return true; }

signals:
  void ActiveChanged(bool active);

public slots:
  QString Update(int mode, double lat, double lon, double accuracy, const QDBusMessage &message);

  bool Reset(int mode, const QDBusMessage &message);
  bool Stop(int mode, const QDBusMessage &message);
  bool Stop(const QDBusMessage &message);

private:
  ValhallaMapMatcherDBus *m;
};

#endif // VALHALLAMAPMATCHERDBUSADAPTOR_H
