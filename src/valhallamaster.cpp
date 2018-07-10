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

#include <QDebug>

ValhallaMaster::ValhallaMaster(QObject *parent) : QObject(parent)
{
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
          start();
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
          start();
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
  std::stringstream ss;
  ss << m_config_json;
  boost::property_tree::ptree pt;
  boost::property_tree::read_json(ss, pt);

  m_actor.release();
  m_actor.reset(new valhalla::tyr::actor_t(pt, true)); // with autoclean
  InfoHub::logInfo(tr("Valhalla routing engine started"));
}

// List of blacklisted sub-strings that will not be
// displayed to the user
const static QStringList ignore_messages{
  "[INFO] ",
  "[ANALYTICS] ",
  "elapsed time ",
  "exceeded threshold::",
  " GET /route?json=",
  " 200 ",
  "Tile extract could not be loaded"
};

void ValhallaMaster::stop()
{
  m_actor.release();
}

///////////////////////////////////////////////////////////////////////////////
/// Interaction with Valhalla service

bool ValhallaMaster::callActor(ActorType atype, const QString &json, QByteArray &result)
{
  std::unique_lock<std::mutex> lk(m_mutex);
  if (!m_actor) return false;

  bool success = true;
  try {
    std::string s = json.toStdString();
    std::string r;
    if (atype == Route) r = m_actor->route(s);
    else if (atype == TraceAttributes) r = m_actor->trace_attributes(s);
    result = QByteArray::fromStdString(r);
  }
  catch (std::exception &e) {
    InfoHub::logWarning("Exception in Valhalla routing: " + QString::fromStdString(e.what()));
    success = false;
    m_actor->cleanup();
  }

  return success;
}

bool ValhallaMaster::route(const QString &json, QByteArray &result)
{
  return callActor(Route, json, result);
}

bool ValhallaMaster::trace_attributes(const QString &json, QByteArray &result)
{
  return callActor(TraceAttributes, json, result);
}
#endif
