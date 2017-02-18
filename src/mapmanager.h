#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include <QObject>
#include <QMutex>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QJsonObject>

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

  /// \brief Composes a list of countries in alphabetical order
  ///
  /// Thread-safe wrapper to makeCountriesList
  Q_INVOKABLE void getCountriesList(bool list_available, QStringList &countries, QStringList &ids);

  /// \brief Add country to the list of requested countries
  ///
  Q_INVOKABLE void addCountry(QString id);

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
  void loadSettings();

  void scanDirectories();
  void nothingAvailable(); ///< Helper method called when there are no maps available

  /// \brief Composes a list of countries in alphabetical order
  ///
  /// This is a method that creates a list. Its called by other methods to retrieve the list.
  /// Note that its not locking a mutex and its assumed that the calling method provides
  /// thread-safety
  void makeCountriesList(bool list_available, QStringList &countries, QStringList &ids);

  QString fullPath(QString path) const; ///< Transform relative path to the full path

  bool hasAvailableOsmScout(const QString &path) const;
  bool hasAvailableGeocoderNLP(const QString &path) const;
  bool hasAvailablePostalCountry(const QString &path) const;
  bool hasAvailablePostalGlobal() const;

  void updateOsmScout();
  void updateGeocoderNLP();
  void updatePostal();

  /// helper functions to deal with JSON representation of the features
  QJsonObject loadJson(QString fname) const;
  QString getPath(const QJsonObject &obj, const QString &feature) const;
  QString getId(const QJsonObject &obj) const;
  QString getPretty(const QJsonObject &obj) const;


protected:
  QMutex m_mutex;

  // settings
  QDir m_root_dir;
  bool m_feature_osmscout{false};
  bool m_feature_geocoder_nlp{false};
  bool m_feature_postal_country{false};

  // available maps
  QJsonObject m_maps_available;
  //QHash< QString, MapCountry > m_maps_available;
  QString m_map_selected;

  QString m_postal_global_path;

  const QString const_fname_countries_provided{"countries_provided.json"};
  const QString const_fname_countries_requested{"countries_requested.json"};

  const QString const_feature_id_postal_global{"postal/global"};

  const QString const_feature_name_postal_global{"postal_global"};
  const QString const_feature_name_postal_country{"postal_country"};
  const QString const_feature_name_osmscout{"osmscout"};
  const QString const_feature_name_geocoder_nlp{"geocoder_nlp"};
};

#endif // MAPMANAGER_H
