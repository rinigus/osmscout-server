#ifdef USE_VALHALLA

#include "valhallamapmatcherdbusadaptor.h"

ValhallaMapMatcherDBusAdaptor::ValhallaMapMatcherDBusAdaptor(ValhallaMapMatcherDBus *p):
  QDBusAbstractAdaptor(p), m(p)
{
  setAutoRelaySignals(true);
}

QString ValhallaMapMatcherDBusAdaptor::Update(int mode, double lat, double lon, double accuracy, const QDBusMessage &message)
{
  return m->update(mode,lat,lon,accuracy,message);
}

bool ValhallaMapMatcherDBusAdaptor::Reset(int mode, const QDBusMessage &message)
{
  return m->reset(mode,message);
}

bool ValhallaMapMatcherDBusAdaptor::Stop(int mode, const QDBusMessage &message)
{
  return m->stop(mode,message);
}

bool ValhallaMapMatcherDBusAdaptor::Stop(const QDBusMessage &message)
{
  return m->stop(message);
}

#endif
