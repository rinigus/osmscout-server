#include "mapmanager.h"

#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QMutexLocker>
#include <QDirIterator>
#include <QDir>

#include <QFile>
#include <QBitArray>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <QList>
#include <QPair>
#include <algorithm>

#include <QDebug>

MapManager::MapManager(QObject *parent) : QObject(parent)
{
  loadSettings();
}


MapManager::~MapManager()
{
}

void MapManager::onSettingsChanged()
{
  QMutexLocker lk(&m_mutex);
  loadSettings();
}

void MapManager::loadSettings()
{
  AppSettings settings;

  m_root_dir.setPath(settings.valueString(MAPMANAGER_SETTINGS "root"));
  m_map_selected = settings.valueString(MAPMANAGER_SETTINGS "map_selected");

  m_feature_osmscout = settings.valueBool(MAPMANAGER_SETTINGS "osmscout");
  m_feature_geocoder_nlp = settings.valueBool(MAPMANAGER_SETTINGS "geocoder_nlp");
  m_feature_postal_country = settings.valueBool(MAPMANAGER_SETTINGS "postal_country");

  scanDirectories();
}

void MapManager::nothingAvailable()
{
  m_maps_available.clear();
  m_map_selected.clear();

  updateOsmScout();
  updateGeocoderNLP();
  updatePostal();
}

////////////////////////////////////////////////////////
/// helper functions to deal with JSON
static QJsonObject loadJson(QString fname)
{
  QFile freq(fname);
  if (!freq.open(QIODevice::ReadOnly | QIODevice::Text)) return QJsonObject();
  return QJsonDocument::fromJson(freq.readAll()).object();
}

static bool getFeatureData(const QJsonObject &obj, const QString &feature, QString &path, int &size, int &size_compressed)
{
  if (obj.contains(feature))
    {
      const QJsonObject &fo = obj.value(feature).toObject();
      path = fo.value("path").toString();
      size = fo.value("size").toInt();
      size_compressed = fo.value("size-compressed").toInt();
      return true;
    }
  path = QString();
  size = size_compressed = 0;
  return false;
}

static QString getPath(const QJsonObject &obj, const QString &feature)
{
  QString path;
  int sz, szc;
  getFeatureData(obj, feature, path, sz, szc);
  return path;
}

/// helper functions to deal with JSON: done
////////////////////////////////////////////////////////


void MapManager::scanDirectories()
{
  if (!m_root_dir.exists())
    {
      InfoHub::logWarning(tr("Maps directory does not exist: ") + m_root_dir.absolutePath());
      nothingAvailable();
      return;
    }

  // load list of requested countries and features
  if (!m_root_dir.exists(const_fname_countries_requested))
    {
      InfoHub::logWarning(tr("No maps were requested"));
      nothingAvailable();
      return;
    }

  QJsonObject req_countries = loadJson(m_root_dir.absoluteFilePath(const_fname_countries_requested));

  // with postal countries requested, check if we have postal global part
  if (m_feature_postal_country)
    {
      int sz, sz_compressed;
      if ( !getFeatureData( req_countries.value(const_feature_name_postal_global).toObject(),
                            "postal_global", m_postal_global_path, sz, sz_compressed) )
        {
          InfoHub::logWarning(tr("No maps loaded: libpostal language support is not requested"));
          nothingAvailable();
          return;
        }

      if (!hasAvailablePostalGlobal() )
        {
          InfoHub::logWarning(tr("No maps loaded: libpostal language support unavailable"));
          nothingAvailable();
          return;
        }
    }

  // check whether we have all needed datasets for required countries
  QHash<QString, MapCountry> available;
  for (QJsonObject::const_iterator request_iter = req_countries.constBegin();
       request_iter != req_countries.constEnd(); ++request_iter)
    {
      const QJsonObject request = request_iter->toObject();
      if (request.empty()) continue;

      // check if we have all keys defined
      if (request.contains("id") &&
          request.contains("name") &&
          request.contains("continent") &&
          (!m_feature_geocoder_nlp || request.contains("geocoder_nlp")) &&
          (!m_feature_osmscout || request.contains("osmscout")) &&
          (!m_feature_postal_country || request.contains("postal_country"))
          )
        {
          MapCountry country;
          country.id = request.value("id").toString();
          country.name = request.value("name").toString();
          country.continent = request.value("continent").toString();
          country.geocoder_nlp = getPath(request, "geocoder_nlp");
          country.osmscout = getPath(request, "osmscout");
          country.postal_country = getPath(request, "postal_country");

          if (m_feature_geocoder_nlp && !hasAvailableGeocoderNLP(country.geocoder_nlp))
            InfoHub::logWarning(tr("Missing dataset for geocoder-nlp: ") + country.geocoder_nlp);
          else if (m_feature_osmscout && !hasAvailableOsmScout(country.osmscout))
            InfoHub::logWarning(tr("Missing dataset for libosmscout: ") + country.osmscout);
          else if (m_feature_postal_country && !hasAvailablePostalCountry(country.postal_country))
            InfoHub::logWarning(tr("Missing country-specific dataset for libpostal: ") + country.postal_country);
          else
            available[country.id] = country;
        }
    }

  if ( available != m_maps_available )
    {
      m_maps_available = available;

      if (!m_maps_available.contains(m_map_selected))
        m_map_selected.clear();

      if (m_maps_available.count() == 1) // there is only one map, let's select it as well
        {
          auto i = m_maps_available.begin();
          m_map_selected = i->id;
        }

      QStringList countries, ids;
      makeCountriesList(true, countries, ids);

      // print all loaded countries
      for (const auto &c: countries)
        InfoHub::logInfo(tr("Available country or territory: ") + c);

      updateOsmScout();
      updateGeocoderNLP();
      updatePostal();
    }
}

