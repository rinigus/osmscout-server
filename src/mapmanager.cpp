#include "mapmanager.h"

#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QMutexLocker>
#include <QDirIterator>
#include <QDir>

#include <QFile>
#include <QBitArray>
#include <QPair>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

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
  m_provided_url = settings.valueString(MAPMANAGER_SETTINGS "provided_url");

  m_feature_osmscout = settings.valueBool(MAPMANAGER_SETTINGS "osmscout");
  m_feature_geocoder_nlp = settings.valueBool(MAPMANAGER_SETTINGS "geocoder_nlp");
  m_feature_postal_country = settings.valueBool(MAPMANAGER_SETTINGS "postal_country");

  scanDirectories();
  missingData();
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

size_t MapManager::getSize(const QJsonObject &obj, const QString &feature) const
{
  return obj.value(feature).toObject().value("size").toString().toULong();
}

size_t MapManager::getSizeCompressed(const QJsonObject &obj, const QString &feature) const
{
  return obj.value(feature).toObject().value("size-compressed").toString().toULong();
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

  QJsonObject req_countries = loadJson(fullPath(const_fname_countries_requested));

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
      const QJsonObject request = request_iter.value().toObject();
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
}

void MapManager::getInstalledCountriesList(QStringList &countries, QStringList &ids)
{
  QMutexLocker lk(&m_mutex);
  makeCountriesList(true, countries, ids);
}

void MapManager::getProvidedCountriesList(QStringList &countries, QStringList &ids)
{
  QMutexLocker lk(&m_mutex);
  makeCountriesList(false, countries, ids);
}

void MapManager::makeCountriesList(bool list_available, QStringList &countries, QStringList &ids)
{
  QList< QPair<QString, QString> > available;

  QJsonObject objlist;

  if (list_available) objlist = m_maps_available;
  else objlist = loadJson(fullPath(const_fname_countries_provided));

  for (QJsonObject::const_iterator i = objlist.constBegin();
       i != objlist.constEnd(); ++i )
    if (i.key() != const_feature_id_postal_global && i.key() != const_feature_id_url)
      {
        QJsonObject c = i.value().toObject();
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
      QJsonObject possible = loadJson(fullPath(const_fname_countries_provided));
      QJsonObject requested = loadJson(fullPath(const_fname_countries_requested));

      if (possible.contains(id) && possible.value(id).toObject().value("id") == id)
        {
          requested.insert(id, possible.value(id).toObject());

          QJsonDocument doc(requested);
          QFile file(fullPath(const_fname_countries_requested));
          file.open(QIODevice::WriteOnly | QIODevice::Text);
          file.write( doc.toJson() );
        }

      scanDirectories();
      missingData();
    }
}

void MapManager::rmCountry(QString id)
{
  QMutexLocker lk(&m_mutex);

  if (m_maps_available.contains(id) && m_root_dir.exists() && m_root_dir.exists(const_fname_countries_provided))
    {
      QJsonObject requested = loadJson(fullPath(const_fname_countries_requested));

      if (requested.contains(id))
        {
          InfoHub::logInfo(tr("Removing country from requested list: ") + getPretty(requested.value(id).toObject()));

          requested.remove(id);

          QJsonDocument doc(requested);
          QFile file(fullPath(const_fname_countries_requested));
          file.open(QIODevice::WriteOnly | QIODevice::Text);
          file.write( doc.toJson() );
        }

      scanDirectories();
      missingData();
    }
}

