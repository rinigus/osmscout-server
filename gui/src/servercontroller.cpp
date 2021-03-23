#include "servercontroller.h"

#include "config-common.h"

#include <QDBusConnection>
#include <QDBusPendingReply>

#include <QNetworkRequest>
#include <QUrl>

#include <QSettings>

#include <QDebug>


ServerController::ServerController():
  m_dbus(DBUS_INTERFACE_ROOT, DBUS_PATH_ROOT, DBUS_SERVICE,
         QDBusConnection::sessionBus(), nullptr)
{
}

ServerController::~ServerController()
{
  if (m_process)
    {
      QProcess::ProcessState state = m_process->state();
      if (state == QProcess::Running || state == QProcess::Starting)
        {
          m_process->terminate();
          m_process->waitForFinished(30000);
        }
    }
}

void ServerController::activate()
{
  networkActivate();
}

void ServerController::setStatus(const QString &status)
{
  m_status = status;
  emit statusChanged();
  qDebug() << m_status;
}

void ServerController::connectToServer()
{
  dbusActivate(false); // just call method
}

void ServerController::networkActivate()
{
  setStatus(tr("Activating server via network"));

  QSettings settings;
  QString host = settings.value(HTTP_SERVER_SETTINGS "host").toString();
  QString port = settings.value(HTTP_SERVER_SETTINGS "port").toString();
  QUrl url = QStringLiteral("http://%1:%2/v1/activate").arg(host).arg(port);

  connect(&m_nam, &QNetworkAccessManager::finished,
          this, &ServerController::networkCallback);

  m_nam.get(QNetworkRequest(url));
}

void ServerController::networkCallback(QNetworkReply *reply)
{
  bool success = (reply->isFinished() && reply->error() == QNetworkReply::NoError);
  reply->deleteLater();

  if (success)
    {
      setStatus(tr("Server reached via network"));
      return;
    }

  // network activation failed, try dbus
  setStatus(tr("Failed to reach server via network"));
  dbusActivate(true);
}

void ServerController::dbusActivate(bool full_activation)
{
  QDBusPendingCall pcall = m_dbus.asyncCall(QLatin1String("Connect"));
  if (full_activation)
    {
      setStatus(tr("Activating server via DBus"));
      QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
      QObject::connect(watcher, &QDBusPendingCallWatcher::finished,
                       this, &ServerController::dbusCallback);
    }
}

void ServerController::dbusCallback(QDBusPendingCallWatcher *call)
{
  bool success = call->isValid();
  call->deleteLater();

  if (success)
    {
      setStatus(tr("Server reached via DBus"));
      return;
    }

  // dbus activation failed, try to launch
  setStatus(tr("Failed to reach server via DBus"));
  execute();
}

void ServerController::execute()
{
  setStatus(tr("Starting server"));

  m_process = new QProcess(this);
  connect(m_process,
          static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
          this, &ServerController::executeCallback);
  connect(m_process, &QProcess::errorOccurred,
          this, &ServerController::executeCallbackForError);

  m_process->start(SERVER_EXE, QStringList());
}

void ServerController::executeCallback(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
  setStatus(tr("Server stopped"));
}

void ServerController::executeCallbackForError(QProcess::ProcessError /*error*/)
{
  setStatus(tr("Error starting server<br>Used path: %1").arg(SERVER_EXE));
}
