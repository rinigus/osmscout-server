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
  m_maps_available = QJsonObject();
  m_map_selected.clear();

  updateOsmScout();
  updateGeocoderNLP();
  updatePostal();
}

QJsonObject MapManager::loadJson(QString fname) const
{
  QFile freq(fname);
  if (!freq.open(QIODevice::ReadOnly | QIODevice::Text)) return QJsonObject();
  return QJsonDocument::fromJson(freq.readAll()).object();
}

QString MapManager::getPath(const QJsonObject &obj, const QString &feature) const
{
  return obj.value(feature).toObject().value("path").toString();
}

QString MapManager::getId(const QJsonObject &obj) const
{
  return obj.value("id").toString();
}

QString MapManager::getPretty(const QJsonObject &obj) const
{
  if (obj.value("id").toString() == const_feature_id_postal_global)
    return tr("Address parsing language support");

  return obj.value("continent").toString() + " / " + obj.value("name").toString();
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
  QJsonObject available;
  if (m_feature_postal_country)
    {
      m_postal_global_path = getPath( req_countries.value(const_feature_id_postal_global).toObject(),
                                      const_feature_name_postal_global );
      if ( m_postal_global_path.isEmpty() )
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

      // add postal global to requested
      available.insert(const_feature_id_postal_global, req_countries.value(const_feature_id_postal_global).toObject());
    }

  // check whether we have all needed datasets for required countries
  for (QJsonObject::const_iterator request_iter = req_countries.constBegin();
       request_iter != req_countries.constEnd(); ++request_iter)
    {
      const QJsonObject request = request_iter->toObject();
      if (request.empty()) continue;

      // check if we have all keys defined
      if (request.contains("id") &&
          request.contains("name") &&
          request.contains("continent") &&
          (!m_feature_geocoder_nlp || request.contains(const_feature_name_geocoder_nlp)) &&
          (!m_feature_osmscout || request.contains(const_feature_name_osmscout)) &&
          (!m_feature_postal_country || request.contains(const_feature_name_postal_country))
          )
        {
          QString id = getId(request);
          QString geocoder_nlp = getPath(request, const_feature_name_geocoder_nlp);
          QString osmscout = getPath(request, const_feature_name_osmscout);
          QString postal_country = getPath(request, const_feature_name_postal_country);

          if (m_feature_geocoder_nlp && !hasAvailableGeocoderNLP(geocoder_nlp))
            InfoHub::logWarning(tr("Missing dataset for geocoder-nlp: ") + geocoder_nlp);
          else if (m_feature_osmscout && !hasAvailableOsmScout(osmscout))
            InfoHub::logWarning(tr("Missing dataset for libosmscout: ") + osmscout);
          else if (m_feature_postal_country && !hasAvailablePostalCountry(postal_country))
            InfoHub::logWarning(tr("Missing country-specific dataset for libpostal: ") + postal_country);
          else
            available.insert(id, request);
        }
    }

  if ( available != m_maps_available )
    {
      m_maps_available = available;

      if (!m_maps_available.contains(m_map_selected))
        m_map_selected.clear();

      bool has_postal_global = m_maps_available.contains(const_feature_id_postal_global);
      if ( (m_maps_available.count() == 1 && !has_postal_global) ||
           (m_maps_available.count() == 2 && has_postal_global) ) // there is only one map, let's select it as well
        {
          for (QJsonObject::const_iterator i = m_maps_available.constBegin();
               i != m_maps_available.constEnd(); ++i)
            if (i.key() != const_feature_id_postal_global)
              {
                m_map_selected = i.key();
                break;
              }
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
  else qDebug() << "Same config as before";
}

void MapManager::getCountriesList(bool list_available, QStringList &countries, QStringList &ids)
{
  QMutexLocker lk(&m_mutex);
  makeCountriesList(list_available, countries, ids);
}

void MapManager::makeCountriesList(bool list_available, QStringList &countries, QStringList &ids)
{
  QList< QPair<QString, QString> > available;

  for (QJsonObject::const_iterator i = m_maps_available.constBegin();
       i != m_maps_available.constEnd(); ++i )
    if (i.key() != const_feature_id_postal_global)
      {
        QJsonObject c = i->toObject();
        available.append(qMakePair(getPretty(c), getId(c)));
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

  QString path = fullPath( getPath(m_maps_available.value(m_map_selected).toObject(),
                                   const_feature_name_osmscout) );
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
  QString path = fullPath( getPath( m_maps_available.value(m_map_selected).toObject(),
                                    const_feature_name_geocoder_nlp ) + "/" +  geocodernlp_files[0] );

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
  QString path_country = fullPath( getPath( m_maps_available.value(m_map_selected).toObject(),
                                            const_feature_name_postal_country ) );

  if (settings.valueString(GEOMASTER_SETTINGS "postal_main_dir") != path_global ||
      settings.valueString(GEOMASTER_SETTINGS "postal_country_dir") != path_country )
    {
      settings.setValue(GEOMASTER_SETTINGS "postal_main_dir", path_global);
      settings.setValue(GEOMASTER_SETTINGS "postal_country_dir", path_country);
      emit databasePostalChanged(path_global, path_country);
    }
}
