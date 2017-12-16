#include "mapmanager.h"

#include "appsettings.h"
#include "config.h"
#include "infohub.h"
#include "mapmanager_deleterthread.h"

#include <QDirIterator>
#include <QDir>
#include <QFileInfo>

#include <QFile>
#include <QBitArray>
#include <QPair>
#include <QThread>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <QLocale>

#include <algorithm>
#include <cmath>
#include <iostream>

#include <QDebug>

#define DELAY_BEFORE_SIGNAL 500

using namespace MapManager;

Manager::Manager(QObject *parent) : QObject(parent)
{
  m_features.append(new FeatureOsmScout(this));
  m_features.append(new FeatureGeocoderNLP(this));
  m_features.append(new FeaturePostalGlobal(this));
  m_features.append(new FeaturePostalCountry(this));
  m_features.append(new FeatureMapboxGLGlobal(this));
  m_features.append(new FeatureMapboxGLGlyphs(this));
  m_features.append(new FeatureMapboxGLCountry(this));
  m_features.append(new FeatureMapnikGlobal(this));
  m_features.append(new FeatureMapnikCountry(this));
  m_features.append(new FeatureValhalla(this));

  for (Feature *p: m_features)
    if (p == nullptr)
      {
        InfoHub::logError(tr("Could not allocate Map Manager features"));
        m_features.clear();
        return;
      }

  for (Feature *p: m_features)
    connect(p, &Feature::availabilityChanged,
            this, &Manager::onAvailabilityChanged);

  connect(this, &Manager::downloadingChanged,
          this, &Manager::checkIfReady);

  connect(this, &Manager::deletingChanged,
          this, &Manager::checkIfReady);

  checkIfReady();
}


Manager::~Manager()
{
  for (Feature *p: m_features)
    if (p != nullptr)
      delete p;
}

void Manager::onSettingsChanged()
{
  loadSettings();
}

void Manager::loadSettings()
{
  AppSettings settings;

  bool storage_was_available = m_storage_available;
  QString old_selection = m_map_selected;

  QString root_dir_path = settings.valueString(MAPMANAGER_SETTINGS "root");
  bool root_changed = ( root_dir_path != m_root_dir.path() );
  m_root_dir.setPath(root_dir_path);

  if (root_changed)
    InfoHub::logInfo(tr("Storage folder changed to %1").arg(root_dir_path));

  // check for errors
  bool root_ok = false;
  {
    QFileInfo rinfo(m_root_dir.absolutePath());
    if (!rinfo.exists())
      InfoHub::logWarning(tr("Maps storage folder does not exist: %1").arg(rinfo.absoluteFilePath()));
    else if (!rinfo.isDir())
      {
        QString err = tr("Maps storage folder path does not point to a directory: %1").arg(rinfo.absoluteFilePath());
        InfoHub::logWarning(err);
        emit errorMessage(err);
      }
    else if (!rinfo.isWritable())
      {
        QString err = tr("Maps storage folder is not writable, please adjust permissions for %1").arg(rinfo.absoluteFilePath());
        InfoHub::logWarning(err);
        emit errorMessage(err);
      }
    else
      root_ok = true;
  }

  m_map_selected = settings.valueString(MAPMANAGER_SETTINGS "map_selected");
  m_provided_url = settings.valueString(MAPMANAGER_SETTINGS "provided_url");
  m_development_disable_url_update = settings.valueBool(MAPMANAGER_SETTINGS "development_disable_url_update");

  for (Feature *p: m_features) p->loadSettings();

  if (settings.valueBool(MAPMANAGER_SETTINGS "postal_country"))
    addCountry(const_feature_id_postal_global);
  else
    rmCountry(const_feature_id_postal_global);

  if (settings.valueBool(MAPMANAGER_SETTINGS "mapnik"))
    addCountry(const_feature_id_mapnik_global);
  else
    rmCountry(const_feature_id_mapnik_global);

  if (settings.valueBool(MAPMANAGER_SETTINGS "mapboxgl"))
    {
      addCountry(const_feature_id_mapboxgl_global);
      addCountry(const_feature_id_mapboxgl_glyphs);
    }
  else
    {
      rmCountry(const_feature_id_mapboxgl_global);
      rmCountry(const_feature_id_mapboxgl_glyphs);
    }

  if ( m_db_files.isOpen() && m_db_files.databaseName() != fullPath(const_fname_db_files) )
    {
      m_query_files_available.clear();
      m_query_files_insert.clear();

      m_db_files.close();
      QSqlDatabase::removeDatabase(const_db_connection);
    }

  if ( root_ok &&
       (!m_db_files.isOpen()) )
    {
      // open new connection and prepare database
      m_db_files = QSqlDatabase::addDatabase("QSQLITE", const_db_connection);
      m_db_files.setDatabaseName(fullPath(const_fname_db_files));
      if (!m_db_files.open())
        {
          QSqlError error = m_db_files.lastError();
          InfoHub::logWarning(tr("Failed to open the database for tracking downloaded files") + ": " +
                              error.text());
          emit errorMessage(tr("Failed to open the database for tracking downloaded files") + "<br><br>" +
                            tr("Map Manager functionality would be disturbed") + "<br><br>" +
                            error.text());
        }
      else // all is fine, prepare queries and tables
        {
          m_db_files.exec("CREATE TABLE IF NOT EXISTS files (name TEXT PRIMARY KEY, version TEXT, datetime TEXT)");
          m_query_files_available = QSqlQuery(m_db_files);
          m_query_files_available.setForwardOnly(true);
          m_query_files_insert = QSqlQuery(m_db_files);

          m_query_files_available.prepare("SELECT name, version, datetime FROM files WHERE (name=:name)");
          m_query_files_insert.prepare("INSERT OR REPLACE INTO files (name, version, datetime) VALUES(:name, :version, :datetime)");
        }
    }

  m_storage_available = isStorageAvailable();
  if (storage_was_available != m_storage_available)
    emit storageAvailableChanged(m_storage_available);

  loadUrls();
  scanDirectories(root_changed || old_selection != m_map_selected);
  missingData();
  checkUpdates();
  if (old_selection != m_map_selected)
    emit selectedMapChanged(m_map_selected);
}

