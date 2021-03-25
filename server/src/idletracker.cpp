/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "idletracker.h"

#include "appsettings.h"
#include "config.h"
#include "dbustracker.h"
#include "infohub.h"
#include "mapmanager.h"
#include "systemdservice.h"

#include <iostream>

IdleTracker::IdleTracker(QObject *parent): QObject(parent)
{
  onSettingsChanged();

  clock_gettime(CLOCK_BOOTTIME, &m_last_call);

  connect(&m_timer, &QTimer::timeout,
          this, &IdleTracker::checkIdle);

  // queued connections are used to handle them being emitted from other threads
  // and avoid emitting idle signal while other signals are processed
  connect(InfoHub::instance(), &InfoHub::activitySig,
          this, &IdleTracker::registerCall, Qt::QueuedConnection);
  connect(DBusTracker::instance(), &DBusTracker::serviceDisappeared,
          this, &IdleTracker::onServiceDisappeared, Qt::QueuedConnection);
  connect(MapManager::Manager::instance(), &MapManager::Manager::downloadingChanged,
          this, &IdleTracker::checkIdle);
  connect(SystemDService::instance(), &SystemDService::enabledChanged,
          this, &IdleTracker::checkIdle);
}

void IdleTracker::registerCall()
{
  clock_gettime(CLOCK_BOOTTIME, &m_last_call);
}

void IdleTracker::onSettingsChanged()
{
  AppSettings settings;
  m_idle_timeout = settings.valueInt(REQUEST_MAPPER_SETTINGS "idle_timeout");

  if (m_idle_timeout > 0)
    m_timer.start( std::max(1000, (int)m_idle_timeout*1000/10));
  else m_timer.stop();
}

void IdleTracker::onServiceDisappeared(QString /*service*/)
{
  checkIdle();
}

void IdleTracker::checkIdle()
{
  // keep running if there are dbus clients
  if (DBusTracker::instance()->numberOfServices() > 0) return;

  // no dbus clients anymore: stop if started by dbus or systemd that got
  // later disabled by user
  if (startedByDBus || (startedBySystemD && !SystemDService::instance()->enabled()))
    {
      std::cout << "Idle: no more DBus clients" << std::endl;
      emit idleTimeout();
      return;
    }

  // started by daemon mode or systemd: keep running if downloading
  if (MapManager::Manager::instance()->downloading()) return;

  // no idle timeout specified
  if (m_idle_timeout <= 0) return;

  struct timespec now;
  clock_gettime(CLOCK_BOOTTIME, &now);
  double dt = now.tv_sec - m_last_call.tv_sec;
  if (dt > m_idle_timeout)
    {
      std::cout << "Idle: timeout reached" << std::endl;
      emit idleTimeout();
    }
}
