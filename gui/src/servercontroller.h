#ifndef SERVERCONTROLLER_H
#define SERVERCONTROLLER_H

#include <QObject>

#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>

class ServerController: public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
  ServerController();
  virtual ~ServerController();

  Q_INVOKABLE void activate();

  QString status() const { return m_status; }

public slots:
  void onServiceAppeared();

signals:
  void statusChanged();

private:
  void networkActivate();
  void networkCallback(QNetworkReply *reply);

  void dbusActivate(bool full_activation);
  void dbusCallback(QDBusPendingCallWatcher *call);

  void execute();
  void executeCallback(int exitCode, QProcess::ExitStatus exitStatus);
  void executeCallbackForError(QProcess::ProcessError error);

  void setStatus(const QString &status);

private:
  QNetworkAccessManager m_nam;
  QDBusInterface m_dbus;
  QString m_status;
  QProcess *m_process{nullptr};
};

#endif // SERVERCONTROLLER_H
