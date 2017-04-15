#ifndef MAPMANAGERFEATURE_H
#define MAPMANAGERFEATURE_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QJsonObject>
#include <QDateTime>
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
  class Feature
  {
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

    bool isAvailable(const QJsonObject &request) const;

    void checkMissingFiles(const QJsonObject &request, FilesToDownload &missing) const;

    void fillWantedFiles(const QJsonObject &request, QSet<QString> &wanted) const;

    void deleteFiles(const QJsonObject &request);

    bool hasFeatureDefined(const QJsonObject &request) const;

  protected:
    QString getDateTimeAsString(const QJsonObject &obj) const;

  protected:
    PathProvider *m_path_provider;
    const QString m_type;
    const QString m_name;
    const QString m_pretty;
    const QStringList m_files;
    const int m_version;

    bool m_enabled{false};
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
    virtual QString errorMissing() const;
  };
}

#endif // MAPMANAGERFEATURE_H
