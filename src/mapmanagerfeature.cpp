#include "mapmanagerfeature.h"
#include "mapmanagerfeature_packtaskworker.h"
#include "infohub.h"
#include "appsettings.h"
#include "config.h"

// for libosmscout file version
#include <osmscout/TypeConfig.h>

#include <QDir>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QJsonArray>
#include <QThread>

#include <QDebug>

using namespace MapManager;

Feature::Feature(PathProvider *path,
                 const QString &feature_type,
                 const QString &feature_name,
                 const QString &feature_pretty_name,
                 const QStringList &feature_files,
                 const int version):
  QObject(),
  m_path_provider(path),
  m_type(feature_type),
  m_name(feature_name),
  m_pretty(feature_pretty_name),
  m_version(version),
  m_files(feature_files)
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
      QString special = o.value(m_name).toString();
      m_url = special;
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

bool Feature::isAvailable(const QJsonObject &request)
{
  if (!m_enabled || !isMyType(request) || m_assume_files_exist) return true;
  if (!request.contains(m_name)) return true;
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
                                FilesToDownload &missing)
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
                              QSet<QString> &wanted)
{
  if (!m_enabled || !isMyType(request)) return;

  QString path = getPath(request);
  for (const auto &f: m_files)
    wanted.insert( m_path_provider->fullPath(path + "/" + f) );
}

//////////////////////////////
/// This method was not used. If planning to use it, should also implement
/// corresponding method for Valhalla
//void Feature::deleteFiles(const QJsonObject &request)
//{
//  if (!m_enabled || !isMyType(request)) return;

//  QString path = getPath(request);
//  QDir dir(m_path_provider->fullPath("."));
//  for (const auto &f: m_files)
//    {
//      QString fp = path + "/" + f;
//      if (dir.remove(fp))
//        InfoHub::logInfo(QCoreApplication::translate("MapManagerFeature", "Removed file: %1").arg(fp));
//      else
//        InfoHub::logInfo(QCoreApplication::translate("MapManagerFeature", "Failed to remove file: %1").arg(fp));
//    }
//}

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
          osmscout::FILE_FORMAT_VERSION)
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
          GeoNLP::Geocoder::version)
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
          QCoreApplication::translate("MapManagerFeature", "Mapnik World coastlines"),
          mapnik_global_files,
          1)
{
}

QString FeatureMapnikGlobal::errorMissing() const
{
  return QCoreApplication::translate("MapManagerFeature", "Missing Mapnik World coastlines");
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
          3)
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
/// MapboxGL support
const static QStringList mapboxgl_global_files{
  "tiles-world.sqlite"
};

FeatureMapboxGLGlobal::FeatureMapboxGLGlobal(PathProvider *path):
  Feature(path, "mapboxgl/global", "mapboxgl_global",
          QCoreApplication::translate("MapManagerFeature", "Mapbox GL World overlay"),
          mapboxgl_global_files,
          1)
{
}

QString FeatureMapboxGLGlobal::errorMissing() const
{
  return QCoreApplication::translate("MapManagerFeature", "Missing Mapbox GL World overlay");
}

void FeatureMapboxGLGlobal::loadSettings()
{
  Feature::loadSettings();
  AppSettings settings;
  m_enabled = settings.valueBool(MAPMANAGER_SETTINGS "mapboxgl");
}


const static QStringList mapboxgl_glyphs_files{
  "glyphs.sqlite"
};

FeatureMapboxGLGlyphs::FeatureMapboxGLGlyphs(PathProvider *path):
  Feature(path, "mapboxgl/glyphs", "mapboxgl_glyphs",
          QCoreApplication::translate("MapManagerFeature", "Mapbox GL fonts"),
          mapboxgl_glyphs_files,
          1)
{
}

QString FeatureMapboxGLGlyphs::errorMissing() const
{
  return QCoreApplication::translate("MapManagerFeature", "Missing Mapbox GL fonts");
}

void FeatureMapboxGLGlyphs::loadSettings()
{
  Feature::loadSettings();
  AppSettings settings;
  m_enabled = settings.valueBool(MAPMANAGER_SETTINGS "mapboxgl");
}


