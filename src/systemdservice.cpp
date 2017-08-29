#include "systemdservice.h"
#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QProcess>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

#include <QDebug>

#include <fstream>

#define SCTL "systemctl"

SystemDService::SystemDService(QObject *parent) : QObject(parent)
{
  update();
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
      QString exe_path = QStandardPaths::findExecutable(APP_PREFIX "osmscout-server");
      if (!exe_path.isEmpty())
        {
          // create socket and service files at home directory
          QString socket =
              "[Unit]\n"
              "Description=OSM Scout Server Socket\n\n"
              "[Socket]\n"
              "ListenStream=127.0.0.1:" + QString::number(settings.valueInt(HTTP_SERVER_SETTINGS "port")) + "\n\n"
              "[Install]\n"
              "WantedBy=user-session.target\n";

          QString service =
              "[Unit]\n"
              "Description=OSM Scout Server\n\n"
              "[Service]\n"
              "ExecStart=/usr/bin/" APP_PREFIX "osmscout-server --console --systemd\n";

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
