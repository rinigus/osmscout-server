/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
 *
 * This file is part of OSM Scout Server.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "appsettings.h"
#include "config.h"

#include <QThread>
#include <QStandardPaths>
#include <QTimer>

#ifdef __linux__
#include <sys/sysinfo.h>
#endif

#include <QDebug>

#define DATA_PREFIX DEFAULT_DATA_PREFIX

AppSettings::AppSettings():
  QSettings()
{
  m_available_profile_index = availableProfilesIndex();
}

#define CHECK(s, d) if (!contains(s)) QSettings::setValue(s, d)
#define CHECKF(s, d) if (!contains(s)) QSettings::setValue(s, (double)d)
void AppSettings::initDefaults()
{
  // defaults for server
  CHECK(HTTP_SERVER_SETTINGS "host", "127.0.0.1");
  CHECK(HTTP_SERVER_SETTINGS "port", 8553);

  /////////////////////////////////////////
  /// general settings

  CHECK(GENERAL_SETTINGS "units", 0);
  CHECK(GENERAL_SETTINGS "profile", defaultProfile());
  CHECK(GENERAL_SETTINGS "language", 0);

  CHECK(GENERAL_SETTINGS "rollingLoggerSize", 10);
  CHECK(GENERAL_SETTINGS "logInfo", 1);
  CHECK(GENERAL_SETTINGS "logSession", 0);

  CHECK(GENERAL_SETTINGS "firstTime", 1);
  CHECK(GENERAL_SETTINGS "lastRunVersion", GENERAL_APP_VERSION);

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
  CHECK(MAPMANAGER_SETTINGS "mapboxgl", 0);
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
  /// for multi-map handling will be ready
  CHECK(OSM_SETTINGS "map", "");

  CHECK(OSM_SETTINGS "style", DATA_PREFIX "styles/osmscout/stylesheets/standard.oss");
  CHECK(OSM_SETTINGS "icons", DATA_PREFIX "styles/osmscout/icons/28x28/standard");
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

  /////////////////////////////////////////
  /// geocoder-nlp settings

  CHECK(GEOMASTER_SETTINGS "use_geocoder_nlp", 1);
  CHECK(GEOMASTER_SETTINGS "initialize_every_call", 0);
  CHECK(GEOMASTER_SETTINGS "use_postal", 1);
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
  CHECK(MAPNIKMASTER_SETTINGS "styles_dir", DATA_PREFIX "styles/mapnik");

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

  CHECK(REQUEST_MAPPER_SETTINGS "idle_timeout", 1800);

  /// set the notification of the first time use
  m_first_time = valueBool(GENERAL_SETTINGS "firstTime");
  setValue(GENERAL_SETTINGS "firstTime", 0);

  /// for changelogs
  m_last_run_version = valueInt(GENERAL_SETTINGS "lastRunVersion");
  setValue(GENERAL_SETTINGS "lastRunVersion", GENERAL_APP_VERSION);

  /// profiles changed from version 1 to version 2
  if (m_last_run_version == 1)
    {
      int old_profile = valueInt(GENERAL_SETTINGS "profile");
      if (old_profile > 0) // inserted new profiles with indexes 1 and 2
        {
          setValue(GENERAL_SETTINGS "profile", old_profile + 2);
        }
    }

  /// location of styles changed in version 3
  if (m_last_run_version < 3)
    {
      setValue(OSM_SETTINGS "style", DATA_PREFIX "styles/osmscout/stylesheets/standard.oss");
      setValue(OSM_SETTINGS "icons", DATA_PREFIX "styles/osmscout/icons/28x28/standard");
      setValue(MAPNIKMASTER_SETTINGS "styles_dir", DATA_PREFIX "styles/mapnik");
    }

  /// profiles changed from version 2 to version 4
  if (m_last_run_version < 4)
    {
      int old_profile = valueInt(GENERAL_SETTINGS "profile");
      int new_profile = old_profile;
      if (old_profile == 0) new_profile = 1;
      else if (old_profile == 1) new_profile = 0;
      setValue(GENERAL_SETTINGS "profile", new_profile);
    }

  ///////////////////////////////////////////
  /// check and update settings in accordance
  /// with the available backends

  if (!m_available_profile_index.contains(valueInt(GENERAL_SETTINGS "profile")))
      setValue(GENERAL_SETTINGS "profile", defaultProfile());

  if (!hasBackendOsmScout())
    {
      setValue(MAPMANAGER_SETTINGS "osmscout", 0);
      setValue(GEOMASTER_SETTINGS "use_geocoder_nlp", 1);
      setValue(MAPNIKMASTER_SETTINGS "use_mapnik", 1);
      setValue(VALHALLA_MASTER_SETTINGS "use_valhalla", 1);
    }

  if (!hasBackendMapnik() && hasBackendOsmScout())
    {
      setValue(MAPMANAGER_SETTINGS "mapnik", 0);
      setValue(MAPNIKMASTER_SETTINGS "use_mapnik", 0);
    }

  if (!hasBackendValhalla() && hasBackendOsmScout())
    {
      setValue(MAPMANAGER_SETTINGS "valhalla", 0);
      setValue(VALHALLA_MASTER_SETTINGS "use_valhalla", 0);
    }

  /// set profile if specified (after all version checks)
  setProfile();

  /// check if country selection is needed
  checkCountrySelectionNeeded();
}

