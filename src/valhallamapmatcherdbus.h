#ifndef VALHALLAMAPMATCHERDBUS_H
#define VALHALLAMAPMATCHERDBUS_H

#include "config.h"

#include <QDBusAbstractAdaptor>
#include <QDBusMessage>

class ValhallaMapMatcherDBus : public QDBusAbstractAdaptor
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", DBUS_INTERFACE_MAPMATCHING)

public:
  ValhallaMapMatcherDBus(QObject *parent);
  ~ValhallaMapMatcherDBus();

public slots:
  void start(const QDBusMessage &message);
//  bool start(int mode);
//  bool stop(int mode);
//  bool stop();

protected:
};

#endif // VALHALLAMAPMATCHERDBUS_H