FeatureMapboxGLCountry::FeatureMapboxGLCountry(PathProvider *path):
  Feature(path, "territory", "mapboxgl_country",
          QCoreApplication::translate("MapManagerFeature", "Mapbox GL country-specific support"),
          QStringList(),
          1)
{
}

QString FeatureMapboxGLCountry::errorMissing() const
{
  return QCoreApplication::translate("MapManagerFeature", "Missing country-specific Mapbox GL dataset");
}

void FeatureMapboxGLCountry::loadSettings()
{
  Feature::loadSettings();
  AppSettings settings;
  m_enabled = settings.valueBool(MAPMANAGER_SETTINGS "mapboxgl");
}

void FeatureMapboxGLCountry::requestFiles(const QJsonObject &request)
{
  m_files.clear();
  QJsonArray packs = request.value(m_name).toObject().value("packages").toArray();
  for (QJsonArray::const_iterator iter = packs.constBegin();
       iter != packs.constEnd(); ++iter)
    m_files.append("tiles-section-" + (*iter).toString() + ".sqlite");
}

bool FeatureMapboxGLCountry::isAvailable(const QJsonObject &request)
{
  if (!request.contains(m_name)) return true;
  requestFiles(request);
  return Feature::isAvailable(request);
}

void FeatureMapboxGLCountry::checkMissingFiles(const QJsonObject &request, FilesToDownload &missing)
{
  if (!request.contains(m_name)) return;
  requestFiles(request);
  Feature::checkMissingFiles(request, missing);
}