void AppSettings::setValue(const QString &key, const QVariant &value)
{
  QSettings::setValue(key, value);

  // http connection settings cannot be changed on fly

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

  // always check if country selection is needed
  checkCountrySelectionNeeded();
}

void AppSettings::fireOsmScoutSettingsChanged()
{
  emit osmScoutSettingsChanged();
  m_signal_osm_scout_changed_waiting = false;
  //qDebug() << "Signal fired";
}


int AppSettings::valueInt(const QString &key) const
{
  return value(key, 0).toInt();
}

bool AppSettings::valueBool(const QString &key) const
{
  return (value(key, 0).toInt() > 0);
}

double AppSettings::valueFloat(const QString &key) const
{
  return value(key, 0).toDouble();
}

QString AppSettings::valueString(const QString &key) const
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
/// NB! Profiles have to be set using either local
/// indexes (exposed methods) or internally via
/// configuration settings (protected methods)
///////////////////////////////////////////////////////

QStringList AppSettings::availableProfiles() const
{
  // compose list of locally available profile descriptions
  QStringList l;
  const QList<int> &prof = m_available_profile_index;
  if (prof.contains(0)) l << tr("Default");
  if (prof.contains(1)) l << tr("Recommended for raster tiles maps");
  if (prof.contains(2)) l << tr("Recommended for vector and raster tiles maps");
  if (prof.contains(3)) l << tr("<i>libosmscout</i> with <i>Geocoder-NLP</i>");
  if (prof.contains(4)) l << tr("<i>libosmscout</i>");
  if (prof.contains(5)) l << tr("Custom");
  return l;
}

int AppSettings::currentProfile() const
{
  const QList<int> &prof = m_available_profile_index;
  int index = valueInt(GENERAL_SETTINGS "profile");
  return prof.indexOf(index);
}

void AppSettings::setCurrentProfile(int profile)
{
  if (profile > 0 && profile < m_available_profile_index.size())
    setValue(GENERAL_SETTINGS "profile", m_available_profile_index[profile]);
}

QList<int> AppSettings::availableProfilesIndex() const
{
  // uses universal profile indexes
  QList<int> prof;
  if (hasBackendValhalla()) prof.append(0);
  if (hasBackendMapnik() && hasBackendValhalla()) prof.append(1);
  if (hasBackendMapnik() && hasBackendValhalla()) prof.append(2);
  if (hasBackendOsmScout()) prof.append(3);
  if (hasBackendOsmScout()) prof.append(4);
  prof.append(5);

  return prof;
}

int AppSettings::defaultProfile() const
{
  // uses universal profile indexes
  const QList<int> &prof = m_available_profile_index;
  if (prof.contains(0)) return 0;
  if (prof.contains(1)) return 1;
  return 5; // custom
}

