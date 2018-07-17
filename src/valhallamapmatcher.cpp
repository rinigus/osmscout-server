#ifdef USE_VALHALLA

#include "valhallamapmatcher.h"
#include "infohub.h"

#include <QDebug>

ValhallaMapMatcher::ValhallaMapMatcher(QObject *parent) : QObject(parent)
{
  m_source = QGeoPositionInfoSource::createDefaultSource(this);
  if (m_source==nullptr)
    {
      InfoHub::logWarning("Failed to allocate GeoPositioning source");
      return;
    }

  connect(m_source, &QGeoPositionInfoSource::positionUpdated, this, &ValhallaMapMatcher::onPositionUpdated);
  connect(m_source, &QGeoPositionInfoSource::updateTimeout, this, &ValhallaMapMatcher::onUpdateTimeout);
  connect(m_source, SIGNAL(error(QGeoPositionInfoSource::Error)),
          this, SLOT(onPositioningError(QGeoPositionInfoSource::Error)));
}

ValhallaMapMatcher::~ValhallaMapMatcher()
{
  shutdown();
}

void ValhallaMapMatcher::shutdown()
{
  stopPositioning();
  clearCache();
  m_clients.clear();
}

void ValhallaMapMatcher::stopPositioning()
{
  if (m_source)
    {
      m_source->stopUpdates();
      InfoHub::logInfo(tr("Positioning service stopped"));
      m_positioning_active = false;
      emit positioningActiveChanged(m_positioning_active);
    }
}

void ValhallaMapMatcher::clearCache()
{
  m_properties.clear();
  m_locations.clear();
  m_last_position_info = QGeoPositionInfo();
}

bool ValhallaMapMatcher::start(const QString &id, const QString &mode)
{
  InfoHub::logInfo(tr("Map matching requested: mode=%1").arg(mode));

  if (m_source == nullptr)
    {
      InfoHub::logWarning(tr("Geo-positioning service not available, cannot provide map matching"));
      return false;
    }

  m_clients[mode] << id;

  // reset properties cache to force update
  m_properties[mode] = Properties();

  if (!m_positioning_active)
    {
      InfoHub::logInfo(tr("Starting positioning service"));
      clearCache();
      m_source->startUpdates();
      m_positioning_active = true;
      emit positioningActiveChanged(m_positioning_active);
    }

  return true;
}

void ValhallaMapMatcher::onPositionUpdated(const QGeoPositionInfo &info)
{
  qDebug() << "New position: " << info;

  // do we need to make update or the new point is
  // the same as the last one?


  static int iii=0;
  if (iii>10) shutdown();
  iii++;
}

void ValhallaMapMatcher::onUpdateTimeout()
{
  InfoHub::logInfo(tr("Geo positioning not available within expected timeout. Waiting for positioning fix"));
  clearCache();
}

void ValhallaMapMatcher::onPositioningError(QGeoPositionInfoSource::Error positioningError)
{
  QString error;
  bool e = true;
  switch (positioningError) {
    case QGeoPositionInfoSource::AccessError: error = tr("Lacking positioning access rights"); break;
    case QGeoPositionInfoSource::ClosedError: error = tr("Connection to positioning source closed"); break;
    case QGeoPositionInfoSource::UnknownSourceError: error = tr("Unknown error from positioning source"); break;
    default: e = false;
    }

  if (!e) return; // we can ignore noerror

  InfoHub::logWarning(tr("Geo positioning error: %1").arg(error));

  shutdown();
}

#endif
