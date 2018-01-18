#ifndef MAPBOXGLMASTER_H
#define MAPBOXGLMASTER_H

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QSet>

#include <mutex>

class MapboxGLMaster : public QObject
{
  Q_OBJECT

public:
  explicit MapboxGLMaster(QObject *parent = nullptr);
  virtual ~MapboxGLMaster();

  bool getTile(int x, int y, int z, QByteArray &result, bool &compressed, bool &found);
  bool getGlyphs(QString sprite, QString range, QByteArray &result, bool &compressed, bool &found);
  bool getStyle(const QString &stylename, QByteArray &result);
  bool getSpriteJson(const QString &fname, QByteArray &result);
  bool getSpriteImage(const QString &fname, QByteArray &result);

signals:

public slots:
  void onSettingsChanged();
  void onMapboxGLChanged(QString world_database, QString glyphs_database, QSet<QString> country_databases);

protected:
  QString getFilePath(const QString &dname, const QString &fname);

protected:
  std::mutex m_mutex;
  QSet<QString> m_db_connections;
  QString m_hostname_port;

  QString m_world_fname;
  QString m_glyphs_fname;
  QSet<QString> m_country_fnames;

  const int const_section_level{7};
  const QString const_conn_world{"mapboxgl: world"};
  const QString const_conn_glyphs{"mapboxgl: glyphs"};
  const QString const_conn_prefix{"mapboxgl: "};

  const QString const_tag_hostname_port{"HOSTNAMEPORT"};
};

#endif // MAPBOXGLMASTER_H
