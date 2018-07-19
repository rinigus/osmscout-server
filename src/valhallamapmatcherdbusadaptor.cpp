#include "valhallamapmatcherdbusadaptor.h"

ValhallaMapMatcherDBusAdaptor::ValhallaMapMatcherDBusAdaptor(ValhallaMapMatcherDBus *p):
  QDBusAbstractAdaptor(p), m(p)
{
}

QString ValhallaMapMatcherDBusAdaptor::update(int mode, double lat, double lon, double accuracy, const QDBusMessage &message)
{
  return m->update(mode,lat,lon,accuracy,message);
}

bool ValhallaMapMatcherDBusAdaptor::reset(int mode, const QDBusMessage &message)
{
  return m->reset(mode,message);
}

bool ValhallaMapMatcherDBusAdaptor::stop(int mode, const QDBusMessage &message)
{
  return m->stop(mode,message);
}

bool ValhallaMapMatcherDBusAdaptor::stop(const QDBusMessage &message)
{
  return m->stop(message);
}
