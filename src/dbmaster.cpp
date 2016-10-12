#include "dbmaster.h"
#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QMutexLocker>

DBMaster::DBMaster()
{
    m_database = osmscout::DatabaseRef(new osmscout::Database(m_database_parameter));
    m_map_service = osmscout::MapServiceRef(new osmscout::MapService(m_database));

    if (m_database == nullptr)
    {
        InfoHub::logError("Cannot create database object");
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
    if ( !m_database->IsOpen() || m_map_dir != m_database->GetPath() )
    {
        if ( m_database->IsOpen() )
            m_database->Close();

        if (!m_database->Open(m_map_dir))
        {
            InfoHub::logError("Cannot open database: " + QString::fromStdString(m_map_dir));
            return;
        }

        // clear error state
        InfoHub::logInfo("Opened database " + QString::fromStdString(m_map_dir), true);
    }

    m_icons_dir = settings.valueString(OSM_SETTINGS "icons").toStdString();
    m_render_sea = ( settings.valueInt(OSM_SETTINGS "renderSea") > 0 );
    m_draw_background = ( settings.valueInt(OSM_SETTINGS "drawBackground") > 0 );
    m_font_size = settings.valueFloat(OSM_SETTINGS "fontSize");

    std::string style = settings.valueString(OSM_SETTINGS "style").toStdString();
    if (m_style_name != style)
    {
        m_style_name = style;
        loadStyle(m_daylight);
    }
}


void DBMaster::onSettingsChanged()
{
    loadSettings();
}

bool DBMaster::loadStyle(bool daylight)
{
    if ( m_error_flag ||
         !m_database->IsOpen() )
        return false;

    // check if its the same as before
    if ( m_style_config != nullptr &&
         daylight == m_daylight )
        return true; // nothing to do, all is loaded

    // something changed, have to reload style
    osmscout::TypeConfigRef typeConfig=m_database->GetTypeConfig();
    if (!typeConfig) return false;

    m_style_config = osmscout::StyleConfigRef(new osmscout::StyleConfig(typeConfig));

    if (m_style_config == nullptr)
    {
        InfoHub::logError("Cannot allocate Style config");
        return false;
    }

    m_style_config->AddFlag("daylight", daylight);

    if (!m_style_config->Load(m_style_name))
    {
        InfoHub::logError("Cannot open style: " + QString::fromStdString(m_style_name));
        return false;
    }

    m_daylight = daylight;

    return true;
}


