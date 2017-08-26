#include "systemdservice.h"

#include <QProcess>
#include <QDebug>

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
