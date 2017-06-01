#include "mapmanagerfeature.h"
#include "infohub.h"
#include "appsettings.h"
#include "config.h"

#include <QDir>
#include <QCoreApplication>

#include <QDebug>

using namespace MapManager;

Feature::Feature(PathProvider *path,
                 const QString &feature_type,
                 const QString &feature_name,
                 const QString &feature_pretty_name,
                 const QStringList &feature_files,
                 const int version):
  m_path_provider(path),
  m_type(feature_type),
  m_name(feature_name),
  m_pretty(feature_pretty_name),
  m_files(feature_files),
  m_version(version)
{
}

void Feature::loadSettings()
{
  AppSettings settings;
  m_enabled = settings.valueBool(MAPMANAGER_SETTINGS + m_name);
  m_assume_files_exist = settings.valueBool(MAPMANAGER_SETTINGS "assume_files_exist");
}

void Feature::setUrl(const QJsonObject &obj)
{
  m_url = QString();
  if (obj.contains(const_feature_id_url))
    {
      const QJsonObject o = obj.value(const_feature_id_url).toObject();
      QString base = o.value("base").toString();
      QString special = o.value(m_name).toString();
      m_url = base + "/" + special;
    }
}

QString Feature::getPath(const QJsonObject &obj) const
{
  return obj.value(m_name).toObject().value("path").toString();
}

uint64_t Feature::getSize(const QJsonObject &obj, bool force) const
{
  if (!m_enabled && !force) return 0;
  return obj.value(m_name).toObject().value("size").toString().toULongLong();
}

uint64_t Feature::getSizeCompressed(const QJsonObject &obj) const
{
  if (!m_enabled) return 0;
  return obj.value(m_name).toObject().value("size-compressed").toString().toULongLong();
}

static QDateTime toDT(const QString &t)
{
  if (t.isEmpty()) return QDateTime();
  return QDateTime::fromString(t, "yyyy-MM-dd_hh:mm");
}

QString Feature::getDateTimeAsString(const QJsonObject &obj) const
{
  return obj.value(m_name).toObject().value("timestamp").toString();
}

QDateTime Feature::getDateTime(const QJsonObject &obj) const
{
  return toDT(getDateTimeAsString(obj));
}

bool Feature::isMyType(const QJsonObject &request) const
{
  return (m_type == request.value("type").toString());
}

bool Feature::isCompatible(const QJsonObject &request) const
{
  if (!request.contains(m_name)) return true;
  return (m_version == request.value(m_name).toObject().value("version").toString().toInt());
}

bool Feature::isAvailable(const QJsonObject &request) const
{
  if (!m_enabled || !isMyType(request) || m_assume_files_exist) return true;
  if (!isCompatible(request)) return false;

  QString path = getPath(request);
  QDir dir(m_path_provider->fullPath("."));
  QString version;
  QString datetime;
  QString req_version = request.value(m_name).toObject().value("version").toString();
  QString req_datetime = request.value(m_name).toObject().value("timestamp").toString();

  for (const auto &f: m_files)
    if (!m_path_provider->isRegistered(path + "/" + f, version, datetime) ||
        version != req_version || datetime != req_datetime ||
        !dir.exists(m_path_provider->fullPath(path + "/" + f)))
      return false;
  return true;
}

bool Feature::hasFeatureDefined(const QJsonObject &request) const
{
  if (!m_enabled || !isMyType(request)) return false;
  return request.contains(m_name);
}

void Feature::checkMissingFiles(const QJsonObject &request,
                                FilesToDownload &missing) const
{
  if (!m_enabled || !isMyType(request) || !isCompatible(request) || m_assume_files_exist) return;

  QString path = getPath(request);
  QDir dir(m_path_provider->fullPath("."));
  QString version;
  QString datetime;
  QString req_version = request.value(m_name).toObject().value("version").toString();
  QString req_datetime = request.value(m_name).toObject().value("timestamp").toString();
  bool added = false;

  for (const auto &f: m_files)
    if (!m_path_provider->isRegistered(path + "/" + f, version, datetime) ||
        version != req_version || datetime != req_datetime ||
        !dir.exists(m_path_provider->fullPath(path + "/" + f)))
      {
        added = true;
        FileTask t;
        t.path = m_path_provider->fullPath(path + "/" + f);
        t.url = m_url + "/" + path + "/" + f;
        t.relpath = path + "/" + f;
        t.version = req_version;
        t.datetime = req_datetime;
        missing.files.append(t);
      }

  if (added)
    {
      // this is an upper limit of the sizes. its smaller in reality if
      // the feature is downloaded partially already
      missing.todownload += getSizeCompressed(request);
      missing.tostore  += getSize(request);
    }
}