void FeatureMapboxGLCountry::fillWantedFiles(const QJsonObject &request, QSet<QString> &wanted)
{
  if (!request.contains(m_name)) return;
  requestFiles(request);
  Feature::fillWantedFiles(request, wanted);
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

QString FeatureValhalla::packFileName(QString pack) const
{
  return "valhalla/packages/" + pack + ".tar";
}

QString FeatureValhalla::packListName(QString pack) const
{
  return "valhalla/packages/" + pack + ".tar.list";
}

QStringList FeatureValhalla::getPackTileNames(QString listfname) const
{
  QStringList list;

  QFile file(listfname);
  if (!file.open(QIODevice::ReadOnly)) return list;

  QTextStream stream(&file);
  QString line;
  while (!stream.atEnd())
    {
      line = stream.readLine().trimmed();
      if (!line.isEmpty())
        list.append(line);
    }

  return list;
}

QString FeatureValhalla::getTilesTimestamp() const
{
  QFile file(m_path_provider->fullPath(const_valhalla_tiles_timestamp));
  if (!file.open(QIODevice::ReadOnly)) return QString();

  QTextStream stream(&file);
  return stream.readAll().trimmed();
}

FeatureValhalla::PackStateType FeatureValhalla::getPackState(QString pack, QString req_version, QString req_datetime) const
{
  QString pfname = packFileName(pack);

  QString version;
  QString datetime;

  if ( !m_path_provider->isRegistered(pfname, version, datetime) ||
       version != req_version || datetime != req_datetime )
    {
      return PackNotAvailable;
    }

  QDir dir(m_path_provider->fullPath("."));

  if (dir.exists(m_path_provider->fullPath(pfname)))
    {
      return PackDownloaded;
    }

  // this has to be checked after "Downloaded" to allow to update tiles
  // through unpacking
  QString tilesTimestamp = getTilesTimestamp();
  if (!tilesTimestamp.isEmpty() && tilesTimestamp != req_datetime)
    return PackNotAvailable;

  QString listname = m_path_provider->fullPath(packListName(pack));
  if (dir.exists(listname))
    {
      QStringList tiles = getPackTileNames(listname);
      if (tiles.isEmpty()) return PackNotAvailable; // either list is empty or there was error opening it
      for (const QString tilename: tiles)
        if (!dir.exists(m_path_provider->fullPath(tilename)))
          {
            return PackNotAvailable;
          }

      return PackUnpacked; // all tiles were there
    }

  return PackNotAvailable;
}

FeatureValhalla::PackStateType FeatureValhalla::isPackAvailable(QString pack, QString req_version, QString req_datetime)
{
  PackStateType state = getPackState(pack, req_version, req_datetime);
  if (state == PackDownloaded)
    {
      // have to unpack the archive and would become available after that
      addForUnpacking(m_path_provider->fullPath(packFileName(pack)), req_datetime);
    }

  return state;
}

bool FeatureValhalla::isAvailable(const QJsonObject &request)
{
  if (!m_enabled || !isMyType(request) || m_assume_files_exist) return true;
  if (!request.contains(m_name)) return true;
  if (!isCompatible(request)) return false;

  QString req_version = request.value(m_name).toObject().value("version").toString();
  QString req_datetime = request.value(m_name).toObject().value("timestamp").toString();

  QJsonArray packs = request.value(m_name).toObject().value("packages").toArray();
  bool allAvailable = true;
  for (QJsonArray::const_iterator iter = packs.constBegin();
       iter != packs.constEnd(); ++iter)
    if ( isPackAvailable( (*iter).toString(), req_version, req_datetime ) != PackUnpacked )
      allAvailable = false;

  return allAvailable;
}

void FeatureValhalla::checkMissingFiles(const QJsonObject &request,
                                        FilesToDownload &missing)
{
  if (!m_enabled || !isMyType(request) || !isCompatible(request) || m_assume_files_exist) return;
  if (!request.contains(m_name)) return;

  QString req_version = request.value(m_name).toObject().value("version").toString();
  QString req_datetime = request.value(m_name).toObject().value("timestamp").toString();
  bool added = false;

  QJsonArray packs = request.value(m_name).toObject().value("packages").toArray();
  for (QJsonArray::const_iterator iter = packs.constBegin();
       iter != packs.constEnd(); ++iter)
    if ( getPackState( (*iter).toString(), req_version, req_datetime ) == PackNotAvailable )
      {
        added = true;
        QString f = packFileName((*iter).toString());
        FileTask t;
        t.path = m_path_provider->fullPath(f);
        t.url = m_url + "/" + f;
        t.relpath = f;
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
                                      QSet<QString> &wanted)
{
  if (!m_enabled || !isMyType(request)) return;
  if (!request.contains(m_name)) return;

  // insert timestamp
  wanted.insert( m_path_provider->fullPath(const_valhalla_tiles_timestamp) );

  // insert packages-provided files and packages as well
  QJsonArray packs = request.value(m_name).toObject().value("packages").toArray();
  for (QJsonArray::const_iterator iter = packs.constBegin();
       iter != packs.constEnd(); ++iter)
    {
      QString pack = (*iter).toString();
      QString listname = m_path_provider->fullPath(packListName(pack));
      wanted.insert( m_path_provider->fullPath( packFileName(pack) ) );
      wanted.insert( listname );

      QStringList tiles = getPackTileNames(listname);
      for (const QString tilename: tiles)
        wanted.insert( m_path_provider->fullPath(tilename) );
    }
}

void FeatureValhalla::addForUnpacking(QString packtarname, QString datetime)
{
  PackTask task(packtarname, datetime);
  if ( m_pack_task_current == task || m_pack_tasks.contains(task) )
    return;

  m_pack_tasks.enqueue(task);
  handlePackTasks();
}

void FeatureValhalla::onPackTaskFinished()
{
  m_pack_task_current = PackTask();
  handlePackTasks();
}

void FeatureValhalla::onPackTaskError(QString errtxt)
{
  InfoHub::logWarning(errtxt);
}


void FeatureValhalla::handlePackTasks()
{
  if (!m_pack_task_current.isEmpty())
    return; // already working on a task

  if (m_pack_tasks.isEmpty())
    {
      // should have been called after processing the last task
      emit availabilityChanged();
      return;
    }

  m_pack_task_current = m_pack_tasks.dequeue();
  PackTaskWorker *worker = new PackTaskWorker(m_pack_task_current.filename, m_pack_task_current.datetime,
                                              m_path_provider->fullPath("."),
                                              m_path_provider->fullPath(const_valhalla_tiles_dirname),
                                              getTilesTimestamp());

  if (worker == nullptr)
    {
      // technical message, no need to translate
      InfoHub::logError("Cannot allocate QThread in FeatureValhalla::handlePackTasks");
      return;
    }

  connect(worker, &PackTaskWorker::errorDuringTask, this, &FeatureValhalla::onPackTaskError );
  connect(worker, &PackTaskWorker::finished, this, &FeatureValhalla::onPackTaskFinished );
  connect(worker, &PackTaskWorker::finished, worker, &QObject::deleteLater);

  worker->start();
}
