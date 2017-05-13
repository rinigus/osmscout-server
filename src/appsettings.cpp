#include "appsettings.h"
#include "config.h"

#include <QThread>
#include <QStandardPaths>
#include <QTimer>

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
  beginGroup("http-listener");
  CHECK("host", "127.0.0.1");
  CHECK("port", 8553);
  //CHECK("maxThreads", QThread::idealThreadCount() + 2);
  endGroup();

  CHECK(OSM_SETTINGS "units", 0);

  // defaults for libosmscout
#ifdef IS_SAILFISH_OS
  QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  CHECK(MAPMANAGER_SETTINGS "root", documents + "/Maps.OSMScoutServer");
#else
  CHECK(MAPMANAGER_SETTINGS "root", "Maps");
#endif

  CHECK(MAPMANAGER_SETTINGS "map_selected", "");
  CHECK(MAPMANAGER_SETTINGS "osmscout", 1);
  CHECK(MAPMANAGER_SETTINGS "geocoder_nlp", 0);
  CHECK(MAPMANAGER_SETTINGS "postal_country", 0);
  CHECK(MAPMANAGER_SETTINGS "mapnik", 0);
  CHECK(MAPMANAGER_SETTINGS "max_download_speed_in_kbps", -1);
  CHECK(MAPMANAGER_SETTINGS "development_disable_url_update", 0);
  CHECK(MAPMANAGER_SETTINGS "assume_files_exist", 0);

  // force URL setting
  QSettings::setValue(MAPMANAGER_SETTINGS "provided_url",
                      "https://raw.githubusercontent.com/rinigus/osmscout-server/master/scripts/import/provided/url.json");

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

  CHECK(OSM_SETTINGS "rollingLoggerSize", 10);
  CHECK(OSM_SETTINGS "logInfo", 1);
  CHECK(OSM_SETTINGS "logSession", 0);

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

  CHECK(MAPNIKMASTER_SETTINGS "use_mapnik", 0);
#ifdef IS_SAILFISH_OS
  CHECK(MAPNIKMASTER_SETTINGS "scale", 3.0);
#else
  CHECK(MAPNIKMASTER_SETTINGS "scale", 1.0);
#endif
  CHECK(MAPNIKMASTER_SETTINGS "buffer_size_in_pixels", 64);
  CHECK(MAPNIKMASTER_SETTINGS "configuration_dir", DATA_PREFIX "mapnik/OSMBright");
}

void AppSettings::setValue(const QString &key, const QVariant &value)
{
  QSettings::setValue(key, value);

  if (key.contains(OSM_SETTINGS) ||
      key.contains(ROUTING_SPEED_SETTINGS) ||
      key.contains(GEOMASTER_SETTINGS) ||
      key.contains(MAPNIKMASTER_SETTINGS) ||
      key.contains(MAPMANAGER_SETTINGS))
    {
      // this delayed signal execution prevents fireing signals together
      // if there are many changes in settings
      if (!m_signal_osm_scout_changed_waiting)
        {
          m_signal_osm_scout_changed_waiting = true;
          QTimer::singleShot(200, this, SLOT(fireOsmScoutSettingsChanged()));
        }
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
  return value(OSM_SETTINGS "units", 0).toInt();
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
  if (key == OSM_SETTINGS "routingCostLimitDistance")
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
