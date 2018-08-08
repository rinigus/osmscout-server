/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
 * 
 * This file is part of OSM Scout Server.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "geomaster.h"
#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QMutexLocker>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <QDebug>

#include <map>

GeoMaster::GeoMaster(QObject *parent) : QObject(parent)
{
  onSettingsChanged();
}


void GeoMaster::onSettingsChanged()
{
  QMutexLocker lk(&m_mutex);
  AppSettings settings;

  // prepare for new settings
  m_geocoder.drop();
  m_postal.clear_languages();
  m_countries.clear();

  useGeocoderNLP = (settings.valueInt(GEOMASTER_SETTINGS "use_geocoder_nlp") > 0);

  if (!useGeocoderNLP || m_map_selected.isEmpty())
    {
      checkWarnings(false /*does not play any role here*/);
      return; // no need to load anything
    }

  // apply new settings
  m_postal.set_initialize_every_call(settings.valueBool(GEOMASTER_SETTINGS "initialize_every_call"));
  m_postal.set_use_primitive(settings.valueBool(GEOMASTER_SETTINGS "use_primitive"));

  m_search_all_maps = settings.valueBool(GEOMASTER_SETTINGS "search_all_maps");
  m_continue_search_if_hit_found = settings.valueBool(GEOMASTER_SETTINGS "continue_search_if_hit_found");

  m_geocoder.set_result_language( settings.preferredLanguage().toStdString() );

  // fill country list
  {
    m_postal_full_library = (m_postal_global.isEmpty() && m_postal_country_dirs.isEmpty());

    // put selected map as a first element in the countries list
    if (m_geocoder_dirs.contains(m_map_selected) &&
        (m_postal_full_library || m_postal_country_dirs.contains(m_map_selected)))
      m_countries.append(m_map_selected);
    else
      {
        InfoHub::logError(tr("Selected country not available for geocoder: %1").arg(m_map_selected));
        return;
      }

    if (m_search_all_maps)
      {
        QStringList geokeys = m_geocoder_dirs.keys();
        geokeys.removeAll(m_map_selected);
        for (const QString &k: geokeys)
          if (m_postal_full_library || m_postal_country_dirs.contains(k))
            m_countries.append(k);
      }

    QString info = tr("Countries used in search: ");
    for (const QString &k: m_countries)
      info += k + " ";
    InfoHub::logInfo(info);
  }

  // prepare postal and geocoder
  std::string postal_global = m_postal_global.toStdString();
  std::string postal_country = m_postal_country_dirs.value(m_map_selected).toStdString();
  m_postal.set_postal_datadir(postal_global, postal_country);

  QString geopath = m_geocoder_dirs.value(m_map_selected);
  if (geopath.length() < 1 || !m_geocoder.load(geopath.toStdString()))
    {
      InfoHub::logError(tr("Cannot open geocoder database") + ": " + geopath);
      return;
    }

  InfoHub::logInfo(tr("Opened geocoder database") + " " + geopath, true);

  m_geocoder.set_max_queries_per_hierarchy(settings.valueInt(GEOMASTER_SETTINGS "max_queries_per_hierarchy"));

  QString lang = settings.valueString(GEOMASTER_SETTINGS "languages");
  QStringList lang_list;

  if (lang.length() > 0)
    {
      lang_list = lang.split(',', QString::SkipEmptyParts);
      for (int i=0; i < lang_list.size(); ++i)
        lang_list[i] = lang_list[i].simplified();

      QString used;
      for (QString l: lang_list)
        {
          m_postal.add_language(l.toStdString());
          used += l + " ";
        }
      InfoHub::logInfo(tr("libpostal using languages: %1").arg(used));
      checkWarnings(!lang_list.isEmpty());
    }
  else
    {
      InfoHub::logInfo(tr("libpostal will use all covered languages"));
      checkWarnings(false);
    }

  loadTagAlias(lang_list);
}

void GeoMaster::onGeocoderNLPChanged(QHash<QString, QString> dirs)
{
  bool changed;
  {
    QMutexLocker lk(&m_mutex);
    changed = (m_geocoder_dirs != dirs);
    m_geocoder_dirs = dirs;
  }
  if (changed)
    onSettingsChanged();
}

void GeoMaster::onPostalChanged(QString global, QHash<QString, QString> country_dirs)
{
  bool changed;
  {
    QMutexLocker lk(&m_mutex);
    changed = (m_postal_global != global || m_postal_country_dirs != country_dirs);
    m_postal_global = global;
    m_postal_country_dirs = country_dirs;
  }
  if (changed)
    onSettingsChanged();
}

