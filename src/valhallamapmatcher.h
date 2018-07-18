#ifndef VALHALLAMAPMATCHER_H
#define VALHALLAMAPMATCHER_H

#ifdef USE_VALHALLA

#include <QObject>

#include <QByteArray>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QSet>
#include <QString>

#define VALHALLA_MAP_MATCHER_TESTING

class ValhallaMapMatcher : public QObject
{
  Q_OBJECT

  /// \brief true when Map's storage dir is available
  Q_PROPERTY(bool positioningActive READ positioningActive NOTIFY positioningActiveChanged)

public:
  enum Mode { Auto=1, AutoShorter=2, Bicycle=3, Bus=4, Pedestrian=5 };

public:
  explicit ValhallaMapMatcher(QObject *parent = nullptr);
  virtual ~ValhallaMapMatcher();

  bool start(const QString &id, const Mode mode);
  bool stop(const QString &id, const Mode mode);
  bool stop(const QString &id);

  bool positioningActive() const { return m_positioning_active; }

signals:
  void positioningActiveChanged(bool);

  void propertyChanged(Mode mode, QString key, int value);
  void propertyChanged(Mode mode, QString key, double value);
  void propertyChanged(Mode mode, QString key, QString value);
  void propertyChanged(Mode mode, QString key, QGeoCoordinate value);

public slots:

protected slots:
  void onPositionUpdated(const QGeoPositionInfo &info);
  void onUpdateTimeout();
  void onPositioningError(QGeoPositionInfoSource::Error positioningError);

protected:

  // helper class to keep earlier recorded map
  // matching results. while setting the value,
  // `set` method will return true if the new
  // value of a property is different from the
  // recorded one
  struct Properties {
    QHash<QString,int> m_property_int;
    QHash<QString,double> m_property_double;
    QHash<QString,QString> m_property_string;
    QHash<QString,QGeoCoordinate> m_property_coor;

    bool set(const QString &key, int value);
    bool set(const QString &key, double value);
    bool set(const QString &key, const QString &value);
    bool set(const QString &key, const QGeoCoordinate &value);
  };


protected:

  void fillRequest(Mode mode, const QJsonArray &shape, double accuracy, QByteArray &request);

  void setProperty(Mode mode, const QString &key, int value);
  void setProperty(Mode mode, const QString &key, double value);
  void setProperty(Mode mode, const QString &key, const QString &value);
  void setProperty(Mode mode, const QString &key, const QGeoCoordinate &value);

  void clearCache();
  void stopPositioning();
  void shutdown();

#ifdef VALHALLA_MAP_MATCHER_TESTING
  virtual void timerEvent(QTimerEvent *event);
#endif

protected:
  QGeoPositionInfoSource *m_source{nullptr};
  bool m_positioning_active{false};

  QList<QGeoPositionInfo> m_locations;
  QGeoPositionInfo m_last_position_info;

  QHash<Mode, Properties > m_properties; // key=mode
  QHash<Mode, QSet<QString> > m_clients; // key=mode, set of corresponding client ids
};


#endif // USE_VALHALLA
#endif // VALHALLAMAPMATCHER_H
