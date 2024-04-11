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

#include "systemdservice.h"
#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QProcess>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

#include <QDebug>

#include <fstream>

#define SCTL "systemctl"

SystemDService* SystemDService::s_instance = nullptr;

SystemDService::SystemDService(QObject *parent) : QObject(parent)
{
  update();
}

SystemDService* SystemDService::instance()
{
  if (!s_instance) s_instance = new SystemDService();
  return s_instance;
}

void SystemDService::update()
{
  QStringList args(m_extra); args << "is-enabled" << const_service_name + ".socket";
  QProcess proc;
  proc.start( SCTL, args );
  if ( proc.waitForStarted() && proc.waitForFinished() )
    {
      QString out( proc.readAll() );
      bool v = (out.contains("enabled") && !out.contains("disabled"));
      if ( m_enabled != v)
        {
          m_enabled = v;
          emit enabledChanged();
        }
    }
}

void SystemDService::setEnabled(bool e)
{  
  if (m_enabled == e) return;

  if (e)
    {
      AppSettings settings;
#ifdef IS_LOMIRI
      // this hard-coded path is to keep the path stable using the 'current' version symlink
      QString exe_path = "/opt/click.ubuntu.com/osmscout-server.jonnius/current/usr/bin/osmscout-server";
#else
      QString exe_path = QCoreApplication::applicationFilePath();
#endif

      if (exe_path.isEmpty()) exe_path = QStandardPaths::findExecutable(APP_NAME);
      if (!exe_path.isEmpty())
        {
          // create socket and service files at home directory
          QString socket =
              "[Unit]\n"
              "Description=OSM Scout Server Socket\n\n"
              "[Socket]\n"
              "ListenStream=127.0.0.1:" + QString::number(settings.valueInt(HTTP_SERVER_SETTINGS "port")) + "\n"
              "TriggerLimitIntervalSec=60s\nTriggerLimitBurst=1\n\n"
              "[Install]\n"
#ifdef IS_LOMIRI
              "WantedBy=graphical-session.target\n";
#else
              "WantedBy=user-session.target\n";
#endif

          QString service =
              "[Unit]\n"
              "Description=OSM Scout Server\n\n"
              "[Service]\n"
#ifdef IS_LOMIRI
              "WorkingDirectory=/opt/click.ubuntu.com/osmscout-server.jonnius/current/\n"
              "Environment=\"LD_LIBRARY_PATH=/opt/click.ubuntu.com/osmscout-server.jonnius/current/lib/aarch64-linux-gnu/\"\n"
#endif
              "ExecStart=" + exe_path + " --systemd --quiet\n";

          QDir dir;
          QString path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/systemd/user";
          if (dir.mkpath(path))
            {
              dir.setPath(path);

              QFile fout_socket(dir.absoluteFilePath(const_service_name + ".socket"));
              if (fout_socket.open(QIODevice::WriteOnly))
                fout_socket.write( socket.toLocal8Bit() );

              QFile fout_service(dir.absoluteFilePath(const_service_name + ".service"));
              if (fout_service.open(QIODevice::WriteOnly))
                fout_service.write( service.toLocal8Bit() );
            }
          else
            InfoHub::logWarning(tr("Cannot create or change to directory: %1").arg(path));

          // ensure that new systemd config is loaded
          QStringList args(m_extra);
          args << "daemon-reload";
          QProcess proc;
          proc.start( SCTL, args );
          proc.waitForStarted() && proc.waitForFinished();
        }
      else
        InfoHub::logWarning(tr("Cannot find OSM Scout Server in the PATH"));
    }

  QStringList args(m_extra);
  if ( e ) args << "enable";
  else args << "disable";
  args << const_service_name + ".socket";

  QProcess proc;
  proc.start( SCTL, args );
  proc.waitForStarted() && proc.waitForFinished();

  update();
}

void SystemDService::action_helper(const QString &action, const QString &ext)
{
  QStringList args(m_extra);
  args << action
       << const_service_name + ext;

  QProcess proc;
  proc.start( SCTL, args );
  proc.waitForStarted() && proc.waitForFinished();
}

void SystemDService::stop()
{
  action_helper("stop", ".socket");
  action_helper("stop", "");
}

void SystemDService::start()
{
  // no need to start service since it will be autostarted by the socket
  action_helper("start", ".socket");
}