void MapManager::missingData()
{
  if (!m_root_dir.exists())
    {
      InfoHub::logWarning(tr("Maps directory does not exist: ") + m_root_dir.absolutePath());
      return;
    }

  // load list of requested countries and features
  if (!m_root_dir.exists(const_fname_countries_requested))
    {
      InfoHub::logWarning(tr("No maps were requested"));
      nothingAvailable();
      return;
    }

  QJsonObject req_countries = loadJson(fullPath(const_fname_countries_requested));

  // get URLs
  QJsonObject provided = loadJson(fullPath(const_fname_countries_provided));
  QHash<QString, QString> url;
  if (provided.contains(const_feature_id_url))
    {
      const QJsonObject o = provided.value(const_feature_id_url).toObject();
      QString base = o.value("base").toString();
      for (QJsonObject::const_iterator i=o.constBegin(); i!=o.end(); ++i)
        if (i.key()!="base")
          url[i.key()] = base + "/" + i.value().toString();
    }

  // fill missing data
  m_missing_data.clear();

  for (QJsonObject::const_iterator request_iter = req_countries.constBegin();
       request_iter != req_countries.constEnd(); ++request_iter)
    {
      FilesToDownload missing;

      const QJsonObject request = request_iter.value().toObject();
      if (request.empty()) continue;

      if (m_feature_osmscout && request.contains(const_feature_name_osmscout))
        checkMissingOsmScout(request, url.value(const_feature_name_osmscout), missing);
      if (m_feature_geocoder_nlp && request.contains(const_feature_name_geocoder_nlp))
        checkMissingGeocoderNLP(request, url.value(const_feature_name_geocoder_nlp), missing);
      if (m_feature_postal_country && request.contains(const_feature_name_postal_country))
        checkMissingPostalCountry(request, url.value(const_feature_name_postal_country), missing);
      if (m_feature_postal_country && request.contains(const_feature_name_postal_global))
        checkMissingPostalGlobal(request, url.value(const_feature_name_postal_global), missing);

      if (missing.files.length() > 0)
        {
          missing.id = getId(request);
          missing.pretty = getPretty(request);
          m_missing_data.append(missing);
        }
    }

  if (m_missing_data.length() > 0)
    {
      for (const auto &m: m_missing_data)
        {
          InfoHub::logInfo(tr("Missing data: ") +
                           m.pretty + QString(" (%L1)").arg(m.tostore));
          for (const auto &i: m.files)
            InfoHub::logInfo(tr("To download: ") +
                             i.url + " -> " + i.path);
        }
    }
}


QString MapManager::fullPath(QString path) const
{
  if (path.isEmpty()) return QString();
  QDir dir(m_root_dir.absolutePath());
  return dir.filePath(path);
}

void MapManager::checkMissingFiles(const QJsonObject &request,
                                   const QString &feature,
                                   const QString &url,
                                   const QStringList &files,
                                   FilesToDownload &missing) const
{
  QString path = getPath(request, feature);
  bool added = false;

  for (const auto &f: files)
    if (!m_root_dir.exists(path + "/" + f))
      {
        added = true;
        FileTask t;
        t.path = fullPath(path + "/" + f);
        t.url = url + "/" + path + "/" + f;
        missing.files.append(t);
      }

  if (added)
    {
      // this is an upper limit of the sizes. its smaller in reality if
      // the feature is downloaded partially already
      missing.todownload += getSizeCompressed(request, feature);
      missing.tostore  += getSize(request, feature);
    }
}

////////////////////////////////////////////////////////////
/// support for downloads
bool MapManager::updateProvided()
{
  QMutexLocker lk(&m_mutex);
  return startDownload(m_provided_url,
                       fullPath(const_fname_countries_provided),
                       QString());
}

bool MapManager::getCountries()
{
  QMutexLocker lk(&m_mutex);

  if (m_missing_data.length() < 1) return true; // all has been downloaded already
  if (m_missing_data[0].files.length() < 1)
    {
      InfoHub::logError("Internal error: missing data has no files");
      return false;
    }

  bool started = startDownload( m_missing_data[0].files[0].url + ".bz2",
      m_missing_data[0].files[0].path, "BZ2" );

  if (started) m_downloading_countries = true;
  return started;
}

bool MapManager::downloading()
{
  QMutexLocker lk(&m_mutex);
  return m_file_downloader;
}

