#ifndef MAPBOXGLMASTER_H
#define MAPBOXGLMASTER_H

#include <QByteArray>
#include <QObject>
#include <QString>

class MapboxGLMaster : public QObject
{
  Q_OBJECT

public:
  explicit MapboxGLMaster(QObject *parent = nullptr);
  virtual ~MapboxGLMaster();

  bool getTile(int x, int y, int z, QByteArray &result, bool &compressed, bool &found);

signals:

public slots:
  void onSettingsChanged();
  void onMapboxGLChanged(QString world_database, QStringList country_databases);

protected:

  const QString const_conn_world{"mapboxgl: world"};
};

#endif // MAPBOXGLMASTER_H
