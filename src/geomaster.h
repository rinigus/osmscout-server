#ifndef GEOMASTER_H
#define GEOMASTER_H

#include "postal.h"
#include "geocoder.h"

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QPair>
#include <QSet>
#include <QMutex>

/////////////////////////////////////////
/// \brief The GeoMaster class
///
/// Access to Geocoder-NLP and LibPostal provided geocoder
///
class GeoMaster : public QObject
{
  Q_OBJECT

  /////////////////////////////////////////////////////////
  /// true if memory usage can be large due to non-specified
  /// language selection
  Q_PROPERTY(bool warnLargeRamLangNotSpecified READ warnLargeRamLangNotSpecified NOTIFY warnLargeRamLangNotSpecifiedChanged)

public:
  explicit GeoMaster(QObject *parent = 0);

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Search for a pattern and return results as a JSON array written to QByteString
  ///
  /// Have to use unique name to simplify binding it
  ///
  /// \param searchPattern
  /// \param result
  /// \param limit
  /// \return
  bool searchExposed(const QString &searchPattern, QByteArray &result, size_t limit, bool full_result);

  /////////////////////////////////////////////////////////////
  /// \brief Search for a pattern and return the coordinates of the first found object
  ///
  /// Can be used for finding coordinates of the reference points
  ///
  /// \param searchPattern
  /// \param lat this is a return value, latitude of the reference point
  /// \param lon this is a return value, longitude of the reference point
  /// \param name this is a return value, description of the found reference point
  /// \return true if a reference point was found
  ///
  bool search(const QString &searchPattern, double &lat, double &lon, std::string &name);

  /// \brief Search for POI by its type or name near the given origin
  ///
  bool guide(const QString &poitype, const QString &name,
             double lat, double lon, double radius, size_t limit, QByteArray &result_data);

  bool warnLargeRamLangNotSpecified() const { return m_warnLargeRamLangNotSpecified; }

public slots:
  void onSettingsChanged();
  void onGeocoderNLPChanged(QHash<QString, QString> dirs);
  void onPostalChanged(QString global, QHash<QString, QString> dirs_country);
  void onSelectedMapChanged(QString);

signals:
  void warnLargeRamLangNotSpecifiedChanged(bool warning);

protected:
  bool search(const QString &searchPattern, QJsonObject &result, size_t limit,
              double &lat, double &lon, std::string &name, size_t &number_of_results);

  void checkWarnings(bool lang_specified);

  void loadTagAlias(const QStringList &lang_list);
  QString normalize(const QString &str) const;
  QString tag2alias(const QString &tag) const;

protected:
  QMutex m_mutex;

  GeoNLP::Postal m_postal;
  GeoNLP::Geocoder m_geocoder;

  bool m_search_all_maps{false};
  bool m_continue_search_if_hit_found{false};
  bool m_postal_full_library{false};

  bool m_warnLargeRamLangNotSpecified{false};

  QString m_map_selected;
  QString m_postal_global;
  QStringList m_countries;
  QHash<QString, QString> m_postal_country_dirs;
  QHash<QString, QString> m_geocoder_dirs;

  QStringList m_tag_alias_langs;
  QHash<QString, QString> m_tag_to_alias;         ///< mapping from geocoder tags to preferred alias
  QHash<QString, QSet<QString> > m_alias_to_tag;  ///< mapping from alias to geocoder tags
};

#endif // GEOMASTER_H
