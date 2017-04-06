#include "mapnikmaster.h"
#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/image_util.hpp>

#include <string>


MapnikMaster::MapnikMaster(QObject *parent) :
  QObject(parent),
  m_projection_longlat("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"),
  m_projection_merc("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over"),
  m_projection_transform(m_projection_longlat, m_projection_merc)
{
  mapnik::datasource_cache::instance().register_datasources("/usr/local/lib/mapnik/input");
  mapnik::freetype_engine::register_fonts("/usr/local/lib/mapnik/fonts");

  onSettingsChanged();

#pragma message "Should be initialized by map manager with the main folder and list of files"
  onMapnikChanged(QStringList());
}

MapnikMaster::~MapnikMaster()
{
}

void MapnikMaster::onSettingsChanged()
{
  QMutexLocker lk(&m_mutex);
  AppSettings settings;

  m_scale = std::max(1e-3, settings.valueFloat(MAPNIKMASTER_SETTINGS "scale"));
  useMapnik = settings.valueBool(MAPNIKMASTER_SETTINGS "use_mapnik");

  if (!useMapnik && m_map.layer_count()>0)
    m_map.remove_all();
}

void MapnikMaster::onMapnikChanged(QStringList /*files*/)
{
  QMutexLocker lk(&m_mutex);

  m_map.remove_all();
  if (useMapnik)
    {
      try {
        mapnik::load_map(m_map, "map.xml");
      }
      catch ( std::exception const& ex )
      {
        InfoHub::logError("Mapnik exception: " + QString::fromStdString(ex.what()));
      }
    }
}

bool MapnikMaster::renderMap(bool /*daylight*/, int width, int height, double lat0, double lon0, double lat1, double lon1, QByteArray &result)
{
  QMutexLocker lk(&m_mutex);

  try
  {
    mapnik::box2d<double> box(lon0, lat0, lon1, lat1);

    if ( !m_projection_transform.forward(box) )
      {
        InfoHub::logError("Mapnik: failed to transform coordinates");
        return false;
      }

    m_map.set_height(height);
    m_map.set_width(width);

#pragma message "This has to be optimized somehow"
    m_map.set_buffer_size(width/2*m_scale);

    m_map.zoom_to_box(box);

    mapnik::image_rgba8 buf(m_map.width(),m_map.height());
    mapnik::agg_renderer<mapnik::image_rgba8> ren(m_map,buf,m_scale);
    ren.apply();

    result = QByteArray::fromStdString(mapnik::save_to_string(buf,"png"));

    return true;
  }
  catch ( std::exception const& ex )
  {
    InfoHub::logError("Mapnik exception: " + QString::fromStdString(ex.what()));
  }

  return false;
}
