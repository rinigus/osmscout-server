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

#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#include <QDebug>


MapnikMaster::MapnikMaster(QObject *parent) :
  QObject(parent),
  m_projection_longlat("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"),
  m_projection_merc("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over"),
  m_projection_transform(m_projection_longlat, m_projection_merc)
{
  mapnik::datasource_cache::instance().register_datasources(MAPNIK_INPUT_PLUGINS_DIR);
  mapnik::freetype_engine::register_fonts(MAPNIK_FONTS_DIR);

  mapnik::logger::set_severity(mapnik::logger::error);

  m_ncpus = std::max(1, QThread::idealThreadCount());
#ifdef IS_SAILFISH_OS
  // In Sailfish, CPUs could be switched off one by one. As a result,
  // "ideal thread count" set by Qt could be off.
  // In other systems, this procedure is not needed and the defaults can be used
  //
  m_ncpus = 0;
  QDir dir;
  while ( dir.exists(QString("/sys/devices/system/cpu/cpu") + QString::number(m_ncpus)) )
    ++m_ncpus;
#endif

  onSettingsChanged();
}

MapnikMaster::~MapnikMaster()
{
}

void MapnikMaster::onSettingsChanged()
{
  std::unique_lock<std::mutex> lk(m_mutex);
  AppSettings settings;

  m_scale = std::max(1e-3, settings.valueFloat(MAPNIKMASTER_SETTINGS "scale"));
  m_buffer_size = std::max(0, settings.valueInt(MAPNIKMASTER_SETTINGS "buffer_size_in_pixels"));
  useMapnik = settings.valueBool(MAPNIKMASTER_SETTINGS "use_mapnik");
  m_configuration_dir = settings.valueString(MAPNIKMASTER_SETTINGS "configuration_dir");

  if (!useMapnik && m_pool_maps.size() > 0)
    {
      m_pool_maps.clear();
      m_pool_maps_generation++;
      m_available = false;
    }

  if (useMapnik)
    {
      if (m_old_config_style != m_configuration_dir || (useMapnik && !m_available) )
        {
          // prepare folder to keep mapnik configuration
          QString local_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
          QDir dir(local_path);
          if ( local_path.isEmpty() || !dir.mkpath(dir.absoluteFilePath(const_dir)) )
            {
              InfoHub::logWarning(tr("Cannot create configuration directory for Mapnik"));
              return;
            }

          // make symbolic links to global configuration
          dir.setPath(dir.absoluteFilePath(const_dir));
          QDir global_dir(m_configuration_dir);
          QDirIterator it(global_dir.absolutePath(), QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
          while (it.hasNext())
            {
              it.next();
              QString tgt = it.filePath();
              QString fname = it.fileName();
              if (fname == const_xml) continue; // new configuration XML will be generated

              QFile lnk(tgt);
              lnk.remove(dir.absoluteFilePath(fname));
              if (!lnk.link(dir.absoluteFilePath(fname)))
                {
                  InfoHub::logWarning(tr("Failed to create symbolic link to Mapnik configuration (%1)").
                                      arg(fname));
                  return;
                }
            }

          m_local_xml = dir.absoluteFilePath(const_xml);

          if ( !m_old_config_path_world.isEmpty() || !m_old_config_countries.isEmpty() )
            {
              reloadMapnik(m_old_config_path_world, m_old_config_countries, true);
            }
        }
    }

  m_pool_maps_cv.notify_all();
}

void MapnikMaster::onMapnikChanged(QString world_directory, QStringList country_dirs)
{
  std::unique_lock<std::mutex> lk(m_mutex);

  reloadMapnik(world_directory, country_dirs, false);
  m_old_config_path_world = world_directory;
  m_old_config_countries = country_dirs;

  m_pool_maps_cv.notify_all();
}

// NB! should be called with the locked mutex
void MapnikMaster::reloadMapnik(QString world_directory, QStringList country_dirs, bool config_changed)
{
  if (useMapnik)
    {
      // regenerate configuration only if it changed
      if ( config_changed ||
           world_directory != m_old_config_path_world ||
           country_dirs != m_old_config_countries )
        {
          QString world_directory_root;
          {
            QDir d(world_directory);
            d.cdUp();
            world_directory_root = d.absolutePath();
          }

          // generate new XML configuration
          QDomDocument doc;
          {
            QFile file(m_configuration_dir + "/" + const_xml);
            if (!file.open(QIODevice::ReadOnly) || !doc.setContent(&file))
              {
                InfoHub::logWarning(tr("Failed to load Mapnik configuration: %1").arg(file.fileName()));
                return;
              }
          }

          QDomNodeList lold = doc.elementsByTagName("Layer");
          int orig_layers_left = lold.size();
          for (int li=0; li < orig_layers_left; ++li)
            {
              QDomElement layer = lold.item(li).toElement();
              QDomNodeList plist = layer.firstChildElement("Datasource").elementsByTagName("Parameter");
              for (int pi=0; pi < plist.size(); ++pi)
                {
                  QDomElement e = plist.item(pi).toElement();

                  if (e.hasAttribute("name") && e.attribute("name") == "file")
                    {
                      // country-specific layer: remove the defined layer and make as many clones
                      // as many countries have been given in country_dirs
                      QString fname = e.text();
                      if (fname.indexOf("country")>=0 && fname.indexOf(".sqlite") > 0)
                        {
                          QFileInfo finfo(fname);
                          QString sqlname = finfo.fileName();
                          for (QString f: country_dirs)
                            {
                              while (e.hasChildNodes())
                                e.removeChild(e.firstChild());
                              QDomText txt = doc.createTextNode(f + "/" + sqlname);
                              e.appendChild(txt);

                              QDomElement element = layer.cloneNode().toElement();
                              layer.parentNode().appendChild(element);
                            }

                          layer.parentNode().removeChild(layer);

                          orig_layers_left--;
                          li--;
                          break;
                        }

                      else if (fname.indexOf("global")==0)
                        {
                          // world shapes: just prepend root_directory
                          while (e.hasChildNodes())
                            e.removeChild(e.firstChild());
                          QDomText txt = doc.createTextNode(world_directory_root + "/" + fname);
                          e.appendChild(txt);
                        }
                    }
                }
            }

          {
            QFile f(m_local_xml);
            if (!f.open(QIODevice::WriteOnly))
              {
                InfoHub::logWarning(tr("Cannot write Mapnik configuration file: %1").arg(f.fileName()));
                return;
              }
            f.write(doc.toByteArray());
          }

          // config is ready, keep the values for checking against new requests
          m_old_config_style = m_configuration_dir;

          for (auto s: country_dirs)
            InfoHub::logInfo(tr("Mapnik: adding %1").arg(s));
        }

      m_pool_maps.clear();
      m_pool_maps_generation++;
      m_available = false;
      if (!world_directory.isEmpty() || !country_dirs.isEmpty())
        {
          try {
            for (int i = 0; i < m_ncpus; ++i)
              {
                std::shared_ptr<mapnik::Map> map = std::make_shared<mapnik::Map>();
                mapnik::load_map(*map, m_local_xml.toStdString());
                m_pool_maps.push_back(map);
              }

            m_available = true;
          }
          catch ( std::exception const& ex )
          {
            InfoHub::logError("Mapnik exception: " + QString::fromStdString(ex.what()));
          }
        }
    }
  else if (!m_pool_maps.size()>0)
    {
      m_pool_maps.clear();
      m_pool_maps_generation++;
      m_available = false;
    }
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
        InfoHub::logError(tr("Mapnik: failed to transform coordinates"));
        return false;
      }

    while (m_available && m_pool_maps.empty())
      m_pool_maps_cv.wait(lk);

    if (!m_available || m_pool_maps.empty())
      {
        InfoHub::logWarning(tr("Mapnik not available, probably due to missing datasets"));
        return false;
      }

    map = m_pool_maps.front();
    m_pool_maps.pop_front();
    generation = m_pool_maps_generation;
  }

  try
  {
    map->set_height(height);
    map->set_width(width);
    map->set_buffer_size(m_buffer_size*m_scale);
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
