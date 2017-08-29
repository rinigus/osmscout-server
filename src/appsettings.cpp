#include "appsettings.h"
#include "config.h"

#include <QThread>
#include <QStandardPaths>
#include <QTimer>

#ifdef __linux__
#include <sys/sysinfo.h>
#endif

#include <QDebug>

#ifdef IS_SAILFISH_OS
#define DATA_PREFIX "/usr/share/harbour-osmscout-server/"
#endif

#ifdef IS_CONSOLE_QT
#define DATA_PREFIX ""
#endif

AppSettings::AppSettings():
  QSettings()
{
}

#define CHECK(s, d) if (!contains(s)) QSettings::setValue(s, d);
#define CHECKF(s, d) if (!contains(s)) QSettings::setValue(s, (double)d);
void AppSettings::initDefaults()
{
  // defaults for server
  CHECK(HTTP_SERVER_SETTINGS "host", "127.0.0.1");
  CHECK(HTTP_SERVER_SETTINGS "port", 8553);
  //CHECK("maxThreads", QThread::idealThreadCount() + 2);

  /////////////////////////////////////////
  /// general settings

  CHECK(GENERAL_SETTINGS "units", 0);
  CHECK(GENERAL_SETTINGS "profile", 0);
  CHECK(GENERAL_SETTINGS "language", 0);

  CHECK(GENERAL_SETTINGS "rollingLoggerSize", 10);
  CHECK(GENERAL_SETTINGS "logInfo", 1);
  CHECK(GENERAL_SETTINGS "logSession", 0);

  CHECK(GENERAL_SETTINGS "firstTime", 1);
  CHECK(GENERAL_SETTINGS "lastRunVersion", 0);

#ifdef IS_SAILFISH_OS
  QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  CHECK(MAPMANAGER_SETTINGS "root", documents + "/Maps.OSMScoutServer");
#else
  CHECK(MAPMANAGER_SETTINGS "root", "Maps");
#endif

  /////////////////////////////////////////
  /// defaults for map manager

  CHECK(MAPMANAGER_SETTINGS "map_selected", "");
  CHECK(MAPMANAGER_SETTINGS "osmscout", 0);
  CHECK(MAPMANAGER_SETTINGS "geocoder_nlp", 1);
  CHECK(MAPMANAGER_SETTINGS "postal_country", 1);
  CHECK(MAPMANAGER_SETTINGS "mapnik", 1);
  CHECK(MAPMANAGER_SETTINGS "valhalla", 1);
  CHECK(MAPMANAGER_SETTINGS "max_download_speed_in_kbps", -1);
  CHECK(MAPMANAGER_SETTINGS "development_disable_url_update", 0);
  CHECK(MAPMANAGER_SETTINGS "assume_files_exist", 0);

  // force URL setting
  QSettings::setValue(MAPMANAGER_SETTINGS "provided_url",
                      "https://raw.githubusercontent.com/rinigus/osmscout-server/master/scripts/import/provided/url.json");

  /////////////////////////////////////////
  /// defaults for libosmscout

  /// used internally by MapManager to set the path - will be modified when support
  /// for multi-map handling is be ready
  CHECK(OSM_SETTINGS "map", "");

  CHECK(OSM_SETTINGS "style", DATA_PREFIX "stylesheets/standard.oss");
  CHECK(OSM_SETTINGS "icons", DATA_PREFIX "data/icons/28x28/standard");
  CHECK(OSM_SETTINGS "fontSize", 5.0);
  CHECK(OSM_SETTINGS "renderSea", 1);
  CHECK(OSM_SETTINGS "drawBackground", 1);
  CHECK(OSM_SETTINGS "dataLookupArea", 1.25);
  CHECK(OSM_SETTINGS "tileBordersZoomCutoff", 16);

  CHECK(OSM_SETTINGS "routingCostLimitDistance", 50.0);
  CHECK(OSM_SETTINGS "routingCostLimitFactor", 5.0);

  CHECK(ROUTING_SPEED_SETTINGS "highway_living_street", 10);
  CHECK(ROUTING_SPEED_SETTINGS "highway_motorway", 110);
  CHECK(ROUTING_SPEED_SETTINGS "highway_motorway_junction", 60);
  CHECK(ROUTING_SPEED_SETTINGS "highway_motorway_link", 60);
  CHECK(ROUTING_SPEED_SETTINGS "highway_motorway_primary", 70);
  CHECK(ROUTING_SPEED_SETTINGS "highway_motorway_trunk", 100);
  CHECK(ROUTING_SPEED_SETTINGS "highway_primary", 70);
  CHECK(ROUTING_SPEED_SETTINGS "highway_primary_link", 60);
  CHECK(ROUTING_SPEED_SETTINGS "highway_residential", 40);
  CHECK(ROUTING_SPEED_SETTINGS "highway_road", 50);
  CHECK(ROUTING_SPEED_SETTINGS "highway_roundabout", 40);
  CHECK(ROUTING_SPEED_SETTINGS "highway_secondary", 60);
  CHECK(ROUTING_SPEED_SETTINGS "highway_secondary_link", 50);
  CHECK(ROUTING_SPEED_SETTINGS "highway_service", 30);
  CHECK(ROUTING_SPEED_SETTINGS "highway_tertiary", 55);
  CHECK(ROUTING_SPEED_SETTINGS "highway_tertiary_link", 55);
  CHECK(ROUTING_SPEED_SETTINGS "highway_trunk", 100);
  CHECK(ROUTING_SPEED_SETTINGS "highway_trunk_link", 60);
  CHECK(ROUTING_SPEED_SETTINGS "highway_unclassified", 50);
  CHECK(ROUTING_SPEED_SETTINGS "Bicycle", 20);
  CHECK(ROUTING_SPEED_SETTINGS "Foot", 5);
  CHECK(ROUTING_SPEED_SETTINGS "Car", 160);

  // Fix icons dir setting if coming from earlier versions.
  QString icons = valueString(OSM_SETTINGS "icons");
  if (icons.size() > 1 && icons.at(icons.size()-1) == '/')
    {
      qDebug() << "Looks like icons path has trailing /: " << icons;
      icons.chop(1);
      qDebug() << "New icons path: " << icons;
      setValue(OSM_SETTINGS "icons", icons);
    }

  CHECK(GEOMASTER_SETTINGS "use_geocoder_nlp", 0);
  CHECK(GEOMASTER_SETTINGS "initialize_every_call", 0);
  CHECK(GEOMASTER_SETTINGS "use_primitive", 1);
  CHECK(GEOMASTER_SETTINGS "max_queries_per_hierarchy", 30);
  CHECK(GEOMASTER_SETTINGS "languages", "");
  CHECK(GEOMASTER_SETTINGS "search_all_maps", 1);
  CHECK(GEOMASTER_SETTINGS "continue_search_if_hit_found", 1);

  /////////////////////////////////////////
  /// mapnik settings

  CHECK(MAPNIKMASTER_SETTINGS "use_mapnik", 0);
#ifdef IS_SAILFISH_OS
  CHECK(MAPNIKMASTER_SETTINGS "scale", 3.0);
#else
  CHECK(MAPNIKMASTER_SETTINGS "scale", 1.0);
#endif
  CHECK(MAPNIKMASTER_SETTINGS "buffer_size_in_pixels", 64);
  CHECK(MAPNIKMASTER_SETTINGS "styles_dir", DATA_PREFIX "mapnik");

  /////////////////////////////////////////
  /// valhalla settings

  CHECK(VALHALLA_MASTER_SETTINGS "use_valhalla", 0);
  {
    int cache_size_default = 128;
#ifdef __linux__
    struct sysinfo s;
    if (sysinfo(&s) == 0)
      {
        double ram = s.totalram/1024./1024./1024.;
        if ( ram < 1.25 ) cache_size_default = 16;
        else if ( ram < 2.1) cache_size_default = 64;
      }
#endif
    CHECK(VALHALLA_MASTER_SETTINGS "cache_in_mb", cache_size_default);
  }
  CHECK(VALHALLA_MASTER_SETTINGS "route_port", 8554);
  CHECK(VALHALLA_MASTER_SETTINGS "limit_max_distance_auto", 5000.0);
  CHECK(VALHALLA_MASTER_SETTINGS "limit_max_distance_bicycle", 100.0);
  CHECK(VALHALLA_MASTER_SETTINGS "limit_max_distance_pedestrian", 75.0);

  CHECK(REQUEST_MAPPER_SETTINGS "idle_timeout", 3600);

  /// set profile if specified
  setProfile();

  /// set the notification of the first time use
  m_first_time = valueBool(GENERAL_SETTINGS "firstTime");
  setValue(GENERAL_SETTINGS "firstTime", 0);
}

