#include "mapnikmaster.h"
#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/well_known_srs.hpp>
#include <mapnik/debug.hpp>

#include <string>
#include <algorithm>

#include <QThread>
#include <QDir>

MapnikMaster::MapnikMaster(QObject *parent) :
  QObject(parent),
  m_projection_longlat("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"),
  m_projection_merc("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over"),
  m_projection_transform(m_projection_longlat, m_projection_merc)
{
  mapnik::datasource_cache::instance().register_datasources(MAPNIK_INPUT_PLUGINS_DIR);
  mapnik::freetype_engine::register_fonts(MAPNIK_FONTS_DIR);

  mapnik::logger::set_severity(mapnik::logger::error);

  onSettingsChanged();

#pragma message "Should be initialized by map manager with the main folder and list of files"
  onMapnikChanged(QStringList());
}

MapnikMaster::~MapnikMaster()
{
}

void MapnikMaster::onSettingsChanged()
{
  std::unique_lock<std::mutex> lk(m_mutex);
  AppSettings settings;

  m_scale = std::max(1e-3, settings.valueFloat(MAPNIKMASTER_SETTINGS "scale"));
  useMapnik = settings.valueBool(MAPNIKMASTER_SETTINGS "use_mapnik");

  if (!useMapnik && m_pool_maps.size() > 0)
    {
      m_pool_maps.clear();
      m_pool_maps_generation++;
    }

  m_pool_maps_cv.notify_all();
}

void MapnikMaster::onMapnikChanged(QStringList /*files*/)
{
  std::unique_lock<std::mutex> lk(m_mutex);

  m_pool_maps.clear();
  m_pool_maps_generation++;

  if (useMapnik)
    {
      try {
        int ncpus = std::max(1, QThread::idealThreadCount());
#ifdef IS_SAILFISH_OS
        // In Sailfish, CPUs could be switched off one by one. As a result,
        // "ideal thread count" set by Qt could be off.
        // In other systems, this procedure is not needed and the defaults can be used
        //
        ncpus = 0;
        QDir dir;
        while ( dir.exists(QString("/sys/devices/system/cpu/cpu") + QString::number(ncpus)) )
          ++ncpus;
#endif
        for (int i = 0; i < ncpus; ++i)
          {
            std::shared_ptr<mapnik::Map> map(new mapnik::Map());
            mapnik::load_map(*map, "Mapnik/map.xml");
            m_pool_maps.push_back(map);
          }
      }
      catch ( std::exception const& ex )
      {
        InfoHub::logError("Mapnik exception: " + QString::fromStdString(ex.what()));
      }
    }

  m_pool_maps_cv.notify_all();
}

bool MapnikMaster::renderMap(bool /*daylight*/, int width, int height, double lat0, double lon0, double lat1, double lon1, QByteArray &result)
{
  bool success = false;

  std::shared_ptr<mapnik::Map> map;
  int generation = -1;

  mapnik::box2d<double> box(lon0, lat0, lon1, lat1);

  {
    std::unique_lock<std::mutex> lk(m_mutex);

    if ( !m_projection_transform.forward(box) )
      {
        InfoHub::logError("Mapnik: failed to transform coordinates");
        return false;
      }

    while (useMapnik && m_pool_maps.empty())
      m_pool_maps_cv.wait(lk);

    if (!useMapnik || m_pool_maps.empty()) return false;

    map = m_pool_maps.front();
    m_pool_maps.pop_front();
    generation = m_pool_maps_generation;
  }

  try
  {
    map->set_height(height);
    map->set_width(width);

#pragma message "This has to be optimized somehow"
    map->set_buffer_size(256/2*m_scale);

    map->zoom_to_box(box);

    mapnik::image_rgba8 buf(map->width(),map->height());
    mapnik::agg_renderer<mapnik::image_rgba8> ren(*map,buf,m_scale);
    ren.apply();

    std::string res = mapnik::save_to_string(buf,"png");
    result.append(res.data(), res.size());

    success = true;
  }
  catch ( std::exception const& ex )
  {
    InfoHub::logError("Mapnik exception: " + QString::fromStdString(ex.what()));
  }

  {
    std::unique_lock<std::mutex> lk(m_mutex);
    if (generation == m_pool_maps_generation)
      m_pool_maps.push_front(map);
  }

  return success;
}
