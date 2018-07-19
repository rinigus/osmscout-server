#ifndef VALHALLAMAPMATCHERDBUS_H
#define VALHALLAMAPMATCHERDBUS_H

#include "config.h"
#include "valhallamapmatcher.h"

#include <QDBusAbstractAdaptor>
#include <QDBusMessage>
#include <QHash>
#include <QPointer>

class ValhallaMapMatcherDBus : public QDBusAbstractAdaptor
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", DBUS_INTERFACE_MAPMATCHING)

public:
  ValhallaMapMatcherDBus(QObject *parent);
  ~ValhallaMapMatcherDBus();

public slots:
  QString update(double lat, double lon, double accuracy, const QDBusMessage &message);

  bool start(int mode, const QDBusMessage &message);
  bool stop(int mode, const QDBusMessage &message);
  bool stop(const QDBusMessage &message);

private:
  QHash<QString, QPointer<ValhallaMapMatcher> > m_matchers;
};

#endif // VALHALLAMAPMATCHERDBUS_H
