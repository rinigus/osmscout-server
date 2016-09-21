#include "dbmaster.h"
#include "appsettings.h"

#include <osmscout/MapPainterQt.h>

#include <QMutexLocker>
#include <QPixmap>
#include <QPainter>
#include <QBuffer>

#ifdef IS_SAILFISH_OS
#define DATA_PREFIX "/usr/share/osmscout-server/"
#endif

#ifdef IS_CONSOLE_QT
#define DATA_PREFIX ""
#endif


DBMaster::DBMaster()
{
    AppSettings settings;

    std::string map = settings.value(OSM_SETTINGS "map", "map").toString().toStdString();
    std::string style  = settings.value(OSM_SETTINGS "style", DATA_PREFIX "stylesheets/standard.oss").toString().toStdString();

    database = osmscout::DatabaseRef(new osmscout::Database(databaseParameter));
    mapService = osmscout::MapServiceRef(new osmscout::MapService(database));

    if (database == NULL || !database->Open(map))
    {
        std::cerr << "Cannot open database: "  << map << std::endl;
        m_error_flag = true;
    }

    styleConfig = osmscout::StyleConfigRef(new osmscout::StyleConfig(database->GetTypeConfig()));

    if (!styleConfig->Load(style))
    {
        std::cerr << "Cannot open style" << std::endl;
    }

    m_icons_dir = settings.value(OSM_SETTINGS "style", DATA_PREFIX "data/icons/14x14/standard/").toString().toStdString();
}


bool DBMaster::renderMap(double dpi, int zoom_level, int width, int height, double lat, double lon, QByteArray &result)
{
    osmscout::MercatorProjection  projection;
    osmscout::MapParameter        drawParameter;
    osmscout::AreaSearchParameter searchParameter;
    osmscout::MapData             data;
    osmscout::MapPainterQt        mapPainter(styleConfig);

    const bool renderSea = true;
    const bool drawBackground = true;

    osmscout::Magnification magnification;
    magnification.SetLevel(zoom_level);

    std::list<std::string> paths;
    paths.push_back(m_icons_dir);

    drawParameter.SetIconPaths(paths);
    drawParameter.SetDebugData(false);
    drawParameter.SetDebugPerformance(true);

    drawParameter.SetIconPaths(paths);
    //            drawParameter.SetPatternPaths(paths);
    //            drawParameter.SetDebugData(false);
    //            drawParameter.SetDebugPerformance(true);

    drawParameter.SetFontSize(4.0);

    // optimize process can reduce number of nodes before rendering
    // it helps for slow renderer backend, but it cost some cpu
    // it seems that it is better to disable it for mobile devices with slow cpu
    drawParameter.SetOptimizeWayNodes(osmscout::TransPolygon::none);
    drawParameter.SetOptimizeAreaNodes(osmscout::TransPolygon::none);

    drawParameter.SetRenderBackground(drawBackground || renderSea);
    drawParameter.SetRenderSeaLand(renderSea);

    // To get accurate label drawing at tile borders, we take into account labels
    // of other than the current tile, too.
    if (zoom_level >= 14) {
        // but not for high zoom levels, it is too expensive
        drawParameter.SetDropNotVisiblePointLabels(true);
    }else{
        drawParameter.SetDropNotVisiblePointLabels(false);
    }

    // https://github.com/Framstag/libosmscout/blob/master/Documentation/RenderTuning.txt
    //searchParameter.SetBreaker(dataLoadingBreaker);
    if (magnification.GetLevel() >= 15) {
      searchParameter.SetMaximumAreaLevel(6);
    }
    else {
      searchParameter.SetMaximumAreaLevel(4);
    }
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
        QMutexLocker lk(&m_mutex);

        std::list<osmscout::TileRef> tiles;

        mapService->LookupTiles(projection,tiles);
        mapService->LoadMissingTileData(searchParameter,*styleConfig,tiles);
        mapService->ConvertTilesToMapData(tiles,data);

        if (drawParameter.GetRenderSeaLand())
          mapService->GetGroundTiles(projection, data.groundTiles);
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
