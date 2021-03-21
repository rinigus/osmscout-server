#include "infohubdbusadaptor.h"

InfoHubDBusAdaptor::InfoHubDBusAdaptor(InfoHub *p):
  QDBusAbstractAdaptor(p), m(p)
{
  connect(m, &InfoHub::errorChanged,
          this, &InfoHubDBusAdaptor::errorChanged, Qt::QueuedConnection);
  connect(m, &InfoHub::queueChanged,
          this, &InfoHubDBusAdaptor::queueChanged, Qt::QueuedConnection);
}