void GeoMaster::onSelectedMapChanged(QString selected)
{
  bool changed;
  {
    QMutexLocker lk(&m_mutex);
    changed = (m_map_selected != selected);
    m_map_selected = selected;
  }
  if (changed)
    onSettingsChanged();
}

void GeoMaster::checkWarnings(bool lang_specified)
{
  bool toWarnLang = false;
  if (!useGeocoderNLP || m_map_selected.isEmpty()) toWarnLang = false;
  else
    {
      if (lang_specified) toWarnLang = false;
      else toWarnLang = true;
    }

  if (toWarnLang != m_warnLargeRamLangNotSpecified)
    {
      m_warnLargeRamLangNotSpecified = toWarnLang;
      emit warnLargeRamLangNotSpecifiedChanged(m_warnLargeRamLangNotSpecified);
    }
}

QString GeoMaster::normalize(const QString &str) const
{
  return str.normalized(QString::NormalizationForm_KC).toCaseFolded();
}

void GeoMaster::loadTagAlias(const QStringList &lang_list)
{
  QStringList langs;
  QString locale = QLocale::system().name();

  langs.append(locale.toLower());
  langs.append(locale.left( locale.indexOf('_') ).toLower());
  langs.append(lang_list);

  if ( m_tag_alias_langs == langs)
    return; // we loaded that already

  m_tag_to_alias.clear();
  m_alias_to_tag.clear();
  m_aliases.clear();

  QSet<QString> aliases;

  // load JSON aliases and tags
  QJsonObject data;
  {
    QFile f(GEOCODERNLP_ALIASFILE);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
      data = QJsonDocument::fromJson(f.readAll()).object();
  }

  for (const QString &lang: langs)
    {
      {
        const QJsonObject d = data.value("tag2alias").toObject().value(lang).toObject();
        for (auto iter = d.constBegin(); iter!=d.end(); ++iter)
          {
            QString tag = iter.key();
            QString alias = iter.value().toString();
            if ( !m_tag_to_alias.contains(tag) )
              m_tag_to_alias[tag] = alias;
          }
      }

      {
        const QJsonObject d = data.value("alias2tag").toObject().value(lang).toObject();
        for (auto iter = d.constBegin(); iter!=d.constEnd(); ++iter)
          {
            QString alias = GeoMaster::normalize(iter.key());
            QJsonArray arr = iter.value().toArray();
            for (auto ti=arr.constBegin(); ti!=arr.constEnd(); ti++)
              m_alias_to_tag[alias].insert( (*ti).toString() );
            aliases.insert(iter.key());
          }
      }
    }

  m_aliases = QStringList::fromSet(aliases);
  m_aliases.sort();

  m_tag_alias_langs = langs;
}

QString GeoMaster::tag2alias(const QString &tag) const
{
  auto iter = m_tag_to_alias.find(tag);
  if (iter == m_tag_to_alias.constEnd())
    return tag;
  return *iter;
}

static std::string v2s(const std::vector<std::string> &v)
{
  std::string s = "{";
  for (auto i: v)
    {
      if (s.length() > 1) s += ", ";
      s += i;
    }
  s += "}";
  return s;
}

