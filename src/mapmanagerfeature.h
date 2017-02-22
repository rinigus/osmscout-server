#ifndef MAPMANAGERFEATURE_H
#define MAPMANAGERFEATURE_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QJsonObject>
#include <QDateTime>

namespace MapManager {

  /// \brief Abstract class used to get access to Map Manager full path method
  ///
  class PathProvider
  {
  public:
    virtual QString fullPath(const QString &path) const = 0;
  };

  ////////////////////////////////////////////////////////
  /// Helper structures used with the downloads
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


  /// \brief Abstract class describing features of the map in respect to Map Manager
  ///
  class Feature
  {
  protected:
    explicit Feature(const PathProvider *path,
                     const QString &feature_type,
                     const QString &feature_name,
                     const QStringList &feature_files);

  public:
    virtual ~Feature() {}

    const QString& name() const { return m_name; }

    virtual void loadSettings();
    virtual QString errorMissing() const = 0;

    bool enabled() const { return m_enabled; }
    void setEnabled(bool e) { m_enabled = e; }

    void setUrl(const QJsonObject &obj);

    QString getPath(const QJsonObject &obj) const;
    size_t getSize(const QJsonObject &obj) const;
    size_t getSizeCompressed(const QJsonObject &obj) const;
    QDateTime getDateTime(const QJsonObject &obj) const;

    bool isMyType(const QJsonObject &request) const;

    bool isAvailable(const QJsonObject &request) const;

    void checkMissingFiles(const QJsonObject &request, FilesToDownload &missing) const;

    void fillWantedFiles(const QJsonObject &request, QSet<QString> &wanted) const;

    void deleteFiles(const QJsonObject &request);

    bool hasFeatureDefined(const QJsonObject &request) const;

  protected:
    const PathProvider *m_path_provider;
    const QString m_type;
    const QString m_name;
    const QStringList m_files;

    bool m_enabled{false};
    QString m_url;

    const QString const_feature_id_url{"url"};
  };


  ///////////////////////////////////////////////
  /// Specific backends support

  class FeatureOsmScout: public Feature
  {
  public:
    FeatureOsmScout(const PathProvider *path);
    virtual ~FeatureOsmScout() {}
    virtual QString errorMissing() const;
  };

  class FeatureGeocoderNLP: public Feature
  {
  public:
    FeatureGeocoderNLP(const PathProvider *path);
    virtual ~FeatureGeocoderNLP() {}
    virtual QString errorMissing() const;
  };

  class FeaturePostalGlobal: public Feature
  {
  public:
    FeaturePostalGlobal(const PathProvider *path);
    virtual ~FeaturePostalGlobal() {}
    virtual void loadSettings();
    virtual QString errorMissing() const;
  };

  class FeaturePostalCountry: public Feature
  {
  public:
    FeaturePostalCountry(const PathProvider *path);
    virtual ~FeaturePostalCountry() {}
    virtual QString errorMissing() const;
  };

}

#endif // MAPMANAGERFEATURE_H
