#include "dbmaster.h"
#include "appsettings.h"
#include "config.h"

#include <osmscout/MapPainterQt.h>

#include <QMutexLocker>
#include <QPixmap>
#include <QPainter>
#include <QBuffer>

DBMaster::DBMaster()
{
    m_database = osmscout::DatabaseRef(new osmscout::Database(m_database_parameter));
    m_map_service = osmscout::MapServiceRef(new osmscout::MapService(m_database));

    if (m_database == nullptr)
    {
        std::cerr << "Cannot create database object" << std::endl;
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
    if (m_error_flag) return;

    QMutexLocker lk(&m_mutex_settings);
    AppSettings settings;

    std::string map = settings.valueString(OSM_SETTINGS "map").toStdString();
    if ( !m_database->IsOpen() || map != m_database->GetPath() )
    {
        if ( m_database->IsOpen() )
            m_database->Close();

        if (!m_database->Open(map))
        {
            std::cerr << "Cannot open database: "  << map << std::endl;
            return;
        }
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
        std::cerr << "Cannot allocate Style config" << std::endl;
        return false;
    }

    m_style_config->AddFlag("daylight", daylight);

    if (!m_style_config->Load(m_style_name))
    {
        std::cerr << "Cannot open style: " << m_style_name << std::endl;
        return false;
    }

    m_daylight = daylight;

    return true;
}


bool DBMaster::renderMap(bool daylight, double dpi, int zoom_level, int width, int height, double lat, double lon, QByteArray &result)
{
    if (m_error_flag) return false;

    // reading in settings - area protected by mutex
    m_mutex_settings.lock();

    bool renderSea = m_render_sea;
    bool drawBackground = m_draw_background;
    float fontSize = m_font_size;

    std::list<std::string> paths;
    paths.push_back(m_icons_dir);

    if ( !loadStyle(daylight) )
        return false;

    osmscout::MapPainterQt mapPainter(m_style_config);

    m_mutex_settings.unlock();
    // settings read in - mutex protected area ends

    osmscout::MercatorProjection  projection;
    osmscout::MapParameter        drawParameter;
    osmscout::AreaSearchParameter searchParameter;
    osmscout::MapData             data;

    osmscout::Magnification magnification;
    magnification.SetLevel(zoom_level);

    drawParameter.SetIconPaths(paths);
    //drawParameter.SetPatternPaths(paths);

    drawParameter.SetDebugData(false);
    drawParameter.SetDebugPerformance(false);

    drawParameter.SetFontSize(fontSize);

    // optimize process can reduce number of nodes before rendering
    // it helps for slow renderer backend, but it cost some cpu
    // it seems that it is better to disable it for mobile devices with slow cpu
    drawParameter.SetOptimizeWayNodes(osmscout::TransPolygon::none);
    drawParameter.SetOptimizeAreaNodes(osmscout::TransPolygon::none);

    drawParameter.SetRenderBackground(drawBackground || renderSea);
    drawParameter.SetRenderSeaLand(renderSea);

    // To get accurate label drawing at tile borders, we take into account labels
    // of other than the current tile, too.
    if (zoom_level >= 14)
        // but not for high zoom levels, it is too expensive
        drawParameter.SetDropNotVisiblePointLabels(true);
    else
        drawParameter.SetDropNotVisiblePointLabels(false);

    // https://github.com/Framstag/libosmscout/blob/master/Documentation/RenderTuning.txt
    //searchParameter.SetBreaker(dataLoadingBreaker);
    if (magnification.GetLevel() >= 15)
        searchParameter.SetMaximumAreaLevel(6);
    else
        searchParameter.SetMaximumAreaLevel(4);

    // multithreading is realized by performing calls to
    // this function from multiple threads
    searchParameter.SetUseMultithreading(false);
    searchParameter.SetUseLowZoomOptimization(true);

    projection.Set(osmscout::GeoCoord(lat,lon),
                   0,
                   magnification,
                   dpi,
                   width,
                   height);

    projection.SetLinearInterpolationUsage(zoom_level >= 10);

    {
        QMutexLocker lk(&m_mutex_database);

        std::list<osmscout::TileRef> tiles;

        m_map_service->LookupTiles(projection,tiles);
        m_map_service->LoadMissingTileData(searchParameter,*m_style_config,tiles);
        m_map_service->ConvertTilesToMapData(tiles,data);

        if (drawParameter.GetRenderSeaLand())
            m_map_service->GetGroundTiles(projection, data.groundTiles);
    }

    QPixmap *pixmap=new QPixmap(width,height);
    if (pixmap == NULL)
        return false;

    QPainter* painter=new QPainter(pixmap);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    bool success = false;
    if (painter != NULL &&
            mapPainter.DrawMap(projection,
                               drawParameter,
                               data,
                               painter))
    {
        QBuffer buffer(&result);
        buffer.open(QIODevice::WriteOnly);
        pixmap->save(&buffer, "PNG");
        success = true;
    }

    delete painter;
    delete pixmap;

    return success;
}
