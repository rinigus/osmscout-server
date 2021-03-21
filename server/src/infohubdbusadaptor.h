#ifndef INFOHUBDBUSADAPTOR_H
#define INFOHUBDBUSADAPTOR_H

#include "infohub.h"
#include "config.h"

#include <QDBusAbstractAdaptor>

class InfoHubDBusAdaptor : public QDBusAbstractAdaptor
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", DBUS_INTERFACE_INFOHUB)

  Q_PROPERTY(bool error READ error NOTIFY errorChanged)
  Q_PROPERTY(int queue READ queue NOTIFY queueChanged)

public:
  InfoHubDBusAdaptor(InfoHub *parent);

  bool error() { return m->error(); }
  int queue() { return m->queue(); }

signals:
  void errorChanged(bool error);
  void queueChanged(int queue);

private:
  InfoHub *m;
};

#endif // INFOHUBDBUSADAPTOR_H
