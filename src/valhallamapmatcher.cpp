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
static const QString propLatitude{"latitude"};
static const QString propLongitude{"longitude"};
static const QString propStreetName{"street_name"};
static const QString propStreetSpeedAssumed{"street_speed_assumed"};
static const QString propStreetSpeedLimit{"street_speed_limit"};
static const QString propDirection{"direction"};
static const QString propDirectionValid{"direction_valid"};


ValhallaMapMatcher::ValhallaMapMatcher(int mode) :
  QObject(),
  m_mode(int2mode(mode))
{
  preFillRequest();
}

ValhallaMapMatcher::~ValhallaMapMatcher()
{
}

bool ValhallaMapMatcher::start()
{
  InfoHub::logInfo(tr("Map matching requested: mode=%1").arg(mode2str(m_mode)));

  // reset properties cache to force update
  m_properties = Properties();
  m_last_position_info = Point();

  return true;
}

bool ValhallaMapMatcher::stop()
{
  // reset properties cache to force update
  m_properties = Properties();
  m_last_position_info = Point();

  return true;
}

QString ValhallaMapMatcher::update(double lat, double lon, double accuracy)
{
  // do we need to make update or the new point is
  // the same as the last one?
  QGeoCoordinate c(lat, lon);
  Point curr_point(c, accuracy);
  if (m_last_position_info.coordinate.isValid() &&
      c.distanceTo(m_last_position_info.coordinate) < const_min_coordinate_change)
    return "{}";

  // check if there is at least one location
  // in the cache. map matching requires at least
  // two points
  if (m_locations.size() < 1)
    {
      m_locations.append(curr_point);
      return "{}";
    }

  // compose coordinate array
  QJsonArray shape;
  for (const auto &p: m_locations)
    {
      const QGeoCoordinate &c = p.coordinate;
      accuracy = std::max(accuracy, p.accuracy);
      QJsonObject o;
      o.insert("lat", c.latitude());
      o.insert("lon", c.longitude());
      shape.append(o);
    }

  // insert the last recorded point
  QJsonObject o;
  o.insert("lat", lat);
  o.insert("lon", lon);
  shape.append(o);

  // map match for all requested modes
  QJsonObject response;

  QByteArray request;
  fillRequest(shape, accuracy, request);

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
          return "{}";
        }

      QJsonObject p = points.last().toObject();

      // check that we have all required keys in matched point
      if ( !p.contains("distance_along_edge") ||
           !p.contains("lat") ||
           !p.contains("lon") ||
           !p.contains("edge_index") ||
           !p.contains("type") )
        return "{}";

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

                  setProperty(propStreetName, street_name, response);
                  setProperty(propDirection, direction, response);
                  setProperty(propDirectionValid, 1, response);
                  setProperty(propStreetSpeedAssumed, speed, response);
                  setProperty(propStreetSpeedLimit, speed_limit, response);
                }
            }
        }

      setProperty(propLatitude, cmatch.latitude(), response);
      setProperty(propLongitude, cmatch.longitude(), response);

      if (!street_found)
        {
          setProperty(propStreetName, "", response);
          setProperty(propStreetSpeedAssumed, 0.0, response);
          setProperty(propStreetSpeedLimit, 0.0, response);
          setProperty(propDirection, 0.0, response);
          setProperty(propDirectionValid, 0, response);
        }
    }

  // save last location
  if (m_locations.back().coordinate.distanceTo(c) > const_min_distance_to_record)
    {
      m_locations.append(curr_point);
      if (m_locations.size() > const_max_recorded_points)
        m_locations.pop_front();
    }

  m_last_position_info = curr_point;

  QJsonDocument d(response);
  return d.toJson(QJsonDocument::Compact);
}

QString ValhallaMapMatcher::mode2str(Mode mode)
{
  switch (mode) {
    case Auto: return "auto"; break;
    case AutoShorter: return "auto_shorter";
    case Bicycle: return "bicycle";
    case Bus: return "bus";
    case Pedestrian: return "pedestrian";
    default: return "Unknown";
    }
  return "Unknown";
}

ValhallaMapMatcher::Mode ValhallaMapMatcher::int2mode(int i)
{
  if (i>=1 && i<=5) return Mode(i);
  return Unknown;
}

void ValhallaMapMatcher::preFillRequest()
{
  QJsonObject r;
  switch (m_mode) {
    case Auto: r.insert("costing", "auto"); break;
    case AutoShorter: r.insert("costing", "auto_shorter"); break;
    case Bicycle: r.insert("costing", "bicycle"); break;
    case Bus: r.insert("costing", "bus"); break;
    case Pedestrian: r.insert("costing", "pedestrian"); break;
    default:
      InfoHub::logWarning(tr("Map matching mode %1 is not supported").arg(m_mode));
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
  att.append("matched.type");
  att.append("matched.edge_index");
  att.append("matched.distance_along_edge");

  QJsonObject filters;
  filters.insert("attributes", att);
  filters.insert("action", "include");

  QJsonObject dopt;
  dopt.insert("units", "kilometers");

  r.insert("filters", filters);
  r.insert("direction_options", dopt);


  m_request_base = r;
}

void ValhallaMapMatcher::fillRequest(const QJsonArray &shape, double accuracy, QByteArray &request)
{
  QJsonObject r(m_request_base);

  r.insert("shape", shape);
  r.insert("gps_accuracy", accuracy);

  QJsonDocument d(r);
  request = d.toJson(QJsonDocument::Compact);
}

void ValhallaMapMatcher::setProperty(const QString &key, int value, QJsonObject &response)
{
  if (m_properties.set(key, value))
    response.insert(key, value);
}

void ValhallaMapMatcher::setProperty(const QString &key, double value, QJsonObject &response)
{
  if (m_properties.set(key, value))
    response.insert(key, value);
}

void ValhallaMapMatcher::setProperty(const QString &key, const QString &value, QJsonObject &response)
{
  if (m_properties.set(key, value))
    response.insert(key, value);
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

#endif