void AppSettings::setValue(const QString &key, const QVariant &value)
{
  QSettings::setValue(key, value);

  if (key.contains(OSM_SETTINGS) ||
      key.contains(ROUTING_SPEED_SETTINGS) ||
      key.contains(GEOMASTER_SETTINGS) ||
      key.contains(MAPNIKMASTER_SETTINGS) ||
      key.contains(VALHALLA_MASTER_SETTINGS) ||
      key.contains(MAPMANAGER_SETTINGS) ||
      key.contains(REQUEST_MAPPER_SETTINGS) ||
      key == GENERAL_SETTINGS "language" )
    {
      // this delayed signal execution prevents fireing signals together
      // if there are many changes in settings
      if (!m_signal_osm_scout_changed_waiting)
        {
          m_signal_osm_scout_changed_waiting = true;
          QTimer::singleShot(200, this, SLOT(fireOsmScoutSettingsChanged()));
        }
    }

  else if (key == GENERAL_SETTINGS "profile")
    {
      setProfile();
    }
}

void AppSettings::fireOsmScoutSettingsChanged()
{
  emit osmScoutSettingsChanged();
  m_signal_osm_scout_changed_waiting = false;
  //qDebug() << "Signal fired";
}


int AppSettings::valueInt(const QString &key)
{
  return value(key, 0).toInt();
}

