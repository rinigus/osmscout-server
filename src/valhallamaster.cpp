#ifdef USE_VALHALLA

#include "valhallamaster.h"
#include "appsettings.h"
#include "infohub.h"
#include "config.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include <algorithm>

#include <QDebug>

ValhallaMaster::ValhallaMaster(QObject *parent) : QObject(parent)
{
#pragma message "THIS SHOULD BE REMOVED"
  onSettingsChanged();
  onValhallaChanged("../valhalla/valhalla_tiles", QStringList());
}

ValhallaMaster::~ValhallaMaster()
{
  if (m_process) m_process->deleteLater();
}


void ValhallaMaster::onSettingsChanged()
{
  std::unique_lock<std::mutex> lk(m_mutex);
  AppSettings settings;

  useValhalla = settings.valueBool(VALHALLA_MASTER_SETTINGS "use_valhalla");

  bool changed = false;

  int cache = std::max(0, settings.valueInt(VALHALLA_MASTER_SETTINGS "cache_in_mb"));
  changed |= (cache != m_cache);
  m_cache = cache;

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

      if (changed)
        {
          stop();
          generateConfig();
          start();
        }
    }
  else
    stop();
}

void ValhallaMaster::onValhallaChanged(QString valhalla_directory, QStringList countries)
{
  if (!useValhalla) return;

  std::unique_lock<std::mutex> lk(m_mutex);
  if ( valhalla_directory != m_dirname || m_countries != countries )
    {
      m_dirname = valhalla_directory;
      m_countries = countries;
      stop();
      generateConfig();
      start();
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

  QString cache= QString::number(m_cache*1024*1024);
  conf.replace(const_tag_cache, cache);
  conf.replace(const_tag_dirname, m_dirname);

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
  stop();

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

  qDebug() << arguments;
}

void ValhallaMaster::stop()
{
  if (m_process)
    {
      m_process->kill();
      m_process->deleteLater();
      m_process = nullptr;
      m_process_started = false;
    }
}

void ValhallaMaster::onProcessRead()
{
  if (!m_process) return;

  QString txt = m_process->readAllStandardOutput();
  QTextStream tin(&txt, QIODevice::ReadOnly);
  while (!tin.atEnd())
    InfoHub::logInfo("Valhalla: " + tin.readLine());
}

void ValhallaMaster::onProcessReadError()
{
  if (!m_process) return;

  QString txt = m_process->readAllStandardError();
  QTextStream tin(&txt, QIODevice::ReadOnly);
  while (!tin.atEnd())
    InfoHub::logError("Valhalla: " + tin.readLine());
}


void ValhallaMaster::onProcessStarted()
{
  m_process_started = true;
}

void ValhallaMaster::onProcessStopped(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
  if (exitCode != 0)
    {
      InfoHub::logWarning(tr("Valhalla exited with error: %1").arg(exitCode));
      return;
    }
}

void ValhallaMaster::onProcessStateChanged(QProcess::ProcessState state)
{
  if (!m_process) return;

  if ( !m_process_started && state == QProcess::NotRunning )
    {
      InfoHub::logWarning(tr("Could not start the Valhalla routing service: %1").arg(m_process->program()));
    }
}

#endif
