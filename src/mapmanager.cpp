#include "mapmanager.h"

#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QMutexLocker>

#include <QDirIterator>
#include <QDir>
#include <QFileInfo>

#include <QFile>
#include <QBitArray>
#include <QPair>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>

#include <QDebug>

using namespace MapManager;

Manager::Manager(QObject *parent) : QObject(parent)
{
  m_features.append(new FeatureOsmScout(this));
  m_features.append(new FeatureGeocoderNLP(this));
  m_features.append(new FeaturePostalGlobal(this));
  m_features.append(new FeaturePostalCountry(this));

  for (Feature *p: m_features)
    if (p == nullptr)
      {
        InfoHub::logError(tr("Could not allocate Map Manager features"));
        m_features.clear();
        return;
      }

  loadSettings();
}


Manager::~Manager()
{
  for (Feature *p: m_features)
    if (p != nullptr)
      delete p;
}

void Manager::onSettingsChanged()
{
  QMutexLocker lk(&m_mutex);
  loadSettings();
}

void Manager::loadSettings()
{
  AppSettings settings;

  m_root_dir.setPath(settings.valueString(MAPMANAGER_SETTINGS "root"));
  m_map_selected = settings.valueString(MAPMANAGER_SETTINGS "map_selected");
  m_provided_url = settings.valueString(MAPMANAGER_SETTINGS "provided_url");

  for (Feature *p: m_features) p->loadSettings();

  if (settings.valueBool(MAPMANAGER_SETTINGS "postal_country"))
    addCountryNoLock(const_feature_id_postal_global);
  else
    rmCountryNoLock(const_feature_id_postal_global);

  scanDirectories();
  missingData();
  checkUpdates();
}

void Manager::nothingAvailable()
{
  m_maps_available = QJsonObject();
  m_map_selected.clear();

  updateOsmScout();
  updateGeocoderNLP();
  updatePostal();
}

QJsonObject Manager::loadJson(QString fname) const
{
  QFile freq(fname);
  if (!freq.open(QIODevice::ReadOnly | QIODevice::Text)) return QJsonObject();
  return QJsonDocument::fromJson(freq.readAll()).object();
}

QString Manager::getId(const QJsonObject &obj) const
{
  return obj.value("id").toString();
}

QString Manager::getType(const QJsonObject &obj) const
{
  return obj.value("type").toString();
}

QString Manager::getPretty(const QJsonObject &obj) const
{
  if (obj.value("id").toString() == const_feature_id_postal_global)
    return tr("Address parsing language support");

  return obj.value("continent").toString() + " / " + obj.value("name").toString();
}

void Manager::scanDirectories()
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
  QJsonObject available;

  // check for global features that are required
  for (QJsonObject::const_iterator request_iter = req_countries.constBegin();
       request_iter != req_countries.constEnd(); ++request_iter)
    {
      const QJsonObject request = request_iter.value().toObject();
      if (request.empty()) continue;

      if (getType(request) != const_feature_type_country)
        {
          for (const Feature *f: m_features)
            if (!f->isAvailable(request))
              {
                InfoHub::logWarning(tr("No maps loaded: %1").arg(f->errorMissing()));
                nothingAvailable();
                return;
              }

          available.insert(request_iter.key(), request);
        }
    }

  // check for available countries
  size_t countries_added = 0;
  for (QJsonObject::const_iterator request_iter = req_countries.constBegin();
       request_iter != req_countries.constEnd(); ++request_iter)
    {
      const QJsonObject request = request_iter.value().toObject();
      if (request.empty()) continue;

      // check if we have all keys defined
      if (getType(request) == const_feature_type_country &&
          request.contains("id") &&
          request.contains("name") &&
          request.contains("continent") )
        {
          bool add = true;
          for (const Feature *f: m_features)
            if (!f->isAvailable(request))
              {
                InfoHub::logWarning(tr("Missing dataset for %1: %2").
                                    arg(getPretty(request)).
                                    arg(f->errorMissing()));
                add = false;
              }

          if (add)
            {
              available.insert(request_iter.key(), request);
              countries_added++;
            }
        }
    }

  if ( available != m_maps_available )
    {
      m_maps_available = available;

      AppSettings settings;
      m_map_selected = settings.valueString(MAPMANAGER_SETTINGS "map_selected");

      if (!m_maps_available.contains(m_map_selected))
        m_map_selected.clear();

      if ( countries_added == 1 || m_map_selected.isEmpty() ) // there is only one map or nothing is selected, let's select one
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
      QList<qint64> szs;
      makeCountriesList(true, countries, ids, szs);

      // print all loaded countries
      for (const auto &c: countries)
        InfoHub::logInfo(tr("Available country or territory: ") + c);

      updateOsmScout();
      updateGeocoderNLP();
      updatePostal();
    }
}