void Feature::fillWantedFiles(const QJsonObject &request,
                              QSet<QString> &wanted) const
{
  if (!m_enabled || !isMyType(request)) return;

  QString path = getPath(request);
  for (const auto &f: m_files)
    wanted.insert( m_path_provider->fullPath(path + "/" + f) );
}

void Feature::deleteFiles(const QJsonObject &request)
{
  if (!m_enabled || !isMyType(request)) return;

  QString path = getPath(request);
  QDir dir(m_path_provider->fullPath("."));
  for (const auto &f: m_files)
    {
      QString fp = path + "/" + f;
      if (dir.remove(fp))
        InfoHub::logInfo(QCoreApplication::translate("MapManagerFeature", "Removed file: %1").arg(fp));
      else
        InfoHub::logInfo(QCoreApplication::translate("MapManagerFeature", "Failed to remove file: %1").arg(fp));
    }
}

////////////////////////////////////////////////////////////
/// libosmscout support
const static QStringList osmscout_files{
  "areaarea.idx", "areanode.idx", "areas.dat", "areasopt.dat", "areaway.idx", "bounding.dat",
  "intersections.dat", "intersections.idx", "location.idx", "nodes.dat", "router2.dat", "router.dat",
  "router.idx", "textloc.dat", "textother.dat", "textpoi.dat", "textregion.dat",
  "water.idx", "ways.dat", "waysopt.dat", "types.dat"};

FeatureOsmScout::FeatureOsmScout(PathProvider *path):
  Feature(path, "territory", "osmscout",
          QCoreApplication::translate("MapManagerFeature", "OSM Scout library"),
          osmscout_files,
          13)
{
}

QString FeatureOsmScout::errorMissing() const
{
  return QCoreApplication::translate("MapManagerFeature", "Missing libosmscout maps");
}


////////////////////////////////////////////////////////////
/// Geocoder NLP support
const static QStringList geocodernlp_files{
  "geonlp-primary.sqlite", "geonlp-normalized.trie", "geonlp-normalized-id.kch"};

FeatureGeocoderNLP::FeatureGeocoderNLP(PathProvider *path):
  Feature(path, "territory", "geocoder_nlp",
          QCoreApplication::translate("MapManagerFeature", "Geocoder-NLP"),
          geocodernlp_files,
          2)
{
}

QString FeatureGeocoderNLP::errorMissing() const
{
  return QCoreApplication::translate("MapManagerFeature", "Missing geocoder-nlp dataset");
}

////////////////////////////////////////////////////////////
/// libpostal support
const static QStringList postal_global_files{
  "address_expansions/address_dictionary.dat", "language_classifier/language_classifier.dat",
  "numex/numex.dat", "transliteration/transliteration.dat" };

const static QStringList postal_country_files{
  "address_parser/address_parser.dat", "address_parser/address_parser_phrases.trie",
  "address_parser/address_parser_vocab.trie", "geodb/geodb_feature_graph.dat",
  "geodb/geodb_features.trie", "geodb/geodb_names.trie", "geodb/geodb_postal_codes.dat",
  "geodb/geodb.spi", "geodb/geodb.spl" };

FeaturePostalGlobal::FeaturePostalGlobal(PathProvider *path):
  Feature(path, "postal/global", "postal_global",
          QCoreApplication::translate("MapManagerFeature", "Address parsing language support"),
          postal_global_files,
          1)
{
}

QString FeaturePostalGlobal::errorMissing() const
{
  return QCoreApplication::translate("MapManagerFeature", "Missing address language parsing dataset");
}

void FeaturePostalGlobal::loadSettings()
{
  Feature::loadSettings();
  AppSettings settings;
  m_enabled = settings.valueBool(MAPMANAGER_SETTINGS "postal_country");
}

FeaturePostalCountry::FeaturePostalCountry(PathProvider *path):
  Feature(path, "territory", "postal_country",
          QCoreApplication::translate("MapManagerFeature", "Address parsing country-specific support"),
          postal_country_files,
          1)
{
}

QString FeaturePostalCountry::errorMissing() const
{
  return QCoreApplication::translate("MapManagerFeature", "Missing country-specific address parsing dataset");
}

////////////////////////////////////////////////////////////
/// mapnik support
const static QStringList mapnik_global_files{
  "land-polygons-split-3857/land_polygons.cpg",
  "land-polygons-split-3857/land_polygons.index",
  "land-polygons-split-3857/land_polygons.shp",
  "land-polygons-split-3857/land_polygons.dbf",
  "land-polygons-split-3857/land_polygons.prj",
  "land-polygons-split-3857/land_polygons.shx",
  "simplified-land-polygons-complete-3857/simplified_land_polygons.cpg",
  "simplified-land-polygons-complete-3857/simplified_land_polygons.dbf",
  "simplified-land-polygons-complete-3857/simplified_land_polygons.index",
  "simplified-land-polygons-complete-3857/simplified_land_polygons.prj",
  "simplified-land-polygons-complete-3857/simplified_land_polygons.shp",
  "simplified-land-polygons-complete-3857/simplified_land_polygons.shx"
};