void AppSettings::setProfile()
{
  // uses universal profile indexes
  int index = valueInt(GENERAL_SETTINGS "profile");
  bool profile_active = true;
  const QList<int> &prof = m_available_profile_index;

  if (index == 0 && prof.contains(0)) // default profile: Mapbox GL / GeocoderNLP / Valhalla.
    {
      setValue(MAPMANAGER_SETTINGS "osmscout", 0);
      setValue(MAPMANAGER_SETTINGS "geocoder_nlp", 1);
      setValue(MAPMANAGER_SETTINGS "postal_country", 1);
      setValue(MAPMANAGER_SETTINGS "mapboxgl", 1);
      setValue(MAPMANAGER_SETTINGS "mapnik", 0);
      setValue(MAPMANAGER_SETTINGS "valhalla", 1);

      setValue(GEOMASTER_SETTINGS "use_geocoder_nlp", 1);
      setValue(MAPNIKMASTER_SETTINGS "use_mapnik", 0);
      setValue(VALHALLA_MASTER_SETTINGS "use_valhalla", 1);
    }
  else if (index == 1 && prof.contains(1)) // Mapnik / GeocoderNLP / Valhalla
    {
      setValue(MAPMANAGER_SETTINGS "osmscout", 0);
      setValue(MAPMANAGER_SETTINGS "geocoder_nlp", 1);
      setValue(MAPMANAGER_SETTINGS "postal_country", 1);
      setValue(MAPMANAGER_SETTINGS "mapboxgl", 0);
      setValue(MAPMANAGER_SETTINGS "mapnik", 1);
      setValue(MAPMANAGER_SETTINGS "valhalla", 1);

      setValue(GEOMASTER_SETTINGS "use_geocoder_nlp", 1);
      setValue(MAPNIKMASTER_SETTINGS "use_mapnik", 1);
      setValue(VALHALLA_MASTER_SETTINGS "use_valhalla", 1);
    }
  else if (index == 2 && prof.contains(2)) // Mapbox GL + Mapnik / GeocoderNLP / Valhalla
    {
      setValue(MAPMANAGER_SETTINGS "osmscout", 0);
      setValue(MAPMANAGER_SETTINGS "geocoder_nlp", 1);
      setValue(MAPMANAGER_SETTINGS "postal_country", 1);
      setValue(MAPMANAGER_SETTINGS "mapboxgl", 1);
      setValue(MAPMANAGER_SETTINGS "mapnik", 1);
      setValue(MAPMANAGER_SETTINGS "valhalla", 1);

      setValue(GEOMASTER_SETTINGS "use_geocoder_nlp", 1);
      setValue(MAPNIKMASTER_SETTINGS "use_mapnik", 1);
      setValue(VALHALLA_MASTER_SETTINGS "use_valhalla", 1);
    }
  else if (index == 3 && prof.contains(3)) // libosmscout + geocoder-nlp
    {
      setValue(MAPMANAGER_SETTINGS "osmscout", 1);
      setValue(MAPMANAGER_SETTINGS "geocoder_nlp", 1);
      setValue(MAPMANAGER_SETTINGS "postal_country", 1);
      setValue(MAPMANAGER_SETTINGS "mapboxgl", 0);
      setValue(MAPMANAGER_SETTINGS "mapnik", 0);
      setValue(MAPMANAGER_SETTINGS "valhalla", 0);

      setValue(GEOMASTER_SETTINGS "use_geocoder_nlp", 1);
      setValue(MAPNIKMASTER_SETTINGS "use_mapnik", 0);
      setValue(VALHALLA_MASTER_SETTINGS "use_valhalla", 0);
    }
  else if (index == 4 && prof.contains(4)) // libosmscout
    {
      setValue(MAPMANAGER_SETTINGS "osmscout", 1);
      setValue(MAPMANAGER_SETTINGS "geocoder_nlp", 0);
      setValue(MAPMANAGER_SETTINGS "postal_country", 0);
      setValue(MAPMANAGER_SETTINGS "mapboxgl", 0);
      setValue(MAPMANAGER_SETTINGS "mapnik", 0);
      setValue(MAPMANAGER_SETTINGS "valhalla", 0);

      setValue(GEOMASTER_SETTINGS "use_geocoder_nlp", 0);
      setValue(MAPNIKMASTER_SETTINGS "use_mapnik", 0);
      setValue(VALHALLA_MASTER_SETTINGS "use_valhalla", 0);
    }
  // all other profiles are either custom (index=5) or unknown
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


void AppSettings::checkCountrySelectionNeeded()
{
  bool old = m_country_selection_needed;

  if ( ( !valueBool(GEOMASTER_SETTINGS "search_all_maps") ) ||
       ( valueBool(MAPMANAGER_SETTINGS "osmscout") &&
         (!valueBool(GEOMASTER_SETTINGS "use_geocoder_nlp") ||
          !valueBool(MAPNIKMASTER_SETTINGS "use_mapnik") ||
          !valueBool(VALHALLA_MASTER_SETTINGS "use_valhalla") )
         ) )
    m_country_selection_needed = true;
  else
    m_country_selection_needed = false;

  if (old != m_country_selection_needed)
    emit countrySelectionNeededChanged(m_country_selection_needed);
}
