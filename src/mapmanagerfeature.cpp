#include "mapmanagerfeature.h"
#include "infohub.h"
#include "appsettings.h"
#include "config.h"

#include <QDir>
#include <QCoreApplication>

#include <QDebug>

using namespace MapManager;

Feature::Feature(const PathProvider *path,
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
  return obj.value(m_name).toObject().value("size").toString().toULong();
}

uint64_t Feature::getSizeCompressed(const QJsonObject &obj) const
{
  if (!m_enabled) return 0;
  return obj.value(m_name).toObject().value("size-compressed").toString().toULong();
}

QDateTime Feature::getDateTime(const QJsonObject &obj) const
{
  QString t = obj.value(m_name).toObject().value("timestamp").toString();
  if (t.isEmpty()) return QDateTime();
  return QDateTime::fromString(t, "yyyy-MM-dd_hh:mm");
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
  if (!m_enabled || !isMyType(request)) return true;
  if (!isCompatible(request)) return false;

  QString path = getPath(request);
  QDir dir(m_path_provider->fullPath("."));

  for (const auto &f: m_files)
    if (!dir.exists(m_path_provider->fullPath(path + "/" + f)))
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
  if (!m_enabled || !isMyType(request) || !isCompatible(request)) return;

  QString path = getPath(request);
  QDir dir(m_path_provider->fullPath("."));
  bool added = false;

  for (const auto &f: m_files)
    if (!dir.exists(m_path_provider->fullPath(path + "/" + f)))
      {
        added = true;
        FileTask t;
        t.path = m_path_provider->fullPath(path + "/" + f);
        t.url = m_url + "/" + path + "/" + f;
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

FeatureOsmScout::FeatureOsmScout(const PathProvider *path):
  Feature(path, "territory", "osmscout",
          QCoreApplication::translate("MapManagerFeature", "OSM Scout library"),
          osmscout_files,
          11)
{
}

QString FeatureOsmScout::errorMissing() const
{
  return QCoreApplication::translate("MapManagerFeature", "Missing libosmscout maps");
}


////////////////////////////////////////////////////////////
/// Geocoder NLP support
const static QStringList geocodernlp_files{
  "location.sqlite"};

FeatureGeocoderNLP::FeatureGeocoderNLP(const PathProvider *path):
  Feature(path, "territory", "geocoder_nlp",
          QCoreApplication::translate("MapManagerFeature", "Geocoder-NLP"),
          geocodernlp_files,
          1)
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

FeaturePostalGlobal::FeaturePostalGlobal(const PathProvider *path):
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
  AppSettings settings;
  m_enabled = settings.valueBool(MAPMANAGER_SETTINGS "postal_country");
}

FeaturePostalCountry::FeaturePostalCountry(const PathProvider *path):
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
