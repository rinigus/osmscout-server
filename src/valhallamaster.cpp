#ifdef USE_VALHALLA

#include "valhallamaster.h"
#include "appsettings.h"
#include "infohub.h"
#include "config.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QUrl>

#include <algorithm>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <valhalla/midgard/logging.h>

#include <QDebug>


// Logger class to send messages to InfoHub
class ValhallaLogger: public valhalla::midgard::logging::Logger
{
public:
  ValhallaLogger(const valhalla::midgard::logging::LoggingConfig& config): valhalla::midgard::logging::Logger(config)
  {}

  virtual void Log(const std::string &message, const valhalla::midgard::logging::LogLevel level)
  {
    // filter messages before logging
    if (message.find("Tile extract could not be loaded") != std::string::npos)
      return;

    QString m = "Valhalla: " + QString::fromStdString(message);
    if (level == valhalla::midgard::logging::LogLevel::ERROR)
      InfoHub::logError(m);
    else if (level == valhalla::midgard::logging::LogLevel::WARN)
      InfoHub::logWarning(m);
//    else if (level == valhalla::midgard::logging::LogLevel::INFO)
//      InfoHub::logInfo(m);
//    else
//      qDebug() << "skipped in production: " << m;
  }

  virtual void Log(const std::string &message, const std::string& custom_directive)
  {
    // filter messages before logging
    if (custom_directive.empty() ||
        custom_directive.find("[ANALYTICS]") != std::string::npos)
      return;

    InfoHub::logInfo( "Valhalla: " + QString::fromStdString(custom_directive + ": " + message) );
  }
};

ValhallaMaster::ValhallaMaster(QObject *parent) : QObject(parent)
{
  // register Valhalla logger
  valhalla::midgard::logging::RegisterLogger("osmscout",
                                             [](const valhalla::midgard::logging::LoggingConfig& config) {
      valhalla::midgard::logging::Logger* l = new ValhallaLogger(config);
      return l;
    });

  valhalla::midgard::logging::Configure({ {"type", "osmscout"}, {"color", ""} });

  onSettingsChanged();
}

ValhallaMaster::~ValhallaMaster()
{
}

void ValhallaMaster::onSettingsChanged()
{
  std::unique_lock<std::mutex> lk(m_mutex);
  AppSettings settings;

  useValhalla = settings.valueBool(VALHALLA_MASTER_SETTINGS "use_valhalla");

  bool changed = false;

  {
    int cache = std::max(0, settings.valueInt(VALHALLA_MASTER_SETTINGS "cache_in_mb"));
    changed |= (cache != m_cache);
    m_cache = cache;

    double tol = 1e-3;

    double limit_max_distance_auto = std::max(10, settings.valueInt(VALHALLA_MASTER_SETTINGS "limit_max_distance_auto"));
    changed |= (fabs(m_limit_max_distance_auto-limit_max_distance_auto) > tol);
    m_limit_max_distance_auto = limit_max_distance_auto;

    double limit_max_distance_bicycle = std::max(10, settings.valueInt(VALHALLA_MASTER_SETTINGS "limit_max_distance_bicycle"));
    changed |= (fabs(m_limit_max_distance_bicycle-limit_max_distance_bicycle) > tol);
    m_limit_max_distance_bicycle = limit_max_distance_bicycle;

    double limit_max_distance_pedestrian = std::max(10, settings.valueInt(VALHALLA_MASTER_SETTINGS "limit_max_distance_pedestrian"));
    changed |= (fabs(m_limit_max_distance_pedestrian-limit_max_distance_pedestrian) > tol);
    m_limit_max_distance_pedestrian = limit_max_distance_pedestrian;
  }

  if (useValhalla)
    {
      if (!m_dirname.isEmpty() && (changed || !m_actor))
        {
          generateConfig();
          if (!m_idle_mode) start();
        }
    }
  else
    stop();
}

void ValhallaMaster::onValhallaChanged(QString valhalla_directory, QStringList countries)
{
  std::unique_lock<std::mutex> lk(m_mutex);
  if ( valhalla_directory != m_dirname || m_countries != countries || !m_actor )
    {
      m_dirname = valhalla_directory;
      m_countries = countries;

      if (useValhalla)
        {
          generateConfig();
          if (!m_idle_mode) start();
        }
    }
}

void ValhallaMaster::generateConfig()
{
  QFile fin(VALHALLA_CONFIG_TEMPLATE);
  if (!fin.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      InfoHub::logWarning(tr("Error opening Valhalla's configuration template %1").arg(VALHALLA_CONFIG_TEMPLATE));
      return;
    }

  QTextStream tin(&fin);
  QString conf = tin.readAll();
  if (tin.status() != QTextStream::Ok)
    {
      InfoHub::logWarning(tr("Error reading Valhalla's configuration template"));
      return;
    }

  QString cache= QString::number(m_cache*1024L*1024L);
  conf.replace(const_tag_cache, cache);
  conf.replace(const_tag_dirname, m_dirname);
  conf.replace(const_tag_limit_max_distance_auto, QString::number(m_limit_max_distance_auto*1e3));
  conf.replace(const_tag_limit_max_distance_bicycle, QString::number(m_limit_max_distance_bicycle*1e3));
  conf.replace(const_tag_limit_max_distance_pedestrian, QString::number(m_limit_max_distance_pedestrian*1e3));

  m_config_json = conf.toStdString();
}

/// interaction with valhalla route service process
void ValhallaMaster::start()
{
  m_idle_mode = false;

  std::stringstream ss;
  ss << m_config_json;
  boost::property_tree::ptree pt;
  boost::property_tree::read_json(ss, pt);

  m_actor.release();
  m_actor.reset(new valhalla::tyr::actor_t(pt, true)); // with autoclean
  InfoHub::logInfo(tr("Valhalla routing engine started"));
}

void ValhallaMaster::stop()
{
  m_actor.release();
}

///////////////////////////////////////////////////////////////////////////////
/// Interaction with Valhalla service

bool ValhallaMaster::callActor(ActorType atype, const QByteArray &json, QByteArray &result)
{
  std::unique_lock<std::mutex> lk(m_mutex);
  if (!m_actor) return false;

  bool success = true;
  try {
    std::string s = json.toStdString();
    std::string r;
    if (atype == Height) r = m_actor->height(s);
    else if (atype == Isochrone) r = m_actor->isochrone(s);
    else if (atype == Locate) r = m_actor->locate(s);
    else if (atype == Matrix) r = m_actor->matrix(s);
    else if (atype == OptimizedRoute) r = m_actor->optimized_route(s);
    else if (atype == Route) r = m_actor->route(s);
    else if (atype == TraceAttributes) r = m_actor->trace_attributes(s);
    else if (atype == TraceRoute) r = m_actor->trace_route(s);
    result = QByteArray::fromStdString(r);
  }
  catch (std::exception &e) {
    InfoHub::logWarning(tr("Exception in Valhalla: %1").arg(QString::fromStdString(e.what())));
    success = false;
    m_actor->cleanup();
  }

  return success;
}


#endif
