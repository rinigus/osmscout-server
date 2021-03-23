#ifndef TRACKDBUSSERVICE_H
#define TRACKDBUSSERVICE_H

#include <QDBusServiceWatcher>
#include <QObject>

class TrackDBusService : public QObject
{
  Q_OBJECT

  Q_PROPERTY(bool available READ available NOTIFY availableChanged)

public:
  explicit TrackDBusService(QObject *parent = nullptr);
  bool available() const { return m_available; }

signals:
  void availableChanged(bool available);

private:
  void serviceUpdate(const QString &service);

private:
  QDBusServiceWatcher m_watcher;
  bool m_available{false};
};

#endif // TRACKDBUSSERVICE_H
