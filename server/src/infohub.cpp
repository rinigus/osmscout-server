/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
 *
 * This file is part of OSM Scout Server.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "infohub.h"
#include "config.h"
#include "appsettings.h"

#include <QMutexLocker>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>

#include <QDebug>

#include <iostream>

#ifdef USE_VALHALLA
#include <valhalla/valhalla.h>
#endif

InfoHub* InfoHub::s_instance = nullptr;

InfoHub::InfoHub(QObject *parent) : QObject(parent)
{
  connect( this, &InfoHub::log, this, &InfoHub::sessionLog );
}

InfoHub* InfoHub::instance()
{
  if (!s_instance) s_instance = new InfoHub();
  return s_instance;
}

void InfoHub::onSettingsChanged()
{
  AppSettings settings;

  m_log_info = (settings.valueInt(GENERAL_SETTINGS "logInfo") > 0);
  m_log_session = (settings.valueInt(GENERAL_SETTINGS "logSession") > 0);
}

void InfoHub::setError(bool e)
{
  InfoHub::instance()->impSetError(e);
}

void InfoHub::impSetError(bool e)
{
  bool ce;
  {
    QMutexLocker lk(&m_mutex);
    ce = m_error;
    m_error =  e;
  }
  if (ce != e)
    emit errorChanged(e);
}

bool InfoHub::error()
{
  QMutexLocker lk(&m_mutex);
  return m_error;
}

void InfoHub::addJobToQueue()
{
  InfoHub::instance()->implChangeQueue(+1);
}

void InfoHub::removeJobFromQueue()
{
  InfoHub::instance()->implChangeQueue(-1);
}

void InfoHub::implChangeQueue(int delta)
{
  {
    QMutexLocker lk(&m_mutex);
    m_queue += delta;
  }
  emit queueChanged(m_queue);
}

int InfoHub::queue()
{
  QMutexLocker lk(&m_mutex);
  return m_queue;
}

void InfoHub::activity()
{
  InfoHub::instance()->implActivity();
}

void InfoHub::implActivity()
{
  emit InfoHub::instance()->activitySig();
}

static QString tstamp(const QString &txt)
{
  return QDateTime::currentDateTime().toString("HH:mm:ss") + " " + txt;
}

void InfoHub::logError(const QString &txt)
{
  InfoHub::instance()->impLogError(tstamp(txt));
}

void InfoHub::logWarning(const QString &txt)
{
  InfoHub::instance()->impLogWarning(tstamp(txt));
}

void InfoHub::logInfo(const QString &txt, bool force)
{
  InfoHub::instance()->impLogInfo(tstamp(txt), force);
}

void InfoHub::impLogError(const QString &txt)
{
  emit error(txt);
  emit log(tr("ERROR: ") + txt);
}

void InfoHub::impLogWarning(const QString &txt)
{
  emit warning(txt);
  emit log(tr("WARNING: ") + txt);
}

void InfoHub::impLogInfo(const QString &txt, bool force)
{
  if (!force && !m_log_info && !m_log_session)
    return;

  emit info(txt);
  emit log(tr("INFO: ") + txt);
}

void InfoHub::sessionLog(const QString &txt)
{
  if (!m_log_session)
    return;

  if ( !m_log_file.isOpen() )
    {
      QDir dir;
      QString dirpath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
      if (!dir.mkpath(dirpath))
        {
          std::cerr << "Cannot make directory " << dirpath.toStdString() << std::endl;
          return;
        }

      m_log_file.setFileName(dirpath + "/session.log");
      if (!m_log_file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
          std::cerr << "Error opening file " << m_log_file.fileName().toStdString() << std::endl;
          return;
        }
    }

  QTextStream out(&m_log_file);
  out << txt << "\n";
}

QString InfoHub::version()
{
  return APP_VERSION;
}

QString InfoHub::versionLibs()
{
  QString version;
  QTextStream s(&version);
#ifdef USE_VALHALLA
  s << QStringLiteral("Valhalla: ")
    << VALHALLA_VERSION_MAJOR << "."
    << VALHALLA_VERSION_MINOR << "."
    << VALHALLA_VERSION_PATCH << "\n";
#endif

  return version.trimmed();
}