QString Manager::getInstalledCountries()
{
  QMutexLocker lk(&m_mutex);
  return makeCountriesListAsJSON(true);
}

QString Manager::getProvidedCountries()
{
  QMutexLocker lk(&m_mutex);
  return makeCountriesListAsJSON(false);
}

void Manager::makeCountriesList(bool list_available, QStringList &countries, QStringList &ids, QList<qint64> &sz)
{
  QList< QPair<QString, QString> > available;

  QJsonObject objlist;
  QHash<QString, size_t> sizes;

  if (list_available) objlist = m_maps_available;
  else objlist = loadJson(fullPath(const_fname_countries_provided));

  for (QJsonObject::const_iterator i = objlist.constBegin();
       i != objlist.constEnd(); ++i )
    {
      const QJsonObject c = i.value().toObject();
      if ( getType(c) == const_feature_type_country )
        {
          QString id = getId(c);

          available.append(qMakePair(getPretty(c), id));

          size_t s = 0;
          for (const Feature *f: m_features)
            s += f->getSize(c);

          sizes[id] = s;
        }
    }

  std::sort(available.begin(), available.end());

  countries.clear();
  ids.clear();
  sz.clear();
  for (const auto &i: available)
    {
      countries.append(i.first);
      ids.append(i.second);
      sz.append(sizes[i.second]);
    }
}

QString Manager::makeCountriesListAsJSON(bool list_available)
{
  QStringList countries;
  QStringList ids;
  QList<qint64> sz;

  makeCountriesList(list_available, countries, ids, sz);

  QJsonArray arr;
  for (int i = 0; i < ids.size(); ++i)
    {
      QJsonObject obj;
      obj.insert("name", countries[i]);
      obj.insert("id", ids[i]);
      obj.insert("size", sz[i]);
      arr.append(obj);
    }

  QJsonDocument doc(arr);
  return doc.toJson();
}

void Manager::addCountry(QString id)
{
  QMutexLocker lk(&m_mutex);
  addCountryNoLock(id);
}