bool AppSettings::valueBool(const QString &key)
{
  return (value(key, 0).toInt() > 0);
}

double AppSettings::valueFloat(const QString &key)
{
  return value(key, 0).toFloat();
}

QString AppSettings::valueString(const QString &key)
{
  return value(key, QString()).toString();
}

///////////////////////////////////////////////////////
/// NB! Units have to be in sync here with the QML Settings
///////////////////////////////////////////////////////
int AppSettings::unitIndex() const
{
  return value(GENERAL_SETTINGS "units", 0).toInt();
}

int AppSettings::unitDisplayDecimals() const
{
  int i = unitIndex();

  if (i==1) return 1;

  return 0; /// default
}

bool AppSettings::hasUnits(const QString &key) const
{
  if (key == OSM_SETTINGS "routingCostLimitDistance" ||
      key.contains(VALHALLA_MASTER_SETTINGS "limit_max_distance") ||
      key.indexOf(ROUTING_SPEED_SETTINGS) == 0)
    return true;
  return false;
}

QString AppSettings::unitName(bool speed) const
{
  int i = unitIndex();

  if (i==1) return (speed ? tr("mph") : tr("mi."));

  return (speed ? tr("km/h") : tr("km")); /// default
}

QString AppSettings::unitName(const QString &key) const
{
  if (key == OSM_SETTINGS "routingCostLimitDistance" ||
      key.contains(VALHALLA_MASTER_SETTINGS "limit_max_distance"))
    return unitName(false);

  if (key.indexOf(ROUTING_SPEED_SETTINGS) == 0)
    return unitName(true);

  return QString();
}

double AppSettings::unitFactor() const
{
  int i = unitIndex();

  if (i==1) return 1.0 / 1.609344;

  return 1.0; /// default
}

///////////////////////////////////////////////////////
/// NB! Profiles have to be in sync here with the
/// QML Profile page
///////////////////////////////////////////////////////

void AppSettings::setProfile()
{
  int index = valueInt(GENERAL_SETTINGS "profile");
  bool profile_active = true;

  if (index == 0) // default profile: Mapnik / GeocoderNLP / Valhalla
    {
      setValue(MAPMANAGER_SETTINGS "osmscout", 0);
      setValue(MAPMANAGER_SETTINGS "geocoder_nlp", 1);
      setValue(MAPMANAGER_SETTINGS "postal_country", 1);
      setValue(MAPMANAGER_SETTINGS "mapnik", 1);
      setValue(MAPMANAGER_SETTINGS "valhalla", 1);

      setValue(GEOMASTER_SETTINGS "use_geocoder_nlp", 1);
      setValue(MAPNIKMASTER_SETTINGS "use_mapnik", 1);
      setValue(VALHALLA_MASTER_SETTINGS "use_valhalla", 1);
    }
  else if (index == 1) // libosmscout + geocoder-nlp
    {
      setValue(MAPMANAGER_SETTINGS "osmscout", 1);
      setValue(MAPMANAGER_SETTINGS "geocoder_nlp", 1);
      setValue(MAPMANAGER_SETTINGS "postal_country", 1);
      setValue(MAPMANAGER_SETTINGS "mapnik", 0);
      setValue(MAPMANAGER_SETTINGS "valhalla", 0);

      setValue(GEOMASTER_SETTINGS "use_geocoder_nlp", 1);
      setValue(MAPNIKMASTER_SETTINGS "use_mapnik", 0);
      setValue(VALHALLA_MASTER_SETTINGS "use_valhalla", 0);
    }
  else if (index == 2) // libosmscout
    {
      setValue(MAPMANAGER_SETTINGS "osmscout", 1);
      setValue(MAPMANAGER_SETTINGS "geocoder_nlp", 0);
      setValue(MAPMANAGER_SETTINGS "postal_country", 0);
      setValue(MAPMANAGER_SETTINGS "mapnik", 0);
      setValue(MAPMANAGER_SETTINGS "valhalla", 0);

      setValue(GEOMASTER_SETTINGS "use_geocoder_nlp", 0);
      setValue(MAPNIKMASTER_SETTINGS "use_mapnik", 0);
      setValue(VALHALLA_MASTER_SETTINGS "use_valhalla", 0);
    }
  // all other profiles are either custom (index=3) or unknown
  else
    profile_active = false;

  if (profile_active != m_profiles_used)
    {
      m_profiles_used = profile_active;
      emit profilesUsedChanged(m_profiles_used);
    }
}


////////////////////////////////////////////////////////////////////
/// Language preference support, has to be in sync with QML Settings
/// page
////////////////////////////////////////////////////////////////////

QString AppSettings::preferredLanguage()
{
  int index = valueInt(GENERAL_SETTINGS "language");
  if ( index == 1 ) return "en";
  return QString();
}
