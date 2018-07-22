#ifndef VALHALLAMAPMATCHERDBUS_H
#define VALHALLAMAPMATCHERDBUS_H

#include "valhallamapmatcher.h"

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QHash>
#include <QSharedPointer>

class ValhallaMapMatcherDBus : public QObject
{
  Q_OBJECT

public:
  ValhallaMapMatcherDBus(QObject *parent=nullptr);
  ~ValhallaMapMatcherDBus();

  void activate();

public slots:
  //////////////////////////////////////////////////////////////////////////////////////////
  /// NB! when adding slots for DBus export, don't forget to add them in the adaptor as well

  QString update(int mode, double lat, double lon, double accuracy, const QDBusMessage &message);

  bool reset(int mode, const QDBusMessage &message);
  bool stop(int mode, const QDBusMessage &message);
  bool stop(const QDBusMessage &message);

  // used to track lost clients - not for export on dbus
  void onNameOwnerChanged(QString name, QString old_owner, QString new_owner);

signals:
  void activeChanged(bool);

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
