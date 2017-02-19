#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include "filedownloader.h"

#include <QObject>
#include <QMutex>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QList>
#include <QPointer>

/// \brief Map Manager
///
/// Map Manager keeps the list of available maps, geocoder and
/// libpostal databases as well as tracks dependencies between
/// them
class MapManager : public QObject
{
  Q_OBJECT

  /// \brief true when download is active
  Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

//  Q_PROPERTY(QString masterMap READ masterMap WRITE setMasterMap NOTIFY masterMapChanged)
//  Q_PROPERTY(QStringList masterMapList READ masterMapList NOTIFY masterMapListChanged)

//  Q_PROPERTY(QString databaseOsmScout READ databaseOsmScout NOTIFY databaseOsmScoutChanged)

public:
  explicit MapManager(QObject *parent = 0);
  virtual ~MapManager();

  /// \brief Composes a list of countries on device in alphabetical order
  Q_INVOKABLE void getInstalledCountriesList(QStringList &countries, QStringList &ids);

  /// \brief Composes a list of countries provided for download in alphabetical order
  Q_INVOKABLE void getProvidedCountriesList(QStringList &countries, QStringList &ids);

  /// \brief Add country to the list of requested countries
  ///
  Q_INVOKABLE void addCountry(QString id);

  /// \brief Remove country from the list of requested countries
  ///
  Q_INVOKABLE void rmCountry(QString id);

  /// \brief Update a list of provided countries and features
  ///
  Q_INVOKABLE bool updateProvided();

  /// \brief Download or update missing data files
  ///
  Q_INVOKABLE bool getCountries();

  /// \brief Create a list of non-required files
  ///
  /// Makes a list of non-requiered files to show to the user. This
  /// method will fail (return false) if there are active downloads.
  /// Otherwise, we could delete partially downloaded files
  Q_INVOKABLE bool getNonNeededFilesList(QStringList &files);

//  /// \brief Delete all non-required files
//  ///
//  Q_INVOKABLE bool cleanup();

  /// Properties exposed to QML
  bool downloading();

//  QString masterMap() const { return m_master_map; }
//  void setMasterMap(QString masterMap);

//  QStringList masterMapList() const { return m_master_map_list; }

//  QString databaseOsmScout() const { return m_osmscout_dirs; }

signals:
  void databaseOsmScoutChanged(QString database);
  void databaseGeocoderNLPChanged(QString database);
  void databasePostalChanged(QString global, QString country);

  void downloadingChanged(bool state);

public slots:
  void onSettingsChanged();

protected:
  struct FileTask {
    QString url;
    QString path;
  };

  struct FilesToDownload {
    QString id;
    QString pretty;
    size_t tostore{0};
    size_t todownload{0};
    QList< FileTask > files;
  };

protected:
  void loadSettings();

  void scanDirectories();
  void nothingAvailable(); ///< Helper method called when there are no maps available

  void missingData();

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

  void checkMissingOsmScout(const QJsonObject &request, const QString &url, FilesToDownload &missing) const;
  void checkMissingGeocoderNLP(const QJsonObject &request, const QString &url, FilesToDownload &missing) const;
  void checkMissingPostalCountry(const QJsonObject &request, const QString &url, FilesToDownload &missing) const;
  void checkMissingPostalGlobal(const QJsonObject &request, const QString &url, FilesToDownload &missing) const;

  void updateOsmScout();
  void updateGeocoderNLP();
  void updatePostal();

  /// helper functions to deal with JSON representation of the features
  QJsonObject loadJson(QString fname) const;
  QString getId(const QJsonObject &obj) const;
  QString getPretty(const QJsonObject &obj) const;
  QString getPath(const QJsonObject &obj, const QString &feature) const;
  size_t getSize(const QJsonObject &obj, const QString &feature) const;
  size_t getSizeCompressed(const QJsonObject &obj, const QString &feature) const;

  void checkMissingFiles(const QJsonObject &request,
                         const QString &feature,
                         const QString &url,
                         const QStringList &files,
                         FilesToDownload &missing) const;

  // handling of downloads
  void onDownloadFinished(QString path);
  void onDownloadError(QString err);
  void onDownloadedBytes(size_t sz);
  void onWrittenBytes(size_t sz);

  bool startDownload(const QString &url, const QString &path, const QString &mode);
  void cleanupDownload();

protected:
  QMutex m_mutex;

  // settings
  QDir m_root_dir;
  bool m_feature_osmscout{false};
  bool m_feature_geocoder_nlp{false};
  bool m_feature_postal_country{false};
  QString m_provided_url;

  // available maps
  QJsonObject m_maps_available;
  //QHash< QString, MapCountry > m_maps_available;
  QString m_map_selected;

  QString m_postal_global_path;

  QList< FilesToDownload > m_missing_data;
  QNetworkAccessManager m_network_manager;
  QPointer<FileDownloader> m_file_downloader;
  bool m_downloading_countries{false};

  /// const values used to access data
  const QString const_fname_countries_provided{"countries_provided.json"};
  const QString const_fname_countries_requested{"countries_requested.json"};

  const QString const_feature_id_postal_global{"postal/global"};
  const QString const_feature_id_url{"url"};

  const QString const_feature_name_postal_global{"postal_global"};
  const QString const_feature_name_postal_country{"postal_country"};
  const QString const_feature_name_osmscout{"osmscout"};
  const QString const_feature_name_geocoder_nlp{"geocoder_nlp"};
};

#endif // MAPMANAGER_H
