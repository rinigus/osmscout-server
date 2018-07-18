#ifdef USE_VALHALLA

#include "valhallamapmatcher.h"
#include "config.h"
#include "infohub.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>

// constants used in algorithm decisions
static const qreal const_min_coordinate_change = 1.0; // in meters
static const qreal const_min_distance_to_record = 10.0; // in meters
static const int   const_max_recorded_points = 10; // max number of points in location cache

// property key values
static const QString propCoordinate{"coordinate"};
static const QString propStreetName{"street_name"};
static const QString propStreetSpeedAssumed{"street_speed_assumed"};
static const QString propStreetSpeedLimit{"street_speed_limit"};
static const QString propDirection{"direction"};
static const QString propDirectionValid{"direction_valid"};


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

#ifdef VALHALLA_MAP_MATCHER_TESTING
  startTimer(1000);
#endif
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

bool ValhallaMapMatcher::start(const QString &id, const Mode mode)
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

  if (!info.isValid() || !info.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
    return;

  // do we need to make update or the new point is
  // the same as the last one?
  QGeoCoordinate c = info.coordinate();
  if (m_last_position_info.isValid() &&
      c.distanceTo(m_last_position_info.coordinate()) < const_min_coordinate_change)
    return;

  // check if there is at least one location
  // in the cache. map matching requires at least
  // two points
  if (m_locations.size() < 1)
    {
      m_locations.append(info);
      return;
    }

  // compose coordinate array
  QJsonArray shape;
  float accuracy = info.attribute(QGeoPositionInfo::HorizontalAccuracy);
  for (const QGeoPositionInfo &p: m_locations)
    {
      QGeoCoordinate c = p.coordinate();
      accuracy = std::max(accuracy, (float)p.attribute(QGeoPositionInfo::HorizontalAccuracy));
      QJsonObject o;
      o.insert("lat", c.latitude());
      o.insert("lon", c.longitude());
      shape.append(o);
    }

  // insert the last recorded point
  QJsonObject o;
  o.insert("lat", c.latitude());
  o.insert("lon", c.longitude());
  shape.append(o);

  // map match for all requested modes
  for (auto pmode = m_clients.keyBegin(); pmode != m_clients.keyEnd(); ++pmode)
    {
      const Mode mode = *pmode;
      QByteArray request;
      fillRequest(mode, shape, accuracy, request);

      qDebug() << "Request: " << request.toStdString().c_str();

      QByteArray res;
      if ( valhallaMaster->callActor(ValhallaMaster::TraceAttributes, request, res) )
        {
          QJsonObject d = QJsonDocument::fromJson(res).object();
          QJsonArray points = d.value("matched_points").toArray();
          QJsonArray edges = d.value("edges").toArray();

          if (points.size() != shape.size())
            {
              // technical message, should not occure => no translation needed
              InfoHub::logWarning("Something is wrong with map matching: mismatch of number of points");
              continue;
            }

          QJsonObject p = points.last().toObject();

          // check that we have all required keys in matched point
          if ( !p.contains("distance_along_edge") ||
               !p.contains("lat") ||
               !p.contains("lon") ||
               !p.contains("edge_index") ||
               !p.contains("type") )
            continue;

          bool street_found = true;
          QGeoCoordinate cmatch(c);
          if ( p.value("type").toString() != "matched" )
            street_found = false;
          else
            {
              cmatch.setLatitude(p.value("lat").toDouble());
              cmatch.setLongitude(p.value("lon").toDouble());

              double ed = p.value("distance_along_edge").toDouble();
              int ei = p.value("edge_index").toInt();

              setProperty(mode, propCoordinate, c);

              if (ei<0 ||
                  edges.size() <= ei)
                street_found = false;
              else
                {
                  QJsonObject e = edges.at(ei).toObject();
                  if ( !e.contains("begin_heading") ||
                       !e.contains("end_heading") )
                    street_found = false;
                  else
                    {
                      double a0 = e.value("begin_heading").toDouble() / 180.0 * M_PI;
                      double a1 = e.value("end_heading").toDouble() / 180.0 * M_PI;
                      double d0_x = cos(a0);
                      double d0_y = sin(a0);
                      double d1_x = cos(a1);
                      double d1_y = sin(a1);
                      double direction = atan2(d0_y*(1-ed)+d1_y*ed, d0_x*(1-ed)+d1_x*ed);

                      QString street_name="";
                      for (auto v: e.value("names").toArray())
                        {
                          if (street_name.length() > 0)
                            street_name += "; ";
                          street_name += v.toString();
                        }

                      double speed = e.value("speed").toDouble(-1);
                      double speed_limit = e.value("speed_limit").toDouble(-1);

                      setProperty(mode, propStreetName, street_name);
                      setProperty(mode, propDirection, direction);
                      setProperty(mode, propDirectionValid, 1);
                      setProperty(mode, propStreetSpeedAssumed, speed);
                      setProperty(mode, propStreetSpeedLimit, speed_limit);
                    }
                }
            }

          setProperty(mode, propCoordinate, cmatch);

          if (!street_found)
            {
              setProperty(mode, propStreetName, "");
              setProperty(mode, propStreetSpeedAssumed, 0.0);
              setProperty(mode, propStreetSpeedLimit, 0.0);
              setProperty(mode, propDirection, 0.0);
              setProperty(mode, propDirectionValid, 0);
            }
        }
    }

  // save last location
  if (m_locations.back().coordinate().distanceTo(c) > const_min_distance_to_record)
    {
      m_locations.append(info);
      if (m_locations.size() > const_max_recorded_points)
        m_locations.pop_front();
    }

  m_last_position_info = info;
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

void ValhallaMapMatcher::fillRequest(Mode mode, const QJsonArray &shape, double accuracy, QByteArray &request)
{
  QJsonObject r;
  switch (mode) {
    case Auto: r.insert("costing", "auto"); break;
    case AutoShorter: r.insert("costing", "auto_shorter"); break;
    case Bicycle: r.insert("costing", "bicycle"); break;
    case Bus: r.insert("costing", "bus"); break;
    case Pedestrian: r.insert("costing", "pedestrian"); break;
    default:
      InfoHub::logWarning(tr("Map matching mode %1 is not supported").arg(mode));
      return;
    }

  r.insert("shape_match", "map_snap");

  QJsonArray att;
  att.append("edge.names");
  att.append("edge.id");
  att.append("edge.speed_limit");
  att.append("edge.speed");
  att.append("edge.begin_heading");
  att.append("edge.end_heading");
  att.append("matched.point");
  att.append("matched.edge_index");
  att.append("matched.distance_along_edge");

  QJsonObject filters;
  filters.insert("attributes", att);
  filters.insert("action", "include");

  QJsonObject dopt;
  dopt.insert("units", "kilometers");

  r.insert("filters", filters);
  r.insert("direction_options", dopt);

  r.insert("shape", shape);
  r.insert("gps_accuracy", accuracy);

  QJsonDocument d(r);
  request = d.toJson();
}

void ValhallaMapMatcher::setProperty(Mode mode, const QString &key, int value)
{
  if (m_properties[mode].set(key, value))
    emit propertyChanged(mode, key, value);
}

void ValhallaMapMatcher::setProperty(Mode mode, const QString &key, double value)
{
  if (m_properties[mode].set(key, value))
    emit propertyChanged(mode, key, value);
}

void ValhallaMapMatcher::setProperty(Mode mode, const QString &key, const QString &value)
{
  if (m_properties[mode].set(key, value))
    emit propertyChanged(mode, key, value);
}

void ValhallaMapMatcher::setProperty(Mode mode, const QString &key, const QGeoCoordinate &value)
{
  if (m_properties[mode].set(key, value))
    emit propertyChanged(mode, key, value);
}

bool ValhallaMapMatcher::Properties::set(const QString &key, int value)
{
  if (m_property_int.contains(key) &&
      m_property_int[key] == value)
    return false;
  m_property_int[key] = value;
  return true;
}

bool ValhallaMapMatcher::Properties::set(const QString &key, double value)
{
  if (m_property_double.contains(key) &&
      fabs(m_property_double[key]-value) < 1e-10)
    return false;
  m_property_double[key] = value;
  return true;
}

bool ValhallaMapMatcher::Properties::set(const QString &key, const QString &value)
{
  if (m_property_string.contains(key) &&
      m_property_string[key] == value)
    return false;
  m_property_string[key] = value;
  return true;
}

bool ValhallaMapMatcher::Properties::set(const QString &key, const QGeoCoordinate &value)
{
  if (m_property_coor.contains(key) &&
      m_property_coor[key].distanceTo(value) < 1e-10)
    return false;
  m_property_coor[key] = value;
  return true;
}

#ifdef VALHALLA_MAP_MATCHER_TESTING
void ValhallaMapMatcher::timerEvent(QTimerEvent *)
{
  static double lat = 59.437;
  static double lon = 24.7536;

  QGeoPositionInfo p;
  p.setCoordinate( QGeoCoordinate(lat, lon, 0) );
  p.setTimestamp(QDateTime::currentDateTime());
  p.setAttribute(QGeoPositionInfo::HorizontalAccuracy, 15);

  onPositionUpdated(p);

  lat += 1e-4;
  lon -= 1e-4;
}
#endif

#endif
