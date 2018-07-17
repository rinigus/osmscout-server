#ifndef VALHALLAMAPMATCHER_H
#define VALHALLAMAPMATCHER_H

#ifdef USE_VALHALLA

#include <QObject>

#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QSet>

class ValhallaMapMatcher : public QObject
{
  Q_OBJECT

  /// \brief true when Map's storage dir is available
  Q_PROPERTY(bool positioningActive READ positioningActive NOTIFY positioningActiveChanged)

public:
  explicit ValhallaMapMatcher(QObject *parent = nullptr);
  virtual ~ValhallaMapMatcher();

signals:
  void positioningActiveChanged(bool);

public:

  bool start(const QString &id, const QString &mode);
  bool stop(const QString &id, const QString &mode);
  bool stop(const QString &id);

  bool positioningActive() const { return m_positioning_active; }

public slots:

protected slots:
  void onPositionUpdated(const QGeoPositionInfo &info);
  void onUpdateTimeout();
  void onPositioningError(QGeoPositionInfoSource::Error positioningError);

protected:

  void clearCache();
  void stopPositioning();
  void shutdown();

  // helper class to keep earlier recorded map
  // matching results. while setting the value,
  // `set` method will return true if the new
  // value of a property is different from the
  // recorded one
  struct Properties {
    QHash<QString,int> m_property_int;
    QHash<QString,double> m_property_double;
    QHash<QString,QString> m_property_string;

    bool set(const QString &key, int value);
    bool set(const QString &key, double value);
    bool set(const QString &key, const QString &value);
  };

protected:
  QGeoPositionInfoSource *m_source{nullptr};
  bool m_positioning_active{false};

  QList<QGeoPositionInfo> m_locations;
  QGeoPositionInfo m_last_position_info;

  QHash<QString, Properties > m_properties; // key=mode
  QHash<QString, QSet<QString> > m_clients; // key=mode, set of corresponding client ids
};


#endif // USE_VALHALLA
#endif // VALHALLAMAPMATCHER_H
