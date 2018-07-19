#ifndef VALHALLAMAPMATCHERDBUS_H
#define VALHALLAMAPMATCHERDBUS_H

#include "config.h"
#include "valhallamapmatcher.h"

#include <QDBusAbstractAdaptor>
#include <QDBusMessage>
#include <QHash>
#include <QSharedPointer>

class ValhallaMapMatcherDBus : public QDBusAbstractAdaptor
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", DBUS_INTERFACE_MAPMATCHING)

public:
  ValhallaMapMatcherDBus(QObject *parent);
  ~ValhallaMapMatcherDBus();

public slots:
  QString update(int mode, double lat, double lon, double accuracy, const QDBusMessage &message);

  bool reset(int mode, const QDBusMessage &message);
  bool stop(int mode, const QDBusMessage &message);
  bool stop(const QDBusMessage &message);

protected:
  QSharedPointer<ValhallaMapMatcher> get(int mode, const QString &caller);

private:
  /// \brief map matchers stored in a map
  ///
  /// For each client, a map matchers are organized according to
  /// the mode. This allows to have multiple modes per one client
  QHash< QString, QHash<int, QSharedPointer<ValhallaMapMatcher> > > m_matchers;
};

#endif // VALHALLAMAPMATCHERDBUS_H