bool GeoMaster::search(const QString &searchPattern, QJsonObject &result, size_t limit,
                       double &lat, double &lon, std::string &name, size_t &number_of_results)
{
  QMutexLocker lk(&m_mutex);

  std::vector<GeoNLP::Geocoder::GeoResult> search_result;
  QJsonObject parsed;
  QJsonObject parsed_normalized;

  struct PostalRes {
    std::vector< GeoNLP::Postal::ParseResult > parsed;
    GeoNLP::Postal::ParseResult nonorm;
  };

  size_t levels_resolved = 0;
  std::map< std::string, PostalRes > postal_cache;
  for(const QString country: m_countries)
    {
      if (!m_geocoder.load(m_geocoder_dirs.value(country).toStdString()))
        {
          InfoHub::logError(tr("Cannot open geocoding database: %1").arg(m_geocoder_dirs.value(country)));
          return false;
        }

      // parsing with libpostal
      std::vector< GeoNLP::Postal::ParseResult > parsed_query;
      GeoNLP::Postal::ParseResult nonorm;
      std::string postal_id;

      if (!m_postal_full_library)
        postal_id = m_postal_country_dirs.value(country).toStdString();

      if ( postal_cache.count(postal_id) > 0 )
        {
          PostalRes &r = postal_cache[postal_id];
          parsed_query = r.parsed;
          nonorm = r.nonorm;
        }
      else
        {
          if (!m_postal_full_library)
            m_postal.set_postal_datadir_country(postal_id);

          if ( !m_postal.parse( searchPattern.toStdString(),
                                parsed_query, nonorm) )
            {
              InfoHub::logError(tr("Error parsing by libpostal, maybe libpostal databases are not available"));
              return false;
            }

          // record parsing results
          {
            QJsonObject r;
            for (auto a: nonorm)
              r.insert(QString::fromStdString(a.first), QString::fromStdString(v2s(a.second)));
            parsed.insert(country, r);
          }

          PostalRes r;
          r.parsed = parsed_query;
          r.nonorm = nonorm;
          postal_cache[postal_id] = r;

          {
            QJsonArray arr;
            QStringList info_id_split = QString::fromStdString(postal_id).split('/');
            QString info_id;
            if (info_id_split.size() > 0)
              info_id = info_id_split.value(info_id_split.size()-1);

            for (const GeoNLP::Postal::ParseResult &pr: parsed_query)
              {
                QJsonObject r;
                QString info;
                for (auto a: pr)
                  {
                    r.insert(QString::fromStdString(a.first), QString::fromStdString(v2s(a.second)));
                    info += QString::fromStdString(a.first) + ": " + QString::fromStdString(v2s(a.second)) + "; ";
                  }

                arr.push_back(r);

                InfoHub::logInfo(tr("Parsed query [%1]: %2").arg(info_id).arg(info));
              }
            parsed_normalized.insert(country, arr);
          }
        }

      // search
      m_geocoder.set_max_results(limit);
      std::vector<GeoNLP::Geocoder::GeoResult> search_result_country;
      if ( !m_geocoder.search(parsed_query, search_result_country, levels_resolved) )
        {
          InfoHub::logError(tr("Error while searching with geocoder-nlp"));
          return false;
        }

      if (!search_result_country.empty())
        {
          if ( search_result.empty() ||
               ( search_result[0].levels_resolved < search_result_country[0].levels_resolved ) )
            {
              search_result = search_result_country;
              levels_resolved = search_result_country[0].levels_resolved;
            }
          else if ( search_result[0].levels_resolved == search_result_country[0].levels_resolved )
            search_result.insert(search_result.end(),
                                 search_result_country.begin(), search_result_country.end());
        }

      if (!search_result.empty() && !m_continue_search_if_hit_found)
        break;
    }

  // sort results
  std::sort(search_result.begin(), search_result.end() );

  // enforce the limit
  if (search_result.size() > limit)
    search_result.resize(limit);

  // record results
  result.insert("query", searchPattern);
  result.insert("parsed", parsed);
  result.insert("parsed_normalized", parsed_normalized);

  {
    QJsonArray arr;
    for (const GeoNLP::Geocoder::GeoResult &sr: search_result)
      {
        QJsonObject r;

        r.insert("admin_region", QString::fromStdString(sr.address));
        r.insert("title", QString::fromStdString(sr.title));
        r.insert("lat", sr.latitude);
        r.insert("lng", sr.longitude);
        r.insert("object_id", sr.id);
        r.insert("type", tag2alias(QString::fromStdString(sr.type)));
        r.insert("levels_resolved", (int)sr.levels_resolved);
        r.insert("admin_levels", (int)sr.admin_levels);

        arr.push_back(r);
      }

    result.insert("result", arr);
  }

  number_of_results = search_result.size();
  if (number_of_results > 0)
    {
      lat = search_result[0].latitude;
      lon = search_result[0].longitude;
      name = search_result[0].address;
    }

  return true;
}

bool GeoMaster::search(const QString &searchPattern, double &lat, double &lon, std::string &name)
{
  QJsonObject obj;
  size_t number_of_results;

  if ( !search(searchPattern, obj, 1, lat, lon, name, number_of_results ) )
    {
      InfoHub::logWarning("Search for reference point failed");
      return false;
    }

  if ( number_of_results > 0 )
    return true;

  InfoHub::logWarning(tr("Search for reference point failed: cannot find") + " " + searchPattern);
  return false;
}

bool GeoMaster::searchExposed(const QString &searchPattern, QByteArray &result, size_t limit, bool full_result)
{
  QJsonObject sres;
  double lat, lon;
  std::string name;
  size_t number_of_results;

  if ( !search(searchPattern, sres, limit, lat, lon, name, number_of_results ) )
    return false;

  if (!full_result)
    {
      QJsonDocument document(sres.value("result").toArray());
      result = document.toJson();
    }
  else
    {
      QJsonDocument document(sres);
      result = document.toJson();
    }

  return true;
}


