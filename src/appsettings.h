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

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>
#include <QString>

/// \brief A wrapper around QSettings allowing to expose it to QML
///
class AppSettings : public QSettings
{
  Q_OBJECT

  /// \brief true when profiles are used to select backends
  Q_PROPERTY(bool profilesUsed READ profilesUsed NOTIFY profilesUsedChanged)

  /// \brief true if country selection has to be shown
  Q_PROPERTY(bool countrySelectionNeeded READ countrySelectionNeeded NOTIFY countrySelectionNeededChanged)

  /// \brief true if the application has been run the first time
  Q_PROPERTY(bool firstTime READ firstTime)

  /// \brief application version that was run before
  Q_PROPERTY(int lastRunVersion READ lastRunVersion)

public:
  AppSettings();
  virtual ~AppSettings() {}

  Q_INVOKABLE void setValue(const QString &key, const QVariant &value);

  Q_INVOKABLE int valueInt(const QString &key);
  Q_INVOKABLE bool valueBool(const QString &key);
  Q_INVOKABLE double valueFloat(const QString &key);
  Q_INVOKABLE QString valueString(const QString &key);

  Q_INVOKABLE int unitIndex() const; ///< Index of selected units
  Q_INVOKABLE int unitDisplayDecimals() const;
  Q_INVOKABLE double unitFactor() const; /// multiply by this factor when displaying distance or speed to the user

  Q_INVOKABLE bool hasUnits(const QString &key) const;
  Q_INVOKABLE QString unitName(const QString &key) const;

  void initDefaults(); ///< Initialize settings for configurable parameters on the first start

  bool profilesUsed() const { return m_profiles_used; }

  bool countrySelectionNeeded() const { return m_country_selection_needed; }

  bool firstTime() const { return m_first_time; }

  bool lastRunVersion() const { return m_last_run_version; }

  /// \brief Preferred language for search results, map, and routing
  ///
  /// Returns empty string if the preference is in displayed/searhed local country
  /// language or a language code
  QString preferredLanguage();

signals:
  void osmScoutSettingsChanged();
  void profilesUsedChanged(bool used);
  void countrySelectionNeededChanged(bool selection);

public slots:
  void fireOsmScoutSettingsChanged();

protected:
  QString unitName(bool speed) const;

  void setProfile();

  void checkCountrySelectionNeeded();

protected:
  bool m_signal_osm_scout_changed_waiting = false;
  bool m_profiles_used = false;
  bool m_country_selection_needed = true;
  bool m_first_time = false;
  int m_last_run_version{0};
};

#endif // APPSETTINGS_H