bool Manager::ready()
{
  return (!downloading() && !deleting());
}

void Manager::checkIfReady()
{
  bool oldr = m_ready;
  m_ready = ready();
  if (oldr != m_ready)
    emit readyChanged(m_ready);
}


bool Manager::isStorageAvailable() const
{
  QFileInfo rinfo(m_root_dir.absolutePath());
  return (rinfo.exists() && rinfo.isDir() && rinfo.isWritable() && m_db_files.isOpen());
}

QString Manager::defaultStorageDirectory() const
{
  QString d = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  if (!d.isEmpty())
    {
      QDir dir(d);
      return dir.absoluteFilePath("Maps.OSM");
    }
  return d;
}

bool Manager::createDirectory(QString path)
{
  if (path.isEmpty())
    {
      emit errorMessage(tr("Cannot create directory without any name"));
      return false;
    }

  QDir d(path);
  bool res = d.mkpath(".");
  if (!res)
    emit errorMessage(tr("Error creating directory %1").arg(path));
  return res;
}

QString Manager::selectedMap()
{
  return m_map_selected;
}

void Manager::nothingAvailable()
{
  m_maps_available = QJsonObject();
  m_map_selected.clear();
  m_missing = false;

  updateOsmScout();
  updateGeocoderNLP();
  updatePostal();
  updateMapnik();
  updateMapboxGL();
  updateValhalla();

  emit availabilityChanged();
  emit missingChanged(m_missing);
  emit selectedMapChanged(m_map_selected);
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
  else if (obj.value("id").toString() == const_feature_id_mapnik_global)
    return tr("Mapnik World coastlines");
  else if (obj.value("id").toString() == const_feature_id_mapboxgl_global)
    return tr("Mapbox GL World overlay");
  else if (obj.value("id").toString() == const_feature_id_mapboxgl_glyphs)
    return tr("Mapbox GL fonts");

  QString name = obj.value("name").toString();
  name.replace("/", const_pretty_separator);
  return name;
}

bool Manager::storageAvailable()
{
  return m_storage_available;
}

void Manager::checkStorageAvailable()
{
  m_storage_available = isStorageAvailable();
  emit storageAvailableChanged(m_storage_available);
}

void Manager::onAvailabilityChanged()
{
  scanDirectories();
}

