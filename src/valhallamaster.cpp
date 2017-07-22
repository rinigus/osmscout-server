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
#include <curl/curl.h>

#include <QDebug>

ValhallaMaster::ValhallaMaster(QObject *parent) : QObject(parent)
{
  onSettingsChanged();
}

ValhallaMaster::~ValhallaMaster()
{
  if (m_process)
    {
      stop();
      m_process->deleteLater();
    }
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

    int route_port = settings.valueInt(VALHALLA_MASTER_SETTINGS "route_port");
    changed |= (route_port != m_route_port);
    m_route_port = route_port;

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

  m_valhalla_route_url = QString("http://127.0.0.1:%1").arg(m_route_port);

  if (useValhalla)
    {
      QString local_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
      QDir dir(local_path);
      if ( local_path.isEmpty() || !dir.mkpath(dir.absoluteFilePath(const_dir)) )
        {
          InfoHub::logWarning(tr("Cannot create configuration directory for Valhalla"));
          return;
        }

      dir.setPath(dir.absoluteFilePath(const_dir));
      m_config_fname = dir.absoluteFilePath(const_conf);

      if (!m_dirname.isEmpty() && (changed || !m_process))
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
  if ( valhalla_directory != m_dirname || m_countries != countries || !m_process )
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
  conf.replace(const_tag_route_port, QString::number(m_route_port));
  conf.replace(const_tag_limit_max_distance_auto, QString::number(m_limit_max_distance_auto*1e3));
  conf.replace(const_tag_limit_max_distance_bicycle, QString::number(m_limit_max_distance_bicycle*1e3));
  conf.replace(const_tag_limit_max_distance_pedestrian, QString::number(m_limit_max_distance_pedestrian*1e3));

  QFile fout(m_config_fname);
  if (!fout.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
      InfoHub::logWarning(tr("Error opening Valhalla's configuration file %1").arg(m_config_fname));
      return;
    }

  QTextStream tout(&fout);
  tout << conf;
  if (tout.status() != QTextStream::Ok)
    {
      InfoHub::logWarning(tr("Error writing Valhalla's configuration file"));
      return;
    }
}

/// interaction with valhalla route service process
void ValhallaMaster::start()
{
  if ( m_process )
    {
      // have to stop the running process first - otherwise may have issues with the ports
      m_process_start_when_ready = true;
      stop();
      return;
    }

  start_process();
}

void ValhallaMaster::start_process()
{
  InfoHub::logInfo(tr("Starting Valhalla routing engine"));

  m_process_start_when_ready = false;

  m_process = new QProcess(this);
  connect( m_process, &QProcess::started,
           this, &ValhallaMaster::onProcessStarted );

  connect( m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
           this, &ValhallaMaster::onProcessStopped );

  connect( m_process, &QProcess::stateChanged,
           this, &ValhallaMaster::onProcessStateChanged );

  connect( m_process, &QProcess::readyReadStandardOutput,
           this, &ValhallaMaster::onProcessRead );

  connect( m_process, &QProcess::readyReadStandardError,
           this, &ValhallaMaster::onProcessReadError );

  QStringList arguments;
  arguments << m_config_fname << "1";
  m_process->start(VALHALLA_EXECUTABLE, arguments);
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

void ValhallaMaster::onProcessRead()
{
  if (!m_process) return;

  QString txt = m_process->readAllStandardOutput();
  QTextStream tin(&txt, QIODevice::ReadOnly);
  while (!tin.atEnd())
    {
      const QString l = tin.readLine();
      bool show_message = true;
      for (const QString &i: ignore_messages)
        if (l.contains(i))
          {
            show_message = false;
            break;
          }

      if (show_message)
        InfoHub::logInfo("Valhalla: " + l);
    }
}

void ValhallaMaster::onProcessReadError()
{
  if (!m_process) return;

  QString txt = m_process->readAllStandardError();
  QTextStream tin(&txt, QIODevice::ReadOnly);
  while (!tin.atEnd())
    InfoHub::logWarning("Valhalla: " + tin.readLine());
}


void ValhallaMaster::onProcessStarted()
{
  m_process_ready = true;
  InfoHub::logInfo(tr("Valhalla routing engine started"));
}

void ValhallaMaster::onProcessStopped(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
  if (!m_process) return;

  if (exitCode != 0 && !m_process_killed)
    InfoHub::logWarning(tr("Valhalla exited with error: %1").arg(exitCode));

  if (m_process)
    {
      InfoHub::logInfo(tr("Valhalla routing engine stopped"));

      m_process_ready = false;
      m_process_killed = false;
      m_process->disconnect();
      m_process->deleteLater();
      m_process = nullptr;

      if (m_process_start_when_ready)
        start_process();
    }
}

void ValhallaMaster::stop()
{
  if (m_process)
    {
      InfoHub::logInfo(tr("Stopping Valhalla routing engine"));

      m_process_ready = false;
      if ( m_process->state() == QProcess::Running )
        {
          m_process_killed = true;
          m_process->kill();
        }
      else
        {
          InfoHub::logInfo(tr("Valhalla routing engine process cleanup"));

          // cleanup and get ready to start a new process if needed
          m_process->disconnect();
          m_process->kill();
          m_process->deleteLater();
          m_process = nullptr;

          if (m_process_start_when_ready)
            start_process();
        }
    }
}

void ValhallaMaster::onProcessStateChanged(QProcess::ProcessState state)
{
  if (!m_process || m_process_killed) return;

  if ( !m_process_ready && state == QProcess::NotRunning )
    {
      InfoHub::logWarning(tr("Could not start the Valhalla routing service: %1").arg(m_process->program()));

      m_process->deleteLater();
      m_process = nullptr;
      m_process_ready = false;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Interaction with Valhalla service

static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
  QByteArray *data = (QByteArray*)userp;
  data->append((char*)buffer, size*nmemb);
  return size*nmemb;
}

bool ValhallaMaster::route(QString uri, QByteArray &result)
{
  if (!m_process_ready) return false;

  QUrl url(m_valhalla_route_url + "/route?" + uri);
  bool success = true;

  CURL *curl = curl_easy_init();
  if (curl == NULL)
    {
      InfoHub::logWarning("Error initializing libCURL easy init"); // technical message, no need to translate
      return false;
    }

  curl_easy_setopt(curl, CURLOPT_URL, url.toEncoded().data());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK)
    {
      InfoHub::logWarning(tr("Error while communicating with Valhalla routing engine"));
      success = false;
    }

  curl_easy_cleanup(curl);

  if (success)
    InfoHub::logInfo(tr("Route found by Valhalla"));

  return success;
}

#endif