void Manager::addCountryNoLock(QString id)
{
  if (!m_maps_available.contains(id) && m_root_dir.exists() && m_root_dir.exists(const_fname_countries_provided))
    {
      QJsonObject possible = loadJson(fullPath(const_fname_countries_provided));
      QJsonObject requested = loadJson(fullPath(const_fname_countries_requested));

      if (possible.contains(id) && possible.value(id).toObject().value("id") == id)
        {
          InfoHub::logInfo(tr("Add country or feature to requested list: ") + getPretty(possible.value(id).toObject()));

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

void Manager::rmCountry(QString id)
{
  QMutexLocker lk(&m_mutex);
  rmCountryNoLock(id);
}

void Manager::rmCountryNoLock(QString id)
{
  if ( (m_maps_available.contains(id) || m_maps_available.empty()) &&
       m_root_dir.exists() && m_root_dir.exists(const_fname_countries_requested) )
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

void Manager::missingData()
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
  for (Feature *f: m_features)
    f->setUrl(provided);

  // fill missing data
  m_missing_data.clear();

  for (QJsonObject::const_iterator request_iter = req_countries.constBegin();
       request_iter != req_countries.constEnd(); ++request_iter)
    {
      FilesToDownload missing;

      const QJsonObject request = request_iter.value().toObject();
      if (request.empty()) continue;

      for (const Feature *f: m_features)
        f->checkMissingFiles(request, missing);

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
        }
    }
}

QString Manager::fullPath(const QString &path) const
{
  if (path.isEmpty()) return QString();
  QDir dir(m_root_dir.canonicalPath());
  return dir.filePath(path);
}


////////////////////////////////////////////////////////////
/// support for downloads

bool Manager::getCountries()
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

  if (started) m_download_type = Countries;
  return started;
}

bool Manager::downloading()
{
  QMutexLocker lk(&m_mutex);
  return m_file_downloader;
}

bool Manager::startDownload(const QString &url, const QString &path, const QString &mode)
{
  // check if someone is downloading already
  if ( m_file_downloader ) return false;

  m_last_reported_downloaded = 0;
  m_last_reported_written = 0;

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

  connect(m_file_downloader.data(), &FileDownloader::finished, this, &Manager::onDownloadFinished);
  connect(m_file_downloader.data(), &FileDownloader::error, this, &Manager::onDownloadError);
  connect(m_file_downloader.data(), &FileDownloader::downloadedBytes, this, &Manager::onDownloadedBytes);
  connect(m_file_downloader.data(), &FileDownloader::writtenBytes, this, &Manager::onWrittenBytes);

  return true;
}

void Manager::onDownloadFinished(QString path)
{
  QMutexLocker lk(&m_mutex);

  InfoHub::logInfo(tr("File downloaded:") + " " + path);
  cleanupDownload();

  if (m_download_type == Countries)
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
      m_missing_data[0].todownload -= m_last_reported_downloaded;
      m_missing_data[0].tostore -= m_last_reported_written;
      if (m_missing_data[0].files.length() == 0)
        {
          m_missing_data.pop_front();
          scanDirectories();
        }

      m_download_type = NotKnown;
      lk.unlock();
      getCountries();
    }
  else if (m_download_type == ProvidedList)
    {
      m_download_type = NotKnown;
      checkUpdates();
    }
  else
    m_download_type = NotKnown;
}

void Manager::onDownloadError(QString err)
{
  QMutexLocker lk(&m_mutex);

  InfoHub::logWarning(err);
  cleanupDownload();

  InfoHub::logWarning(tr("Dropping all downloads"));
  m_missing_data.clear();
  m_download_type = NotKnown;
}

void Manager::cleanupDownload()
{
  if (m_file_downloader)
    {
      m_file_downloader->disconnect();
      m_file_downloader->deleteLater();
      m_file_downloader = QPointer<FileDownloader>();
    }
}

void Manager::onDownloadProgress()
{
  static QString last_message;

  QString txt;

  if ( m_download_type == ProvidedList )
    txt = QString(tr("List of provided countries and features: %L1 (D) / %L2 (W) MB")).
        arg(m_last_reported_downloaded/1024/1024).
        arg(m_last_reported_written/1024/1024);

  else if (m_download_type == Countries )
    {
      if (m_missing_data.length() > 0)
        {
          txt = QString(tr("%1: %L2 (D) / %L3 (W) MB")).
              arg(m_missing_data[0].pretty).
              arg((m_missing_data[0].todownload - m_last_reported_downloaded)/1024/1024).
              arg((m_missing_data[0].tostore - m_last_reported_written)/1024/1024);
        }
    }
  else
    txt = QString("Unknown: %L1 (D) %L2 (W) MB").
        arg(m_last_reported_downloaded/1024/1024).
        arg(m_last_reported_written/1024/1024);

  if (txt != last_message )
    {
      last_message = txt;
      emit downloadProgress(txt);

      qDebug() << txt;
    }
}

void Manager::onDownloadedBytes(size_t sz)
{
  m_last_reported_downloaded = sz;
  onDownloadProgress();
}