bool GeoMaster::guide(const QString &poitype, const QString &name,
                      bool accout_for_reference, double lat, double lon,
                      QJsonArray &route_lat, QJsonArray &route_lon,
                      double radius, size_t limit, QByteArray &result_data)
{
  if (poitype.isEmpty() && name.isEmpty())
    return false;

  QMutexLocker lk(&m_mutex);

  std::vector<GeoNLP::Geocoder::GeoResult> search_result;
  std::map< std::string, std::vector<std::string> > postal_cache;
  std::string name_query = name.toStdString();

  // fill route vectors
  std::vector<double> line_lat, line_lon;
  bool has_line = false;
  size_t ignore_segments = 0;
  if (route_lat.size() > 0 || route_lon.size())
    {
      has_line = true;

      for (auto i: route_lat)
        if (i.isDouble())
          line_lat.push_back(i.toDouble());
        else
          {
            // technical message
            InfoHub::logWarning("In guide search: Error while converting route latitudes");
            return false;
          }

      for (auto i: route_lon)
        if (i.isDouble())
          line_lon.push_back(i.toDouble());
        else
          {
            // technical message
            InfoHub::logWarning("In guide search: Error while converting route longitudes");
            return false;
          }

      if (line_lon.size() != line_lat.size())
        {
          std::cout << line_lon.size() << " " << line_lat.size() << std::endl;
          // technical message
          InfoHub::logWarning("In guide search: route given by different number of longitudes and latitudes");
          return false;
        }

      if (accout_for_reference)
        ignore_segments = GeoNLP::Geocoder::closest_segment(line_lat, line_lon, lat, lon);
    }

  // fill type query - for now just use as its a full query
  std::vector<std::string> type_query;
  if (!poitype.isEmpty())
    {
      QString typenorm = normalize(poitype);
      auto tags = m_alias_to_tag.find(typenorm);
      if (tags != m_alias_to_tag.constEnd())
        {
          for (auto t: *tags)
            type_query.push_back(t.toStdString());
        }
      else
        type_query.push_back(poitype.toStdString());
    }

  for(const QString country: m_countries)
    {
      if (!m_geocoder.load(m_geocoder_dirs.value(country).toStdString()))
        {
          InfoHub::logError(tr("Cannot open geocoding database: %1").arg(m_geocoder_dirs.value(country)));
          return false;
        }

      // parsing with libpostal
      std::vector< std::string > parsed_name;
      std::string postal_id;

      if ( !name.isEmpty() )
        {
          if (!m_postal_full_library)
            postal_id = m_postal_country_dirs.value(country).toStdString();

          if ( postal_cache.count(postal_id) > 0 )
            {
              parsed_name = postal_cache[postal_id];
            }
          else
            {
              if (!m_postal_full_library)
                m_postal.set_postal_datadir_country(postal_id);

              m_postal.expand_string(name_query, parsed_name);

              postal_cache[postal_id] = parsed_name;
            }
        }

      // search
      m_geocoder.set_max_results(0); // limit is enforced later

      bool ok = false;
      if (has_line)
        ok = m_geocoder.search_nearby(parsed_name,
                                      type_query,
                                      line_lat, line_lon, radius,
                                      search_result,
                                      m_postal,
                                      ignore_segments);
      else
        ok = m_geocoder.search_nearby(parsed_name,
                                     type_query,
                                     lat, lon, radius,
                                     search_result,
                                     m_postal);
      if (!ok)
        {
          InfoHub::logError(tr("Error while searching with geocoder-nlp"));
          return false;
        }
    }

  // sort and enforce the limit
  GeoNLP::Geocoder::sort_by_distance(search_result.begin(), search_result.end());
  if (search_result.size() > limit)
    search_result.resize(limit);

  // record results
  QJsonObject result;
  result.insert("query_type", poitype);
  result.insert("query_name", name);
  {
    QJsonObject origin;
    origin.insert("lng", lon);
    origin.insert("lat", lat);
    result.insert("origin", origin);
  }

  {
    QJsonArray arr;
    for (const GeoNLP::Geocoder::GeoResult &sr: search_result)
      {
        QJsonObject r;

        r.insert("admin_region", QString::fromStdString(sr.address));
        r.insert("title", QString::fromStdString(sr.title));
        r.insert("lat", sr.latitude);
        r.insert("lng", sr.longitude);
        r.insert("distance", sr.distance);
        r.insert("object_id", sr.id);
        r.insert("type", tag2alias(QString::fromStdString(sr.type)));

        arr.push_back(r);
      }

    result.insert("results", arr);
  }

  QJsonDocument document(result);
  result_data = document.toJson();

  return true;
}

bool GeoMaster::poiTypes(QByteArray &result)
{
  QMutexLocker lk(&m_mutex);

  QJsonArray arr;
  for (const auto s: m_aliases)
    arr.push_back(s);

  QJsonDocument document(arr);
  result = document.toJson();

  return true;
}
