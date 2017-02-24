#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include "filedownloader.h"
#include "mapmanagerfeature.h"

#include <QObject>
#include <QMutex>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QList>
#include <QSet>
#include <QPointer>
#include <QDateTime>
#include <QAtomicInt>
#include <cstdint>

namespace MapManager {

  /// \brief Map Manager
  ///
  /// Map Manager keeps the list of available maps, geocoder and
  /// libpostal databases as well as tracks dependencies between
  /// them
  class Manager : public QObject, public PathProvider
  {
    Q_OBJECT

    /// \brief true when download is active
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

  public:
    explicit Manager(QObject *parent = 0);
    virtual ~Manager();

    /// \brief Composes a list of countries on device in alphabetical order and returns as an JSON array
    Q_INVOKABLE QString getInstalledCountries();

    /// \brief Composes a list of countries provided for download in alphabetical order as an JSON array
    Q_INVOKABLE QString getProvidedCountries();

    /// \brief Add country to the list of requested countries
    ///
    Q_INVOKABLE void addCountry(QString id);

    /// \brief Remove country from the list of requested countries
    ///
    Q_INVOKABLE void rmCountry(QString id);

    /// \brief Download or update missing data files
    ///
    Q_INVOKABLE bool getCountries();

    /// \brief Update a list of provided countries and features
    ///
    /// When the list is retrieved, the installed countries and features
    /// are checked for updates. All found updates are send via signal
    /// updatesFound as a JSON argument. The last found updates are also
    /// available via updatesFound() method
    Q_INVOKABLE bool updateProvided();

    /// \brief List of updates found when fetching the list of provided countries and features
    Q_INVOKABLE QString updatesFound();

    /// \brief Gets missing countries and the found updates
    Q_INVOKABLE void getUpdates();

    /// \brief Create a list of non-required files
    ///
    /// Makes a list of non-required files to show to the user. This
    /// method will fail (return -1) if there are active downloads.
    /// Otherwise, we could delete partially downloaded files. If the list is
    /// found, the returned value would correspond to the size occupied
    /// by the files.
    Q_INVOKABLE qint64 getNonNeededFilesList(QStringList &files);

    /// \brief Delete non-required files
    ///
    /// Deletes files found by the getNonNeededFilesList earlier. It
    /// is required to call getNonNeededFilesList first, after that, call
    /// deleteNonNeededFiles with the same list as found by getNonNeededFilesList.
    /// If the lists don't match, the files will not get deleted. Returns true if
    /// files were deleted successfully.
    Q_INVOKABLE bool deleteNonNeededFiles(const QStringList &files);

    /// Properties exposed to QML
    bool downloading();

  signals:
    void databaseOsmScoutChanged(QString database);
    void databaseGeocoderNLPChanged(QString database);
    void databasePostalChanged(QString global, QString country);

    void downloadingChanged(bool state);
    void downloadProgress(QString info);

    void updatesFound(QString info);

    void errorMessage(QString info);


  public slots:
    void onSettingsChanged();

  protected:
    //enum DownloadType { NoDownload=0, Countries=1, ProvidedList=2 };
    typedef int DownloadType;
    const int NoDownload{0};
    const int Countries{1};
    const int ProvidedList{2};

  protected:
    void loadSettings();

    void scanDirectories();
    void nothingAvailable(); ///< Helper method called when there are no maps available

    void missingData();

    void addCountryNoLock(QString id);
    void rmCountryNoLock(QString id);

    /// \brief Composes a list of countries in alphabetical order
    ///
    /// This is a method that creates a list. Its called by other methods to retrieve the list.
    /// Note that its not locking a mutex and its assumed that the calling method provides
    /// thread-safety
    void makeCountriesList(bool list_available, QStringList &countries, QStringList &ids, QList<uint64_t> &sz);

    /// \brief Wrapper around makeCountriesList transforming the results to JSON
    QString makeCountriesListAsJSON(bool list_available);

    virtual QString fullPath(const QString &path) const; ///< Transform relative path to the full path

    void updateOsmScout();
    void updateGeocoderNLP();
    void updatePostal();

    /// helper functions to deal with JSON representation of the features
    QJsonObject loadJson(QString fname) const;
    QString getId(const QJsonObject &obj) const;
    QString getType(const QJsonObject &obj) const;
    QString getPretty(const QJsonObject &obj) const;

    void checkUpdates();

    // handling of downloads
    void onDownloadFinished(QString path);
    void onDownloadError(QString err);
    void onDownloadedBytes(size_t sz);
    void onWrittenBytes(size_t sz);
    void onDownloadProgress();

    bool startDownload(DownloadType type, const QString &url, const QString &path, const QString &mode);
    void cleanupDownload();

  protected:
    QMutex m_mutex;

    // settings
    QDir m_root_dir;
    QList< Feature* > m_features;
    QString m_provided_url;

    // available maps
    QJsonObject m_maps_available;
    QString m_map_selected;

    QString m_postal_global_path;

    QList< FilesToDownload > m_missing_data;
    QNetworkAccessManager m_network_manager;
    QPointer<FileDownloader> m_file_downloader;

    QAtomicInt m_download_type{NoDownload};
    size_t m_last_reported_downloaded;
    size_t m_last_reported_written;

    QStringList m_not_needed_files;

    QJsonObject m_last_found_updates;

    /// const values used to access data
    const QString const_fname_countries_provided{"countries_provided.json"};
    const QString const_fname_countries_requested{"countries_requested.json"};

    const QString const_feature_id_postal_global{"postal/global"};
    const QString const_feature_type_country{"territory"};
  };

}
#endif // MAPMANAGER_H
