#ifdef USE_OSMSCOUT

#include "dbmaster.h"
#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QMutexLocker>
#include <QDebug>

DBMaster::DBMaster()
{
  m_database = osmscout::DatabaseRef(new osmscout::Database(m_database_parameter));
  m_map_service = osmscout::MapServiceRef(new osmscout::MapService(m_database));

  if (m_database == nullptr)
    {
      InfoHub::logError(tr("Cannot create database object"));
      InfoHub::setError(true);

      m_error_flag = true;

      return;
    }

  loadSettings();
}

DBMaster::~DBMaster()
{
}

void DBMaster::loadSettings()
{
  QMutexLocker lk(&m_mutex);
  AppSettings settings;

  m_map_dir = settings.valueString(OSM_SETTINGS "map").toStdString();
  if ( m_map_dir != m_database->GetPath() )
    {
      if ( m_database->IsOpen() )
        m_database->Close();

      // Database will be opened again on the first call
    }

  m_icons_dir = settings.valueString(OSM_SETTINGS "icons").toStdString();
  m_render_sea = ( settings.valueInt(OSM_SETTINGS "renderSea") > 0 );
  m_draw_background = ( settings.valueInt(OSM_SETTINGS "drawBackground") > 0 );
  m_font_size = settings.valueFloat(OSM_SETTINGS "fontSize");
  m_data_lookup_area = std::max(1.0, settings.valueFloat(OSM_SETTINGS "dataLookupArea"));
  m_tile_borders_zoom_cutoff = settings.valueFloat(OSM_SETTINGS "tileBordersZoomCutoff");
  m_routing_cost_distance = settings.valueFloat(OSM_SETTINGS "routingCostLimitDistance");
  m_routing_cost_factor = settings.valueFloat(OSM_SETTINGS "routingCostLimitFactor");

  m_style_name = settings.valueString(OSM_SETTINGS "style").toStdString();

  // load speed table
  m_routing_speeds.clear();
  QStringList keys = settings.allKeys().filter(ROUTING_SPEED_SETTINGS);
  for (const QString &k: keys)
    {
      QString n = k;
      n.remove(0, strlen(ROUTING_SPEED_SETTINGS));
      m_routing_speeds[n.toStdString()] = settings.valueFloat(k);
    }
}


void DBMaster::onSettingsChanged()
{
  loadSettings();
}

bool DBMaster::loadDatabase()
{
  if (m_database->IsOpen()) return true;

  bool opened = false;
  if (m_map_dir.length() > 0)
    opened = m_database->Open(m_map_dir);

  if (opened)
    InfoHub::logInfo(tr("Opened OSM Scout database: %1").arg(QString::fromStdString(m_map_dir)), true);
  else
    InfoHub::logError(tr("Cannot open OSM Scout database: %1").arg(QString::fromStdString(m_map_dir)));

  return opened;
}

bool DBMaster::loadStyle(bool daylight)
{
  if ( m_error_flag ||
       !loadDatabase() )
    return false;

  // check if its the same as before
  if ( m_style_config != nullptr &&
       m_style_name_loaded == m_style_name &&
       daylight == m_daylight )
    return true; // nothing to do, all is loaded

  // something changed, have to reload style
  osmscout::TypeConfigRef typeConfig=m_database->GetTypeConfig();
  if (!typeConfig) return false;

  m_style_config = osmscout::StyleConfigRef(new osmscout::StyleConfig(typeConfig));

  if (m_style_config == nullptr)
    {
      InfoHub::logError(tr("Cannot allocate Style config"));
      return false;
    }

  m_style_config->AddFlag("daylight", daylight);

  if (!m_style_config->Load(m_style_name))
    {
      InfoHub::logError(tr("Cannot open style") + ": " + QString::fromStdString(m_style_name));
      return false;
    }

  m_daylight = daylight;
  m_style_name_loaded = m_style_name;
  InfoHub::logInfo(tr("Loaded style") + ": " + QString::fromStdString(m_style_name));

  return true;
}

void DBMaster::onDatabaseChanged(QString /*directory*/)
{
  loadSettings();
}

#endif