bool MapManager::startDownload(const QString &url, const QString &path, const QString &mode)
{
  // check if someone is downloading already
  if ( m_file_downloader ) return false;

  m_file_downloader = new FileDownloader(&m_network_manager, url, path, mode, this);
  if (!m_file_downloader)
    {
      InfoHub::logError("Failed to allocate FileDownloader"); // technical message, no need to translate
      return false;
    }

  if ( !bool(*m_file_downloader) )
    {
      InfoHub::logWarning(tr("Error starting the download of") + " " + path);
      return false;
    }

  connect(m_file_downloader.data(), &FileDownloader::finished, this, &MapManager::onDownloadFinished);
  connect(m_file_downloader.data(), &FileDownloader::error, this, &MapManager::onDownloadError);
  connect(m_file_downloader.data(), &FileDownloader::downloadedBytes, this, &MapManager::onDownloadedBytes);
  connect(m_file_downloader.data(), &FileDownloader::writtenBytes, this, &MapManager::onWrittenBytes);

  return true;
}

void MapManager::onDownloadFinished(QString path)
{
  QMutexLocker lk(&m_mutex);

  InfoHub::logInfo(tr("File downloaded:") + " " + path);
  cleanupDownload();

  if (m_downloading_countries)
    {
      if (m_missing_data.length() < 1 ||
          m_missing_data[0].files.length() < 1 ||
          m_missing_data[0].files[0].path != path)
        {
          lk.unlock();
          InfoHub::logError("Internal error: missing data has no files while one was downloaded or an unexpected file was downloaded");
          onDownloadError("Internal error: processing via error handling methods");
          return;
        }

      m_missing_data[0].files.pop_front();
      if (m_missing_data[0].files.length() == 0)
        {
          m_missing_data.pop_front();
          scanDirectories();
        }

      lk.unlock();
      getCountries();
    }
}

void MapManager::onDownloadError(QString err)
{
  QMutexLocker lk(&m_mutex);

  InfoHub::logWarning(err);
  cleanupDownload();

  InfoHub::logWarning(tr("Dropping all downloads"));
  m_missing_data.clear();
  m_downloading_countries = false;
}

void MapManager::cleanupDownload()
{
  if (m_file_downloader)
    {
      m_file_downloader->disconnect();
      m_file_downloader->deleteLater();
      m_file_downloader = QPointer<FileDownloader>();
    }
}

void MapManager::onDownloadedBytes(size_t sz)
{
  qDebug() << "D: " << sz;
}

void MapManager::onWrittenBytes(size_t sz)
{
  qDebug() << "W: " << sz;
}

////////////////////////////////////////////////////////////
/// SUPPORT FOR BACKENDS
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// libosmscout support
const static QStringList osmscout_files{
  "areaarea.idx", "areanode.idx", "areas.dat", "areasopt.dat", "areaway.idx", "bounding.dat",
  "intersections.dat", "intersections.idx", "location.idx", "nodes.dat", "router2.dat", "router.dat",
  "router.idx", "textloc.dat", "textother.dat", "textpoi.dat", "textregion.dat",
  "water.idx", "ways.dat", "waysopt.dat", "types.dat"};

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

void MapManager::checkMissingOsmScout(const QJsonObject &request, const QString &url, FilesToDownload &missing) const
{
  checkMissingFiles(request, const_feature_name_osmscout, url, osmscout_files, missing);
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

void MapManager::checkMissingGeocoderNLP(const QJsonObject &request, const QString &url, FilesToDownload &missing) const
{
  checkMissingFiles(request, const_feature_name_geocoder_nlp, url, geocodernlp_files, missing);
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

void MapManager::checkMissingPostalCountry(const QJsonObject &request, const QString &url, FilesToDownload &missing) const
{
  checkMissingFiles(request, const_feature_name_postal_country, url, postal_country_files, missing);
}

void MapManager::checkMissingPostalGlobal(const QJsonObject &request, const QString &url, FilesToDownload &missing) const
{
  checkMissingFiles(request, const_feature_name_postal_global, url, postal_global_files, missing);
}
