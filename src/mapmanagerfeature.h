#ifndef MAPMANAGERFEATURE_H
#define MAPMANAGERFEATURE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QJsonObject>
#include <QDateTime>
#include <QQueue>

#include <cstdint>

namespace MapManager {

  /// \brief Abstract class used to get access to Map Manager full path method
  ///
  class PathProvider
  {
  public:
    virtual QString fullPath(const QString &path) const = 0;
    virtual bool isRegistered(const QString &path, QString &version, QString &datetime) = 0;
  };

  ////////////////////////////////////////////////////////
  /// Helper structures used with the downloads
  struct FileTask {
    QString url;
    QString path;
    QString relpath;
    QString version;
    QString datetime;
  };

  struct FilesToDownload {
    QString id;
    QString pretty;
    uint64_t tostore{0};
    uint64_t todownload{0};
    QList< FileTask > files;
  };


  /// \brief Abstract class describing features of the map in respect to Map Manager
  ///
  class Feature: public QObject
  {
    Q_OBJECT

  protected:
    explicit Feature(PathProvider *path,
                     const QString &feature_type,
                     const QString &feature_name,
                     const QString &feature_pretty_name,
                     const QStringList &feature_files,
                     const int version);    

  public:
    virtual ~Feature() {}

    const QString& name() const { return m_name; }
    const QString& pretty() const { return m_pretty; }

    virtual void loadSettings();
    virtual QString errorMissing() const = 0;

    bool enabled() const { return m_enabled; }
    void setEnabled(bool e) { m_enabled = e; }

    void setUrl(const QJsonObject &obj);

    QString getPath(const QJsonObject &obj) const;
    uint64_t getSize(const QJsonObject &obj, bool force = false) const;
    uint64_t getSizeCompressed(const QJsonObject &obj) const;
    QDateTime getDateTime(const QJsonObject &obj) const;

    bool isMyType(const QJsonObject &request) const;

    bool isCompatible(const QJsonObject &request) const;

    virtual bool isAvailable(const QJsonObject &request);

    virtual void checkMissingFiles(const QJsonObject &request, FilesToDownload &missing);

    virtual void fillWantedFiles(const QJsonObject &request, QSet<QString> &wanted);

    // virtual void deleteFiles(const QJsonObject &request); // not used

    bool hasFeatureDefined(const QJsonObject &request) const;

  signals:
    void availabilityChanged();

  protected:
    QString getDateTimeAsString(const QJsonObject &obj) const;

  protected:
    PathProvider *m_path_provider;
    const QString m_type;
    const QString m_name;
    const QString m_pretty;
    const int m_version;
    QStringList m_files;

    bool m_enabled{false};
    bool m_assume_files_exist{false};
    QString m_url;

    const QString const_feature_id_url{"url"};
  };


  ///////////////////////////////////////////////
  /// Specific backends support

  class FeatureOsmScout: public Feature
  {
  public:
    FeatureOsmScout(PathProvider *path);
    virtual ~FeatureOsmScout() {}
    virtual QString errorMissing() const;
  };

  class FeatureGeocoderNLP: public Feature
  {
  public:
    FeatureGeocoderNLP(PathProvider *path);
    virtual ~FeatureGeocoderNLP() {}
    virtual QString errorMissing() const;
  };

  class FeaturePostalGlobal: public Feature
  {
  public:
    FeaturePostalGlobal(PathProvider *path);
    virtual ~FeaturePostalGlobal() {}
    virtual void loadSettings();
    virtual QString errorMissing() const;
  };

  class FeaturePostalCountry: public Feature
  {
  public:
    FeaturePostalCountry(PathProvider *path);
    virtual ~FeaturePostalCountry() {}
    virtual QString errorMissing() const;
  };

  class FeatureMapnikGlobal: public Feature
  {
  public:
    FeatureMapnikGlobal(PathProvider *path);
    virtual ~FeatureMapnikGlobal() {}
    virtual void loadSettings();
    virtual QString errorMissing() const;
  };

  class FeatureMapnikCountry: public Feature
  {
  public:
    FeatureMapnikCountry(PathProvider *path);
    virtual ~FeatureMapnikCountry() {}
    virtual void loadSettings();
    virtual QString errorMissing() const;
  };

  class FeatureMapboxGLGlobal: public Feature
  {
  public:
    FeatureMapboxGLGlobal(PathProvider *path);
    virtual ~FeatureMapboxGLGlobal() {}
    virtual void loadSettings();
    virtual QString errorMissing() const;
  };

  class FeatureMapboxGLGlyphs: public Feature
  {
  public:
    FeatureMapboxGLGlyphs(PathProvider *path);
    virtual ~FeatureMapboxGLGlyphs() {}
    virtual void loadSettings();
    virtual QString errorMissing() const;
  };

  class FeatureMapboxGLCountry: public Feature
  {
  public:
    FeatureMapboxGLCountry(PathProvider *path);
    virtual ~FeatureMapboxGLCountry() {}
    virtual void loadSettings();
    virtual QString errorMissing() const;

    virtual bool isAvailable(const QJsonObject &request);
    virtual void checkMissingFiles(const QJsonObject &request, FilesToDownload &missing);
    virtual void fillWantedFiles(const QJsonObject &request, QSet<QString> &wanted);

  protected:
    void requestFiles(const QJsonObject &request); /// refreshes m_files from request pack
  };

  /// \brief Valhalla support
  ///
  /// Valhalla's support includes handling of the packages. If some other
  /// backend will resurface with the similar distribution model, this class
  /// should be refactored into general package handling and Valhalla's specific
  /// parts.
  ///
  class FeatureValhalla: public Feature
  {
  public:
    FeatureValhalla(PathProvider *path);
    virtual ~FeatureValhalla() {}

    virtual QString errorMissing() const;

    virtual bool isAvailable(const QJsonObject &request);
    virtual void checkMissingFiles(const QJsonObject &request, FilesToDownload &missing);
    virtual void fillWantedFiles(const QJsonObject &request, QSet<QString> &wanted);

  protected:
    enum PackStateType { PackNotAvailable, PackDownloaded, PackUnpacked };

    struct PackTask {
      QString filename;
      QString datetime;

      PackTask() {}
      PackTask(const QString &fname, const QString &dtime):
        filename(fname), datetime(dtime) {}

      bool operator ==(const PackTask &a) const
      { return filename == a.filename && datetime == a.datetime; }

      bool isEmpty() const { return filename.isEmpty() && datetime.isEmpty(); }
    };

  protected:
    QString packFileName(QString pack) const;
    QString packListName(QString pack) const;
    QString getTilesTimestamp() const;

    QStringList getPackTileNames(QString listfname) const; ///< provides list of tile fnames; uses list filename as an argument
    PackStateType getPackState(QString pack, QString req_version, QString req_datetime) const;

    PackStateType isPackAvailable(QString pack, QString req_version, QString req_datetime);

    void addForUnpacking(QString packtarname, QString datetime);
    void handlePackTasks();
    void onPackTaskFinished();
    void onPackTaskError(QString errtxt);

  protected:

    QQueue<PackTask> m_pack_tasks;
    PackTask m_pack_task_current;

    const QString const_valhalla_tiles_dirname{"valhalla/tiles"};
    const QString const_valhalla_tiles_timestamp{"valhalla/tiles/timestamp"};
  };
}

#endif // MAPMANAGERFEATURE_H