void Manager::scanDirectories(bool force_update)
{
  if (!m_root_dir.exists())
    {
      InfoHub::logWarning(tr("Maps storage folder does not exist: ") + m_root_dir.absolutePath());
      m_maps_requested = QJsonObject();
      nothingAvailable();
      return;
    }

  // load list of requested countries and features
  if (!m_root_dir.exists(const_fname_countries_requested))
    {
      InfoHub::logWarning(tr("No maps were requested"));
      m_maps_requested = QJsonObject();
      nothingAvailable();
      return;
    }

  m_maps_requested = loadJson(fullPath(const_fname_countries_requested));
  QJsonObject available;

  // check for global features that are required
  for (QJsonObject::const_iterator request_iter = m_maps_requested.constBegin();
       request_iter != m_maps_requested.constEnd(); ++request_iter)
    {
      const QJsonObject request = request_iter.value().toObject();
      if (request.empty()) continue;

      if (getType(request) != const_feature_type_country)
        {
          for (Feature *f: m_features)
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
  for (QJsonObject::const_iterator request_iter = m_maps_requested.constBegin();
       request_iter != m_maps_requested.constEnd(); ++request_iter)
    {
      const QJsonObject request = request_iter.value().toObject();
      if (request.empty()) continue;

      // check if we have all keys defined
      if (getType(request) == const_feature_type_country &&
          request.contains("id") &&
          request.contains("name") )
        {
          bool add = true;
          for (Feature *f: m_features)
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

  if ( available != m_maps_available || force_update )
    {
      bool chatty_update = (available != m_maps_available);
      m_maps_available = available;

      AppSettings settings;
      m_map_selected = settings.valueString(MAPMANAGER_SETTINGS "map_selected");

      if (!m_maps_available.contains(m_map_selected))
        m_map_selected.clear();

      if ( countries_added == 1 || m_map_selected.isEmpty() ) // there is only one map or nothing is selected, let's select one
        {
          for (QJsonObject::const_iterator i = m_maps_available.constBegin();
               i != m_maps_available.constEnd(); ++i)
            if (i.key() != const_feature_id_postal_global &&
                i.key() != const_feature_id_mapnik_global &&
                i.key() != const_feature_id_mapboxgl_global &&
                i.key() != const_feature_id_mapboxgl_glyphs)
              {
                m_map_selected = i.key();
                break;
              }
        }

      QStringList countries, ids;
      QList<uint64_t> szs;
      makeCountriesList(ListAvailable, countries, ids, szs);

      // print all loaded countries
      if (chatty_update)
        for (const auto &c: countries)
          InfoHub::logInfo(tr("Available country or territory: ") + c);

      updateOsmScout();
      updateGeocoderNLP();
      updatePostal();
      updateMapnik();
      updateMapboxGL();
      updateValhalla();

      emit availabilityChanged();
      emit selectedMapChanged(m_map_selected);
    }
}

QString Manager::getAvailableCountries()
{
  QList< QPair<QString, QString> > available;
  for (QJsonObject::const_iterator i = m_maps_available.constBegin();
       i != m_maps_available.constEnd(); ++i )
    {
      const QJsonObject c = i.value().toObject();
      QString id = getId(c);

      if ( getType(c) == const_feature_type_country )
        available.append(qMakePair(getPretty(c), id));
    }

  std::sort(available.begin(), available.end());
  QJsonArray list;
  int currentIndex = -1;
  int counter = 0;
  for (const auto &i: available)
    {
      QJsonObject c;
      c.insert("name", i.first);
      c.insert("id", i.second);
      list.append(c);

      if (i.second == m_map_selected) currentIndex = counter;
      ++counter;
    }

  QJsonObject ret;
  ret.insert("countries", list);
  ret.insert("current", currentIndex);
  QJsonDocument doc(ret);
  return doc.toJson();
}

QString Manager::getRequestedCountries()
{
  return makeCountriesListAsJSON(ListRequested, false);
}

QString Manager::getProvidedCountries()
{
  return makeCountriesListAsJSON(ListProvided, true);
}

void Manager::makeCountriesList(ListType list_type, QStringList &countries, QStringList &ids, QList<uint64_t> &sz)
{
  QList< QPair<QString, QString> > available_global;
  QList< QPair<QString, QString> > available;

  QJsonObject objlist;
  QHash<QString, uint64_t> sizes;

  if (list_type == ListAvailable) objlist = m_maps_available;
  else if (list_type == ListRequested) objlist = m_maps_requested;
  else objlist = loadJson(fullPath(const_fname_countries_provided));

  for (QJsonObject::const_iterator i = objlist.constBegin();
       i != objlist.constEnd(); ++i )
    {
      const QJsonObject c = i.value().toObject();
      QString id = getId(c);

      if ( getType(c) == const_feature_type_country )
        available.append(qMakePair(getPretty(c), id));
      else if (list_type == ListAvailable)
        available_global.append(qMakePair(getPretty(c), id));

      uint64_t s = 0;
      for (const Feature *f: m_features)
        s += f->getSize(c);

      sizes[id] = s;
    }

  std::sort(available.begin(), available.end());
  std::sort(available_global.begin(), available_global.end());

  countries.clear();
  ids.clear();
  sz.clear();

  for (const auto &i: available_global)
    {
      countries.append(i.first);
      ids.append(i.second);
      sz.append(sizes[i.second]);
    }

  for (const auto &i: available)
    {
      countries.append(i.first);
      ids.append(i.second);
      sz.append(sizes[i.second]);
    }
}

bool Manager::checkProvidedAvailable()
{
  QJsonObject objlist_prov = loadJson(fullPath(const_fname_countries_provided));
  return (!objlist_prov.empty() && !m_base_urls.isEmpty());
}

void Manager::loadUrls()
{
  QJsonObject obj = loadJson(fullPath(const_fname_server_url));
  m_base_urls.load(obj);
}

///////////////////////////////////////////////////////
/// support for composing hierarchy of countries
struct CountryBranch {
  QString name;
  bool isDir{false};
  QList< CountryBranch > dir;
  QJsonObject country;
};

static void insertCountry(const QJsonObject &country,
                          const QString &name,
                          QStringList path,
                          CountryBranch &target)
{
  if (path.isEmpty())
    {
      CountryBranch c;
      c.name = name;
      c.country = country;
      target.dir.append(c);
    }
  else
    {
      QString clevel = path.takeFirst();

      QString lname; if (!target.dir.isEmpty()) lname = target.dir.last().name;

      if (lname == clevel && !target.dir.last().isDir) // have to make new subdir and move country to it
        {
          CountryBranch cmove = target.dir.takeLast();
          CountryBranch c;
          c.name = clevel;
          c.isDir = true;
          c.dir.append(cmove);
          insertCountry(country, name, path, c);
          target.dir.push_back(c);
        }
      else if (lname == clevel) // have dir ready and can directly insert
        insertCountry (country, name, path, target.dir.last() );
      else
        {
          CountryBranch c;
          c.name = clevel;
          c.isDir = true;
          insertCountry(country, name, path, c);
          target.dir.push_back(c);
        }
    }
}

static QJsonObject makeList(const CountryBranch branch)
{
  QJsonObject dir;
  dir.insert("type", QString("dir"));
  dir.insert("name", branch.name);

  QJsonArray arr;
  for (const CountryBranch &i: branch.dir)
    {
      if (i.isDir)
        arr.append( makeList(i) );
      else
        arr.append( i.country );
    }
  dir.insert("children", arr);
  return dir;
}

QString Manager::makeCountriesListAsJSON(ListType list_type, bool tree)
{
  QStringList countries;
  QStringList ids;
  QList<uint64_t> sz;

  makeCountriesList(list_type, countries, ids, sz);

  CountryBranch root;
  root.isDir = true;
  root.name = tr("World");
  for (int i = 0; i < ids.size(); ++i)
    {
      QJsonObject obj;
      QString name;
      QStringList path;

      if (tree)
        {
          path = countries[i].split(const_pretty_separator);
          name = path.takeLast();
        }
      else
        name = countries[i];

      obj.insert("name", name);
      obj.insert("id", ids[i]);
      obj.insert("size", QString("%L1").arg( (int)round(sz[i]/1024./1024.) ) );
      obj.insert("type", QString("non-dir"));
      insertCountry(obj, name, path, root);
    }

  QJsonDocument doc(makeList(root));
  return doc.toJson();
}

void Manager::addCountry(QString id)
{
  if (!ready()) return;

  if (!m_maps_available.contains(id) && m_root_dir.exists() && m_root_dir.exists(const_fname_countries_provided))
    {
      QJsonObject possible = loadJson(fullPath(const_fname_countries_provided));
      QJsonObject requested = loadJson(fullPath(const_fname_countries_requested));

      if (possible.contains(id) && possible.value(id).toObject().value("id") == id)
        {
          InfoHub::logInfo(tr("Add country or feature to requested list") + ": " + getPretty(possible.value(id).toObject()));

          requested.insert(id, possible.value(id).toObject());

          QJsonDocument doc(requested);
          QFile file(fullPath(const_fname_countries_requested));
          file.open(QIODevice::WriteOnly | QIODevice::Text);
          file.write( doc.toJson() );
        }

      scanDirectories();
      missingData();
    }

  emit subscriptionChanged();
}

void Manager::rmCountry(QString id)
{
  if (!ready()) return;

  if ( m_root_dir.exists() && m_root_dir.exists(const_fname_countries_requested) )
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

  emit subscriptionChanged();
}

QString Manager::getCountryDetails(QString id)
{
  QJsonObject provided = loadJson(fullPath(const_fname_countries_provided)).value(id).toObject();
  QJsonObject requested = m_maps_requested.value(id).toObject();

  QJsonObject country;
  if (requested.empty()) country = provided;
  else country = requested;

  QJsonObject reply;

  if (country.empty())
    reply.insert("error", QString("Cannot find country with id " + id));
  else
    {
      QString name = getPretty(country);
      reply.insert("name_full", name);
      reply.insert("name", name.split(const_pretty_separator).last());
      uint64_t sz = 0;
      uint64_t sz_tot = 0;

      QJsonArray features;
      for (const Feature *f: m_features)
        {
          u_int64_t s = f->getSize(country, true);
          sz_tot += s;
          if (f->enabled()) sz += s;

          if (s > 0)
            {
              QJsonObject feature;
              feature.insert("name", f->pretty());
              feature.insert("enabled", f->enabled());
              feature.insert("size", QString("%L1").arg( (int)round(s/1024./1024.) ));
              feature.insert("date", QLocale::system().toString( f->getDateTime(country).date(), QLocale::ShortFormat ));
              feature.insert("compatible", QString::number( f->isCompatible(country) ? 1 : 0 ));
              features.append(feature);
            }
        }

      reply.insert("size", QString("%L1").arg( (int)round(sz/1024./1024.) ));
      reply.insert("size_total", QString("%L1").arg( (int)round(sz_tot/1024./1024.) ));
      reply.insert("features", features);
    }

  QJsonDocument doc(reply);
  return doc.toJson();
}

bool Manager::isCountryRequested(QString id)
{
  return m_maps_requested.contains(id);
}

bool Manager::isCountryAvailable(QString id)
{
  return m_maps_available.contains(id);
}

bool Manager::isCountryCompatible(QString id)
{
  if (isCountryAvailable(id)) return true;
  QJsonObject c = m_maps_requested.value(id).toObject();
  if (c.empty()) return true;

  for (const Feature *f: m_features)
    if (!f->isCompatible(c))
      return false;

  return true;
}

bool Manager::missing()
{
  return m_missing;
}

QString Manager::missingInfo()
{
  return m_missing_info;
}

void Manager::missingData()
{
  if (!m_root_dir.exists())
    {
      InfoHub::logWarning(tr("Maps storage folder does not exist: ") + m_root_dir.absolutePath());
      return;
    }

  // load list of requested countries and features
  if (!m_root_dir.exists(const_fname_countries_requested))
    {
      InfoHub::logWarning(tr("No maps were requested"));
      nothingAvailable();
      return;
    }

  QJsonObject req_countries = m_maps_requested;

  // get URL components for features
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

      for (Feature *f: m_features)
        if (f->isCompatible(request))
          f->checkMissingFiles(request, missing);
        else
          InfoHub::logWarning(tr("%1: version of dataset for %2 is not supported").
                              arg(f->pretty()).
                              arg(getPretty(request)));

      if (missing.files.length() > 0)
        {
          missing.id = getId(request);
          missing.pretty = getPretty(request);
          m_missing_data.append(missing);
        }
    }

  QString info;
  if (m_missing_data.length() > 0)
    {
      for (const auto &m: m_missing_data)
        {
          if (!info.isEmpty())
            info  = info + "<br>";
          info += m.pretty + QString(" (%L1)").arg(m.tostore);

          InfoHub::logInfo(tr("Missing data: ") +
                           m.pretty + QString(" (%L1)").arg(m.tostore));
        }
    }

  if (info != m_missing_info)
    {
      m_missing_info = info;
      emit missingInfoChanged(info);
    }

  if ( (m_missing_data.length()>0) != (m_missing>0) )
    {
      m_missing = (m_missing_data.length()>0);
      emit missingChanged( (m_missing_data.length()>0) );
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
  if (!ready()) return false;

  if (m_missing_data.length() < 1) return true; // all has been downloaded already
  if (m_missing_data[0].files.length() < 1)
    {
      InfoHub::logError("Internal error: missing data has no files");
      return false;
    }

  return
      startDownload( Countries, m_missing_data[0].files[0].url + ".bz2", m_missing_data[0].files[0].path, FileDownloader::BZ2 );
}

bool Manager::downloading()
{
  return (m_download_type != NoDownload);
}

bool Manager::isRegistered(const QString &path, QString &version, QString &datetime)
{
  if (!m_db_files.isOpen()) return false;

  m_query_files_available.bindValue(":name", path);
  if (!m_query_files_available.exec())
    return false;

  while (m_query_files_available.next())
    {
      // since name is unique it should be one response only
      QString name = m_query_files_available.value(0).toString();
      version = m_query_files_available.value(1).toString();
      datetime = m_query_files_available.value(2).toString();
      return true;
    }

  return false;
}

bool Manager::startDownload(DownloadType type, QString url, const QString &path, const FileDownloader::Type mode, bool fullpath)
{
  if (!m_root_dir.exists())
    {
      InfoHub::logWarning(tr("Maps storage folder does not exist: ") + m_root_dir.absolutePath());
      return false;
    }

  if (!fullpath && m_base_urls.isEmpty())
    {
      InfoHub::logWarning(tr("No servers found to download the data from"));
      return false;
    }

  // check if someone is downloading already
  if ( m_file_downloader ) return false;

  m_last_reported_downloaded = 0;
  m_last_reported_written = 0;
  m_download_url = url;
  m_download_path = path;
  m_download_filemode = mode;

  if (!fullpath) url = m_base_urls.url() + "/" + url;
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

  m_download_type = type;
  emit downloadingChanged(true);

  return true;
}

void Manager::onDownloadFinished(QString path)
{
  InfoHub::logInfo(tr("File downloaded:") + " " + path);
  cleanupDownload();

  const DownloadType dtype = m_download_type;

  if (dtype == Countries)
    {
      if (m_missing_data.length() < 1 ||
          m_missing_data[0].files.length() < 1 ||
          m_missing_data[0].files[0].path != path)
        {
          InfoHub::logError("Internal error: missing data has no files while one was downloaded or an unexpected file was downloaded");
          onDownloadError("Internal error: processing via error handling methods");
          return;
        }

      // register download in database
      bool registration_ok = false;
      if (m_db_files.isOpen())
        {
          m_query_files_insert.bindValue(":name", m_missing_data[0].files[0].relpath);
          m_query_files_insert.bindValue(":version", m_missing_data[0].files[0].version);
          m_query_files_insert.bindValue(":datetime", m_missing_data[0].files[0].datetime);
          registration_ok = m_query_files_insert.exec();
        }

      if (!registration_ok)
        {
          onDownloadError(tr("Could not register downloaded file in the tracking database"));
          InfoHub::logWarning(tr("File registration error") + ": " + m_query_files_insert.lastError().databaseText());
          return;
        }

      m_missing_data[0].files.pop_front();
      m_missing_data[0].todownload -= m_last_reported_downloaded;
      m_missing_data[0].tostore -= m_last_reported_written;
      if (m_missing_data[0].files.length() == 0)
        {
          m_missing_data.pop_front();
          scanDirectories();
          missingData();
        }

      m_download_type = NoDownload;
      getCountries();
    }
  else if (dtype == ServerUrl)
    {
      m_download_type = NoDownload;
      loadUrls();
      if (m_base_urls.isEmpty())
        {
          onDownloadError(tr("Could not retrieve server URL"));
          InfoHub::logWarning(tr("Could not retrieve server URL"));
          return;
        }

      if ( startDownload(ProvidedList, const_fname_countries_provided,
                         fullPath(const_fname_countries_provided),
                         FileDownloader::Plain) )
        {
          emit downloadProgress(tr("Downloading the list of countries"));
        }
    }
  else if (dtype == ProvidedList)
    {
      m_download_type = NoDownload;
      loadSettings(); // to ensure that global datasets are requested if needed
      checkUpdates();
    }
  else
    m_download_type = NoDownload;

  // check if we set it to NoDownload
  if (m_download_type == NoDownload)
    emit downloadingChanged(false);
}

void Manager::onDownloadError(QString err)
{
  cleanupDownload();

  if (m_download_type != ServerUrl)
    {
      // try to download from the next server if possible
      InfoHub::logInfo(tr("Download failed from %1").arg(m_base_urls.url()));

      if (m_base_urls.next() &&
          startDownload( m_download_type,
                         m_download_url, m_download_path, m_download_filemode ) )
        {
          // new download started from the next server, no need to propagate the error
          return;
        }
    }

  InfoHub::logWarning(tr("Download failed, dropping all downloads"));
  emit errorMessage(err);

  m_download_type = NoDownload;
  emit downloadingChanged(false);
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
  const DownloadType dtype = m_download_type;

  if ( dtype == ProvidedList )
    txt = QString(tr("List of countries: %L1 (D) / %L2 (W) MB")).
        arg(m_last_reported_downloaded/1024/1024).
        arg(m_last_reported_written/1024/1024);
  else if ( dtype == ServerUrl )
    txt = QString(tr("List of countries: %L1 (D) / %L2 (W)")).
        arg(m_last_reported_downloaded/1024/1024).
        arg(m_last_reported_written/1024/1024);
  else if (dtype == Countries )
    {
      if (m_missing_data.length() > 0)
        {
          txt = QString(tr("%1: %L2 (D) / %L3 (W) MB")).
              arg(m_missing_data[0].pretty).
              arg((m_missing_data[0].todownload - m_last_reported_downloaded)/1024.0/1024.0, 0, 'f', 1).
              arg((m_missing_data[0].tostore - m_last_reported_written)/1024.0/1024.0, 0, 'f', 1);
        }
    }
  else
    txt = QString("Unknown: %L1 (D) %L2 (W) MB").
        arg(m_last_reported_downloaded/1024.0/1024.0, 0, 'f', 1).
        arg(m_last_reported_written/1024.0/1024.0, 0, 'f', 1);

  if (txt != last_message )
    {
      last_message = txt;
      emit downloadProgress(txt);

#ifdef IS_CONSOLE_QT
      std::cout << "Download progress: " << txt.toStdString() << std::endl;
#endif
    }
}

void Manager::onDownloadedBytes(uint64_t sz)
{
  m_last_reported_downloaded = sz;
  onDownloadProgress();
}

void Manager::onWrittenBytes(uint64_t sz)
{
  m_last_reported_written = sz;
  onDownloadProgress();
}

void Manager::stopDownload()
{
  if (!downloading()) return;

  InfoHub::logInfo(tr("Stopping downloads"));

  cleanupDownload();

  m_download_type = NoDownload;
  emit downloadingChanged(false);
}

////////////////////////////////////////////////////////////
/// support for cleanup

QStringList Manager::getNonNeededFilesList()
{
  QStringList files;
  m_not_needed_files_size = -1;
  if (!ready()) return files;

  qint64 notNeededSize = 0;

  m_not_needed_files.clear();

  // fill up needed files
  QSet<QString> wanted;
  wanted.insert(fullPath(const_fname_server_url));
  wanted.insert(fullPath(const_fname_countries_requested));
  wanted.insert(fullPath(const_fname_countries_provided));
  wanted.insert(fullPath(const_fname_db_files));

  QJsonObject req_countries = m_maps_requested;
  for (QJsonObject::const_iterator request_iter = req_countries.constBegin();
       request_iter != req_countries.constEnd(); ++request_iter)
    {
      const QJsonObject request = request_iter.value().toObject();
      if (request.empty()) continue;

      for (Feature *f: m_features)
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
  m_not_needed_files_size = notNeededSize;

  return files;
}

qint64 Manager::getNonNeededFilesSize()
{
  return m_not_needed_files_size;
}

QStringList Manager::getDirsWithNonNeededFiles()
{
  QStringList dirlist;

  /// Valhalla files are considered an exception and
  /// we'll report just that some of the valhalla's subdir
  /// will be deleted, not each subdir separately. Otherwise, we
  /// would get too many subdirs in this case
  const QString valhalla_base = fullPath("valhalla");

  if (m_not_needed_files_size >= 0)
    {
      for (const QString &fp: m_not_needed_files)
        {
          QFileInfo fi(fp);
          QString dir = fi.dir().path();

          // special processing of valhalla's case
          if (dir.indexOf(valhalla_base) == 0)
            dir = valhalla_base;

          if (!dirlist.contains(dir))
            dirlist.append(dir);
        }

      dirlist.sort();
    }

  return dirlist;
}


bool Manager::deleteNonNeededFiles(const QStringList files)
{
  if (!ready()) return false;

  if ( files != m_not_needed_files )
    {
      InfoHub::logError("Internal error: list of files given to delete does not matched with an expected one");
      m_not_needed_files.clear();
      return false;
    }

  setDeleting(true);

  DeleterThread *del = new DeleterThread(this, m_root_dir, m_not_needed_files);
  connect(del, &DeleterThread::finished, this, &Manager::onDeleteFinished);
  connect(del, &DeleterThread::finished, del, &DeleterThread::deleteLater);
  del->start();

  return true;
}

void Manager::setDeleting(bool state)
{
  if (state != m_deleting)
    {
      m_deleting = state;
      emit deletingChanged(state);
    }
}

bool Manager::deleting()
{
  return m_deleting;
}

void Manager::onDeleteFinished()
{
  m_not_needed_files.clear();
  this->setDeleting(false);
}

////////////////////////////////////////////////////////////
/// support for updates

bool Manager::updateProvided()
{
  if (!ready()) return false;

  QString fname = const_fname_server_url;
  if (m_development_disable_url_update) fname += "-EXTRA";
  if ( startDownload(ServerUrl, m_provided_url,
                     fullPath(fname),
                     FileDownloader::Plain,
                     true) )
    {
      emit downloadProgress(tr("Updating the distribution server URL"));
      return true;
    }

  return false;
}

void Manager::checkUpdates()
{
  m_last_found_updates = QJsonArray();

  if ( m_root_dir.exists() &&
       m_root_dir.exists(const_fname_countries_requested) &&
       m_root_dir.exists(const_fname_countries_provided) )
    {
      QJsonObject possible_list = loadJson(fullPath(const_fname_countries_provided));
      QJsonObject requested_list = m_maps_requested;

      for (QJsonObject::const_iterator request_iter = requested_list.constBegin();
           request_iter != requested_list.constEnd(); ++request_iter)
        {
          QJsonObject update;

          const QJsonObject request = request_iter.value().toObject();
          if (request.empty()) continue;

          const QJsonObject possible = possible_list.value(request_iter.key()).toObject();
          if (possible.empty()) continue;

          uint64_t s = 0;
          for (const Feature *f: m_features)
            if ( f->hasFeatureDefined(possible) &&
                 (!f->hasFeatureDefined(request) ||
                  f->getDateTime(request) < f->getDateTime(possible)) )
              {
                update.insert(f->name(), possible.value(f->name()).toObject());
                s += f->getSize(request);
              }

          if (!update.empty())
            {
              update.insert("id", request_iter.key());
              update.insert("name", getPretty(possible) );
              update.insert("type", possible.value("type").toString());
              update.insert("size", QString("%L1").arg( (int)round(s/1024./1024.) ) );

              m_last_found_updates.append(update);
            }
        }
    }
  else
    {
      if (!m_root_dir.exists())
        InfoHub::logWarning(tr("Cannot check for updates due to missing maps storage folder"));
      else if (!m_root_dir.exists(const_fname_countries_requested))
        InfoHub::logWarning(tr("Cannot check for updates due to missing list of requested countries. Select countries before checking for updates."));
      else
        InfoHub::logWarning(tr("Cannot check for updates due to missing list of provided countries. Download the list before checking for updates."));
    }

  if (!m_last_found_updates.empty())
    {
      QJsonDocument doc(m_last_found_updates);
      emit updatesForDataFound(doc.toJson());
    }
}

QString Manager::updatesFound()
{
  return QJsonDocument(m_last_found_updates).toJson();
}

void Manager::getUpdates()
{
  if (!ready()) return;

  QJsonObject possible_list = loadJson(fullPath(const_fname_countries_provided));
  QJsonObject requested = m_maps_requested;

  for (QJsonArray::const_iterator iter = m_last_found_updates.constBegin();
       iter != m_last_found_updates.constEnd(); ++iter)
    {
      const QJsonObject update = (*iter).toObject();
      QString key = update.value("id").toString();
      if (!requested.contains(key) || !possible_list.contains(key)) continue;

      requested.insert(key, possible_list.value(key).toObject());
    }

  { // write updated requested json to a file
    QJsonDocument doc(requested);
    QFile file(fullPath(const_fname_countries_requested));
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write( doc.toJson() );
  }

  scanDirectories();
  missingData();

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

  QHash<QString,QString> dirs;

  for (QJsonObject::const_iterator i = m_maps_available.constBegin();
       i != m_maps_available.constEnd(); ++i )
    {
      const QJsonObject c = i.value().toObject();
      QString id = getId(c);

      if ( getType(c) == const_feature_type_country )
        for (const Feature *f: m_features)
          if (f->enabled() && f->name() == "geocoder_nlp")
            dirs[id] = fullPath( f->getPath(c) );
    }

  emit databaseGeocoderNLPChanged(dirs);
}

////////////////////////////////////////////////////////////
/// libpostal support
void Manager::updatePostal()
{
  AppSettings settings;

  QString path_global;
  QHash<QString,QString> dirs_country;

  QJsonObject obj_global = m_maps_available.value(const_feature_id_postal_global).toObject();
  for (const Feature *f: m_features)
    if (f->enabled() && f->name() == "postal_global")
      path_global = fullPath( f->getPath(obj_global) );

  for (QJsonObject::const_iterator i = m_maps_available.constBegin();
       i != m_maps_available.constEnd(); ++i )
    {
      const QJsonObject c = i.value().toObject();
      QString id = getId(c);

      if ( getType(c) == const_feature_type_country )
        for (const Feature *f: m_features)
          if (f->enabled() && f->name() == "postal_country")
            dirs_country[id] = fullPath( f->getPath(c) );
    }

  emit databasePostalChanged(path_global, dirs_country);
}

////////////////////////////////////////////////////////////
/// mapnik support
void Manager::updateMapnik()
{
  // Mapnik is able to draw all available maps, so we give the full list
  QString path_global;
  QStringList path_countries;

  QJsonObject obj_global = m_maps_available.value(const_feature_id_mapnik_global).toObject();
  for (const Feature *f: m_features)
    if (f->enabled() && f->name() == "mapnik_global")
      path_global = fullPath( f->getPath(obj_global) );

  for (QJsonObject::const_iterator i = m_maps_available.constBegin();
       i != m_maps_available.constEnd(); ++i )
    {
      const QJsonObject c = i.value().toObject();
      QString id = getId(c);

      if ( getType(c) == const_feature_type_country )
        for (const Feature *f: m_features)
          if (f->enabled() && f->name() == "mapnik_country")
            path_countries.append( fullPath( f->getPath(c) ) );
    }

  // let Mapnik check whether anything has actually changed
  emit databaseMapnikChanged(path_global, path_countries);
}

////////////////////////////////////////////////////////////
/// mapboxgl support
void Manager::updateMapboxGL()
{
  // MapboxGL is able to draw all available maps, so we give the full list
  QString path_global;
  QString path_glyphs;
  QSet<QString> path_countries;

  {
    QJsonObject obj_global = m_maps_available.value(const_feature_id_mapboxgl_global).toObject();
    for (Feature *f: m_features)
      if (f->enabled() && f->name() == "mapboxgl_global")
        {
          QSet<QString> fnames;
          f->fillWantedFiles(obj_global, fnames);
          if (fnames.size() > 1)
            {
              InfoHub::logError(QString("Internal error, please report as a bug. MapboxGL Global returned wantedFiles > 1: %1").arg(fnames.size()));
              return;
            }
          if (fnames.size() > 0)
            path_global = *(fnames.begin());
        }
  }

  {
    QJsonObject obj_global = m_maps_available.value(const_feature_id_mapboxgl_glyphs).toObject();
    for (Feature *f: m_features)
      if (f->enabled() && f->name() == "mapboxgl_glyphs")
        {
          QSet<QString> fnames;
          f->fillWantedFiles(obj_global, fnames);
          if (fnames.size() > 1)
            {
              InfoHub::logError(QString("Internal error, please report as a bug. MapboxGL Glyphs returned wantedFiles > 1: %1").arg(fnames.size()));
              return;
            }
          if (fnames.size() > 0)
            path_glyphs = *(fnames.begin());
        }
  }

  for (QJsonObject::const_iterator i = m_maps_available.constBegin();
       i != m_maps_available.constEnd(); ++i )
    {
      const QJsonObject c = i.value().toObject();
      QString id = getId(c);

      if ( getType(c) == const_feature_type_country )
        for (Feature *f: m_features)
          if (f->enabled() && f->name() == "mapboxgl_country")
            f->fillWantedFiles(c, path_countries);
    }

  emit databaseMapboxGLChanged(path_global, path_glyphs, path_countries);
}

////////////////////////////////////////////////////////////
/// Valhalla support
void Manager::updateValhalla()
{
  QStringList path_countries;

  for (QJsonObject::const_iterator i = m_maps_available.constBegin();
       i != m_maps_available.constEnd(); ++i )
    {
      const QJsonObject c = i.value().toObject();
      QString id = getId(c);

      if ( getType(c) == const_feature_type_country )
        for (const Feature *f: m_features)
          if (f->enabled() && f->name() == "valhalla")
            path_countries.append( fullPath( f->getPath(c) ) );
    }

  emit databaseValhallaChanged(fullPath( "valhalla/tiles" ), path_countries);
}