void MapManager::getCountriesList(bool list_available, QStringList &countries, QStringList &ids)
{
  QMutexLocker lk(&m_mutex);
  makeCountriesList(list_available, countries, ids);
}

void MapManager::makeCountriesList(bool list_available, QStringList &countries, QStringList &ids)
{
  QList< QPair<QString, QString> > available;

  QHashIterator<QString, MapCountry> i(m_maps_available);
  for (QHashIterator<QString, MapCountry> i(m_maps_available);
       i.hasNext(); )
    {
      i.next();
      available.append(qMakePair(i.value().pretty(), i.value().id));
    }

  std::sort(available.begin(), available.end());

  countries.clear();
  ids.clear();
  for (const auto &i: available)
    {
      countries.append(i.first);
      ids.append(i.second);
    }
}

void MapManager::addCountry(QString id)
{
  QMutexLocker lk(&m_mutex);

  if (!m_maps_available.contains(id) && m_root_dir.exists() && m_root_dir.exists(const_fname_countries_provided))
    {
      QJsonObject possible = loadJson(m_root_dir.absoluteFilePath(const_fname_countries_provided));
      QJsonObject requested = loadJson(m_root_dir.absoluteFilePath(const_fname_countries_requested));

      if (possible.contains(id) && possible.value(id).toObject().value("id") == id)
        {
          requested.insert(id, possible.value(id).toObject());

          QJsonDocument doc(requested);
          QFile file(m_root_dir.absoluteFilePath(const_fname_countries_requested));
          file.open(QIODevice::WriteOnly | QIODevice::Text);
          file.write( doc.toJson() );
        }

      scanDirectories();
    }
}

QString MapManager::fullPath(QString path) const
{
  if (path.length() < 1) return QString();
  QDir dir(m_root_dir.filePath(path));
  return dir.canonicalPath();
}

////////////////////////////////////////////////////////////
/// libosmscout support
const static QStringList osmscout_files{
  "areaarea.idx", "areanode.idx", "areas.dat", "areasopt.dat", "areaway.idx", "bounding.dat",
  "intersections.dat", "intersections.idx", "location.idx", "nodes.dat", "router2.dat", "router.dat",
  "router.idx", "textloc.dat", "textother.dat", "textpoi.dat", "textregion.dat", "types.dat",
  "water.idx", "ways.dat", "waysopt.dat"};

bool MapManager::hasAvailableOsmScout(const QString &path) const
{
  for (const auto &f: osmscout_files)
    if (!m_root_dir.exists(path + "/" + f))
      return false;
  return true;
}

void MapManager::updateOsmScout()
{
  AppSettings settings;

  QString path = fullPath( m_maps_available.value(m_map_selected).osmscout );
  if (settings.valueString(OSM_SETTINGS "map") != path)
    {
      settings.setValue(OSM_SETTINGS "map", path);
      emit databaseOsmScoutChanged(path);
    }
}

////////////////////////////////////////////////////////////
/// Geocoder NLP support
const static QStringList geocodernlp_files{
  "location.sqlite"};

bool MapManager::hasAvailableGeocoderNLP(const QString &path) const
{
  for (const auto &f: geocodernlp_files)
    if (!m_root_dir.exists(path + "/" + f))
      return false;
  return true;
}

void MapManager::updateGeocoderNLP()
{
  AppSettings settings;

  // version of the geocoder where all data is in a single file
  QString path = fullPath( m_maps_available.value(m_map_selected).geocoder_nlp + "/" +  geocodernlp_files[0] );

  if (settings.valueString(GEOMASTER_SETTINGS "geocoder_path") != path)
    {
      settings.setValue(GEOMASTER_SETTINGS "geocoder_path", path);
      emit databaseGeocoderNLPChanged(path);
    }
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

bool MapManager::hasAvailablePostalGlobal() const
{
  for (const auto &f: postal_global_files)
    if (!m_root_dir.exists(m_postal_global_path + "/" + f))
      return false;
  return true;
}

bool MapManager::hasAvailablePostalCountry(const QString &path) const
{
  for (const auto &f: postal_country_files)
    if (!m_root_dir.exists(path + "/" + f))
      return false;
  return true;
}

void MapManager::updatePostal()
{
  AppSettings settings;

  QString path_global = fullPath( m_postal_global_path );
  QString path_country = fullPath( m_maps_available.value(m_map_selected).postal_country );

  if (settings.valueString(GEOMASTER_SETTINGS "postal_main_dir") != path_global ||
      settings.valueString(GEOMASTER_SETTINGS "postal_country_dir") != path_country )
    {
      settings.setValue(GEOMASTER_SETTINGS "postal_main_dir", path_global);
      settings.setValue(GEOMASTER_SETTINGS "postal_country_dir", path_country);
      emit databasePostalChanged(path_global, path_country);
    }
}