void Manager::onWrittenBytes(size_t sz)
{
  m_last_reported_written = sz;
  onDownloadProgress();
}

////////////////////////////////////////////////////////////
/// support for cleanup

qint64 Manager::getNonNeededFilesList(QStringList &files)
{
  QMutexLocker lk(&m_mutex);
  qint64 notNeededSize = 0;

  m_not_needed_files.clear();
  files.clear();

  // this is mutex protected as well
  lk.unlock();
  if (downloading()) return false;
  lk.relock();

  // fill up needed files
  QSet<QString> wanted;
  wanted.insert(fullPath(const_fname_countries_requested));
  wanted.insert(fullPath(const_fname_countries_provided));

  QJsonObject req_countries = loadJson(fullPath(const_fname_countries_requested));
  for (QJsonObject::const_iterator request_iter = req_countries.constBegin();
       request_iter != req_countries.constEnd(); ++request_iter)
    {
      const QJsonObject request = request_iter.value().toObject();
      if (request.empty()) continue;

      for (const Feature *f: m_features)
        f->fillWantedFiles(request, wanted);
    }

  QDir dir(QDir::cleanPath(fullPath(".")));
  dir.setFilter(QDir::Files);
  QDirIterator dirIter( dir, QDirIterator::Subdirectories);
  while (dirIter.hasNext())
    {
      QString path = dirIter.next();
      QFileInfo fi(path);

      if (!fi.isFile()) continue;

      if ( !wanted.contains(path) )
        {
          notNeededSize += fi.size();
          files.append(fullPath(path));
        }
    }

  m_not_needed_files = files;

  return notNeededSize;
}

bool Manager::deleteNonNeededFiles(const QStringList &files)
{
  QMutexLocker lk(&m_mutex);
  if ( files != m_not_needed_files )
    {
      InfoHub::logError("Internal error: list of files given to delete does not matched with an expected one");
      m_not_needed_files.clear();
      return false;
    }

  for (auto fname: m_not_needed_files)
    {
      if ( !m_root_dir.remove(fname) )
        {
          InfoHub::logWarning(tr("Error while deleting file:") + " " + fname);
          InfoHub::logWarning(tr("Cancelling the removal of remaining files."));
          m_not_needed_files.clear();
          return false;
        }

      InfoHub::logInfo(tr("File removed during cleanup:") + " " + fname);
    }

  m_not_needed_files.clear();
  return true;
}

////////////////////////////////////////////////////////////
/// support for updates

bool Manager::updateProvided()
{
  QMutexLocker lk(&m_mutex);
  bool started = startDownload(m_provided_url,
                               fullPath(const_fname_countries_provided),
                               QString());
  if (started) m_download_type = ProvidedList;
  return started;
}

void Manager::checkUpdates()
{
  m_last_found_updates = QJsonObject();

  if ( m_root_dir.exists() &&
       m_root_dir.exists(const_fname_countries_requested) &&
       m_root_dir.exists(const_fname_countries_provided) )
    {
      QJsonObject possible_list = loadJson(fullPath(const_fname_countries_provided));
      QJsonObject requested_list = loadJson(fullPath(const_fname_countries_requested));

      for (QJsonObject::const_iterator request_iter = requested_list.constBegin();
           request_iter != requested_list.constEnd(); ++request_iter)
        {
          QJsonObject update;

          const QJsonObject request = request_iter.value().toObject();
          if (request.empty()) continue;

          const QJsonObject possible = possible_list.value(request_iter.key()).toObject();
          if (possible.empty()) continue;

          for (const Feature *f: m_features)
            if ( f->hasFeatureDefined(possible) &&
                 (!f->hasFeatureDefined(request) ||
                  f->getDateTime(request) < f->getDateTime(possible)) )
              update.insert(f->name(), possible.value(f->name()).toObject());

          if (!update.empty())
            {
              update.insert("id", request_iter.key());
              update.insert("pretty", getPretty(possible) );
              update.insert("type", possible.value("type").toString());

              m_last_found_updates.insert(request_iter.key(), update);
            }
        }
    }
  else
    InfoHub::logWarning(tr("Cannot check for updates due to missing directory or files"));

  QJsonDocument doc(m_last_found_updates);
  emit updatesFound(doc.toJson());
  qDebug() << doc.toJson().constData();
}

