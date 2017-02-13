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

static QJsonArray loadJson(QString fname)
{
  QFile freq(fname);
  if (!freq.open(QIODevice::ReadOnly | QIODevice::Text)) return QJsonArray();
  return QJsonDocument::fromJson(freq.readAll()).array();
}

void MapManager::scanDirectories()
{
  if (!m_root_dir.exists())
    {
      InfoHub::logWarning(tr("Maps directory does not exist: ") + m_root_dir.absolutePath());
      nothingAvailable();
      return;
    }

  // with postal countries requested, check if we have postal global part
  if (m_feature_postal_country && !hasAvailablePostalGlobal())
    {
      InfoHub::logWarning(tr("No maps loaded: libpostal language support unavailable"));
      nothingAvailable();
      return;
    }

  // load list of requested countries
  if (!m_root_dir.exists(const_fname_countries_requested))
    {
      InfoHub::logWarning(tr("No maps were requested"));
      nothingAvailable();
      return;
    }

  QJsonArray req_countries = loadJson(m_root_dir.absoluteFilePath(const_fname_countries_requested));

  // check whether we have all needed datasets for required countries
  QHash<QString, MapCountry> available;
  for (QJsonArray::const_iterator request_iter = req_countries.constBegin();
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
          country.geocoder_nlp = request.value("geocoder_nlp").toString();
          country.osmscout = request.value("osmscout").toString();
          country.postal_country = request.value("postal_country").toString();

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

  if (!m_maps_available.contains(id) && m_root_dir.exists() && m_root_dir.exists(const_fname_countries_available))
    {
      QJsonArray possible = loadJson(m_root_dir.absoluteFilePath(const_fname_countries_available));
      QJsonArray requested = loadJson(m_root_dir.absoluteFilePath(const_fname_countries_requested));

      for (QJsonArray::const_iterator iter = possible.constBegin();
           iter != possible.constEnd(); ++iter)
        {
          const QJsonObject request = iter->toObject();
          if (request.empty()) continue;

          // check if we have all keys defined
          if (request.contains("id") && request.value("id").toString() == id)
            {
              requested.append(request);
              QJsonDocument doc(requested);
              QFile file(m_root_dir.absoluteFilePath(const_fname_countries_requested));
              file.open(QIODevice::WriteOnly | QIODevice::Text);
              file.write( doc.toJson() );
              break;
            }
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

QString MapManager::getOsmScoutPath(const QString &name) const
{
  if (name.length() < 1) return QString();
  return const_dirname_osmscout + "/" + name;
}

bool MapManager::hasAvailableOsmScout(const QString &name) const
{
  for (const auto &f: osmscout_files)
    if (!m_root_dir.exists(getOsmScoutPath(name) + "/" + f))
      return false;
  return true;
}

void MapManager::updateOsmScout()
{
  AppSettings settings;

  QString path = fullPath( getOsmScoutPath( m_maps_available.value(m_map_selected).osmscout ) );
  if (settings.valueString(OSM_SETTINGS "map") != path)
    {
      settings.setValue(OSM_SETTINGS "map", path);
      emit databaseOsmScoutChanged(path);
    }
}

////////////////////////////////////////////////////////////
/// Geocoder NLP support

QString MapManager::getGeocoderNLPPath(const QString &name) const
{
  if (name.length() < 1) return QString();
  return const_dirname_geocoder_nlp + "/" + name;
}

bool MapManager::hasAvailableGeocoderNLP(const QString &name) const
{
  return m_root_dir.exists(getGeocoderNLPPath(name));
}

void MapManager::updateGeocoderNLP()
{
  AppSettings settings;

  QString path = fullPath( getGeocoderNLPPath( m_maps_available.value(m_map_selected).geocoder_nlp ) );

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
    if (!m_root_dir.exists(const_dirname_postal_global + "/" + f))
      return false;
  return true;
}

QString MapManager::getPostalCountryPath(const QString &name) const
{
  if (name.length() < 1) return QString();
  return const_dirname_postal_country + "/" + name;
}

bool MapManager::hasAvailablePostalCountry(const QString &name) const
{
  for (const auto &f: postal_country_files)
    if (!m_root_dir.exists(getPostalCountryPath(name) + "/" + f))
      return false;
  return true;
}

void MapManager::updatePostal()
{
  AppSettings settings;

  QString path_global = fullPath( const_dirname_postal_global );
  QString path_country = fullPath( getPostalCountryPath( m_maps_available.value(m_map_selected).postal_country ) );

  if (settings.valueString(GEOMASTER_SETTINGS "postal_main_dir") != path_global ||
      settings.valueString(GEOMASTER_SETTINGS "postal_country_dir") != path_country )
    {
      settings.setValue(GEOMASTER_SETTINGS "postal_main_dir", path_global);
      settings.setValue(GEOMASTER_SETTINGS "postal_country_dir", path_country);
      emit databasePostalChanged(path_global, path_country);
    }
}
