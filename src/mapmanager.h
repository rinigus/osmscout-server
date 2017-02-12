#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include <QObject>
#include <QMutex>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QHash>

/// \brief Map Manager
///
/// Map Manager keeps the list of available maps, geocoder and
/// libpostal databases as well as tracks dependencies between
/// them
class MapManager : public QObject
{
  Q_OBJECT

//  Q_PROPERTY(QString masterMap READ masterMap WRITE setMasterMap NOTIFY masterMapChanged)
//  Q_PROPERTY(QStringList masterMapList READ masterMapList NOTIFY masterMapListChanged)

//  Q_PROPERTY(QString databaseOsmScout READ databaseOsmScout NOTIFY databaseOsmScoutChanged)

public:
  explicit MapManager(QObject *parent = 0);
  virtual ~MapManager();

  void loadSettings();

  Q_INVOKABLE void getCountriesList(bool list_available, QStringList &countries, QStringList &ids);

//  QString masterMap() const { return m_master_map; }
//  void setMasterMap(QString masterMap);

//  QStringList masterMapList() const { return m_master_map_list; }

//  QString databaseOsmScout() const { return m_osmscout_dirs; }

signals:
  void databaseOsmScoutChanged(QString database);
  void databaseGeocoderNLPChanged(QString database);
  void databasePostalChanged(QString global, QString country);

public slots:
  void onSettingsChanged();

protected:
  void scanDirectories();
  void nothingAvailable(); ///< Helper function called when there are no maps available

  QString fullPath(QString path) const; ///< Transform relative path to the full path

  QString getOsmScoutPath(const QString &name) const;
  QString getGeocoderNLPPath(const QString &name) const;
  QString getPostalCountryPath(const QString &name) const;

  bool hasAvailableOsmScout(const QString &name) const;
  bool hasAvailableGeocoderNLP(const QString &name) const;
  bool hasAvailablePostalCountry(const QString &name) const;
  bool hasAvailablePostalGlobal() const;

  void updateOsmScout();
  void updateGeocoderNLP();
  void updatePostal();

protected:

  /// \brief Keeps list of available maps and how different map components depend on each other
  struct MapCountry {
    QString id;
    QString name;
    QString continent;
    QString osmscout;
    QString postal_country;
    QString geocoder_nlp;

    QString pretty() const { return continent + "/" + name; }
    bool operator ==(const MapCountry &other) const {
      return (id==other.id && name==other.name && continent==other.continent &&
              osmscout==other.osmscout &&
              postal_country==other.postal_country &&
              geocoder_nlp==other.geocoder_nlp );
    }
  };

protected:
  QMutex m_mutex;

  // settings
  QDir m_root_dir;
  bool m_feature_osmscout{false};
  bool m_feature_geocoder_nlp{false};
  bool m_feature_postal_country{false};

  // available maps
  QHash< QString, MapCountry > m_maps_available;
  QString m_map_selected;

  const QString const_fname_countries_available{"countries_available.json"};
  const QString const_fname_countries_requested{"countries_requested.json"};

  const QString const_dirname_osmscout{"osmscout"};
  const QString const_dirname_geocoder_nlp{"geocoder-nlp"};
  const QString const_dirname_postal_country{"postal/countries"};
  const QString const_dirname_postal_global{"postal/global"};

};

#endif // MAPMANAGER_H