QString Manager::updatesFound()
{
  QMutexLocker lk(&m_mutex);
  return QJsonDocument(m_last_found_updates).toJson();
}

void Manager::getUpdates()
{
  QMutexLocker lk(&m_mutex);

  QJsonObject requested = loadJson(fullPath(const_fname_countries_requested));

  for (QJsonObject::const_iterator iter = m_last_found_updates.constBegin();
       iter != m_last_found_updates.constEnd(); ++iter)
    {
      if (!requested.contains(iter.key())) continue;

      const QJsonObject update = iter.value().toObject();
      QJsonObject requpdated = requested.value(iter.key()).toObject();

      for (Feature *f: m_features)
        if ( f->hasFeatureDefined(update) )
          {
            f->deleteFiles(update);
            requpdated.insert( f->name(), update.value(f->name()).toObject() );
          }

      requested.insert(iter.key(), requpdated);
    }

  { // write updated requested json to a file
    QJsonDocument doc(requested);
    QFile file(fullPath(const_fname_countries_requested));
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write( doc.toJson() );
  }

  scanDirectories();
  missingData();

  lk.unlock();
  getCountries();
}

////////////////////////////////////////////////////////////
/// libosmscout support
void Manager::updateOsmScout()
{
  AppSettings settings;

  QString path;
  QJsonObject obj = m_maps_available.value(m_map_selected).toObject();
  for (const Feature *f: m_features)
    if (f->enabled() && f->name() == "osmscout")
      path = fullPath( f->getPath(obj) );

  if (settings.valueString(OSM_SETTINGS "map") != path)
    {
      settings.setValue(OSM_SETTINGS "map", path);
      emit databaseOsmScoutChanged(path);
    }
}

////////////////////////////////////////////////////////////
/// Geocoder NLP support
void Manager::updateGeocoderNLP()
{
  AppSettings settings;

  QString path;

  // version of the geocoder where all data is in a single file
#pragma message "This would have to change when GeocoderNLP format would change to the directory-based one"
  QJsonObject obj = m_maps_available.value(m_map_selected).toObject();
  for (const Feature *f: m_features)
    if (f->enabled() && f->name() == "geocoder_nlp")
      path = fullPath( f->getPath(obj) + "/location.sqlite" );

  if (settings.valueString(GEOMASTER_SETTINGS "geocoder_path") != path)
    {
      settings.setValue(GEOMASTER_SETTINGS "geocoder_path", path);
      emit databaseGeocoderNLPChanged(path);
    }
}

////////////////////////////////////////////////////////////
/// libpostal support
void Manager::updatePostal()
{
  AppSettings settings;

  QString path_global;
  QString path_country;

  QJsonObject obj_global = m_maps_available.value(const_feature_id_postal_global).toObject();
  for (const Feature *f: m_features)
    if (f->enabled() && f->name() == "postal_global")
      path_global = fullPath( f->getPath(obj_global) );

  QJsonObject obj_country = m_maps_available.value(m_map_selected).toObject();
  for (const Feature *f: m_features)
    if (f->enabled() && f->name() == "postal_country")
      path_country = fullPath( f->getPath(obj_country) );

  if (settings.valueString(GEOMASTER_SETTINGS "postal_main_dir") != path_global ||
      settings.valueString(GEOMASTER_SETTINGS "postal_country_dir") != path_country )
    {
      settings.setValue(GEOMASTER_SETTINGS "postal_main_dir", path_global);
      settings.setValue(GEOMASTER_SETTINGS "postal_country_dir", path_country);
      emit databasePostalChanged(path_global, path_country);
    }
}