const static QStringList mapnik_country_files{
  "mapnik.sqlite" };

FeatureMapnikGlobal::FeatureMapnikGlobal(PathProvider *path):
  Feature(path, "mapnik/global", "mapnik_global",
          QCoreApplication::translate("MapManagerFeature", "World coastlines"),
          mapnik_global_files,
          1)
{
}

QString FeatureMapnikGlobal::errorMissing() const
{
  return QCoreApplication::translate("MapManagerFeature", "Missing world coastlines");
}

void FeatureMapnikGlobal::loadSettings()
{
  Feature::loadSettings();
  AppSettings settings;
  m_enabled = settings.valueBool(MAPMANAGER_SETTINGS "mapnik");
}

FeatureMapnikCountry::FeatureMapnikCountry(PathProvider *path):
  Feature(path, "territory", "mapnik_country",
          QCoreApplication::translate("MapManagerFeature", "Mapnik country-specific support"),
          mapnik_country_files,
          2)
{
}

QString FeatureMapnikCountry::errorMissing() const
{
  return QCoreApplication::translate("MapManagerFeature", "Missing country-specific Mapnik dataset");
}

void FeatureMapnikCountry::loadSettings()
{
  Feature::loadSettings();
  AppSettings settings;
  m_enabled = settings.valueBool(MAPMANAGER_SETTINGS "mapnik");
}

////////////////////////////////////////////////////////////
/// valhalla support

FeatureValhalla::FeatureValhalla(PathProvider *path):
  Feature(path, "territory", "valhalla",
          QCoreApplication::translate("MapManagerFeature", "Valhalla"),
          QStringList(),
          1)
{
}

QString FeatureValhalla::errorMissing() const
{
  return QCoreApplication::translate("MapManagerFeature", "Missing Valhalla tiles");
}

bool FeatureValhalla::isAvailable(const QJsonObject &request) const
{
  if (!m_enabled || !isMyType(request) || m_assume_files_exist) return true;
  if (!isCompatible(request)) return false;

  QString path = getPath(request);
  QDir dir(m_path_provider->fullPath("."));
  QString version;
  QString datetime;
  QString req_version = request.value(m_name).toObject().value("version").toString();
  QString req_datetime = request.value(m_name).toObject().value("timestamp").toString();

  for (const auto &f: m_files)
    if (!m_path_provider->isRegistered(path + "/" + f, version, datetime) ||
        version != req_version || datetime != req_datetime ||
        !dir.exists(m_path_provider->fullPath(path + "/" + f)))
      return false;
  return true;
}

void FeatureValhalla::checkMissingFiles(const QJsonObject &request,
                                FilesToDownload &missing) const
{
  if (!m_enabled || !isMyType(request) || !isCompatible(request) || m_assume_files_exist) return;

  QString path = getPath(request);
  QDir dir(m_path_provider->fullPath("."));
  QString version;
  QString datetime;
  QString req_version = request.value(m_name).toObject().value("version").toString();
  QString req_datetime = request.value(m_name).toObject().value("timestamp").toString();
  bool added = false;

  for (const auto &f: m_files)
    if (!m_path_provider->isRegistered(path + "/" + f, version, datetime) ||
        version != req_version || datetime != req_datetime ||
        !dir.exists(m_path_provider->fullPath(path + "/" + f)))
      {
        added = true;
        FileTask t;
        t.path = m_path_provider->fullPath(path + "/" + f);
        t.url = m_url + "/" + path + "/" + f;
        t.relpath = path + "/" + f;
        t.version = req_version;
        t.datetime = req_datetime;
        missing.files.append(t);
      }

  if (added)
    {
      // this is an upper limit of the sizes. its smaller in reality if
      // the feature is downloaded partially already
      missing.todownload += getSizeCompressed(request);
      missing.tostore  += getSize(request);
    }
}

void FeatureValhalla::fillWantedFiles(const QJsonObject &request,
                              QSet<QString> &wanted) const
{
  if (!m_enabled || !isMyType(request)) return;

  QString path = getPath(request);
  for (const auto &f: m_files)
    wanted.insert( m_path_provider->fullPath(path + "/" + f) );
}

void FeatureValhalla::deleteFiles(const QJsonObject &request)
{
  if (!m_enabled || !isMyType(request)) return;

  QString path = getPath(request);
  QDir dir(m_path_provider->fullPath("."));
  for (const auto &f: m_files)
    {
      QString fp = path + "/" + f;
      if (dir.remove(fp))
        InfoHub::logInfo(QCoreApplication::translate("MapManagerFeature", "Removed file: %1").arg(fp));
      else
        InfoHub::logInfo(QCoreApplication::translate("MapManagerFeature", "Failed to remove file: %1").arg(fp));
    }
}
