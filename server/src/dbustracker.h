#ifndef DBUSTRACKER_H
#define DBUSTRACKER_H

#include <QMutex>
#include <QObject>
#include <QSet>
#include <QString>

/// Registers client services and tracks them
class DBusTracker : public QObject
{
  Q_OBJECT

protected:
  explicit DBusTracker(QObject *parent = nullptr);

public:
  static DBusTracker* instance();

  void track(const QString &service);

signals:
  void serviceDisappeared(QString service);

public slots:
  // used to track lost clients
  void onNameOwnerChanged(QString name, QString old_owner, QString new_owner);

private:
  QSet<QString> m_tracked;
  QMutex m_mutex;

  static DBusTracker* s_instance;
};

#endif // DBUSTRACKER_H
