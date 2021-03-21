/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "config-common.h"

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include <QDBusVariant>

/*
 * Proxy class for interface org.osm.scout.server1.settings
 */
class AppSettings: public QDBusAbstractInterface
{
  Q_OBJECT
public:
  static inline const char *staticInterfaceName()
  { return DBUS_INTERFACE_SETTINGS; }

public:
  AppSettings(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

  ~AppSettings();

  /// \brief true when profiles are used to select backends
  Q_PROPERTY(bool profilesUsed READ profilesUsed NOTIFY profilesUsedChanged)

  /// \brief true if country selection has to be shown
  Q_PROPERTY(bool countrySelectionNeeded READ countrySelectionNeeded NOTIFY countrySelectionNeededChanged)

  /// \brief true if the application has been run the first time
  Q_PROPERTY(bool firstTime READ firstTime)

  /// \brief application version that was run before
  Q_PROPERTY(int lastRunVersion READ lastRunVersion)

  /// \brief build configuration: whether mapnik backend is available
  Q_PROPERTY(bool hasBackendMapnik READ hasBackendMapnik NOTIFY hasBackendMapnikChanged)

  /// \brief build configuration: whether libosmscout backend is available
  Q_PROPERTY(bool hasBackendOsmScout READ hasBackendOsmScout NOTIFY hasBackendOsmScoutChanged)

  /// \brief build configuration: whether systemd support is available
  Q_PROPERTY(bool hasBackendSystemD READ hasBackendSystemD NOTIFY hasBackendSystemDChanged)

  /// \brief build configuration: whether valhalla backend is available
  Q_PROPERTY(bool hasBackendValhalla READ hasBackendValhalla NOTIFY hasBackendValhallaChanged)

  inline bool countrySelectionNeeded() const
  { return qvariant_cast< bool >(property("countrySelectionNeeded")); }

  inline bool firstTime() const
  { return qvariant_cast< bool >(property("firstTime")); }

  inline bool hasBackendMapnik() const
  { return qvariant_cast< bool >(property("hasBackendMapnik")); }

  inline bool hasBackendOsmScout() const
  { return qvariant_cast< bool >(property("hasBackendOsmScout")); }

  inline bool hasBackendSystemD() const
  { return qvariant_cast< bool >(property("hasBackendSystemD")); }

  inline bool hasBackendValhalla() const
  { return qvariant_cast< bool >(property("hasBackendValhalla")); }

  inline int lastRunVersion() const
  { return qvariant_cast< int >(property("lastRunVersion")); }

  inline bool profilesUsed() const
  { return qvariant_cast< bool >(property("profilesUsed")); }

public Q_SLOTS: // METHODS
  inline QStringList availableProfiles()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<QStringList> v =
        asyncCallWithArgumentList(QStringLiteral("availableProfiles"), argumentList);
    return qvariant_cast< QStringList >(v.value());
  }

  inline int currentProfile()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<int> v =
        asyncCallWithArgumentList(QStringLiteral("currentProfile"), argumentList);
    return qvariant_cast< int >(v.value());
  }

  inline bool hasUnits(const QString &key)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(key);
    QDBusPendingReply<bool> v =
        asyncCallWithArgumentList(QStringLiteral("hasUnits"), argumentList);
    return qvariant_cast< bool >(v.value());
  }

  inline void setCurrentProfile(int profile)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(profile);
    asyncCallWithArgumentList(QStringLiteral("setCurrentProfile"), argumentList);
  }

  inline void setValue(const QString &key, const QVariant &value)
  {
    QDBusVariant v(value);
    QList<QVariant> argumentList;
    argumentList << key << QVariant::fromValue(v);
    asyncCallWithArgumentList(QStringLiteral("setValue"), argumentList);
  }

  inline int unitDisplayDecimals()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<int> v =
        asyncCallWithArgumentList(QStringLiteral("unitDisplayDecimals"), argumentList);
    return qvariant_cast< int >(v.value());
  }

  inline double unitFactor()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<double> v =
        asyncCallWithArgumentList(QStringLiteral("unitFactor"), argumentList);
    return qvariant_cast< double >(v.value());
  }

  inline int unitIndex()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<int> v =
        asyncCallWithArgumentList(QStringLiteral("unitIndex"), argumentList);
    return qvariant_cast< int >(v.value());
  }

  inline QString unitName(const QString &key)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(key);
    QDBusPendingReply<QString> v =
        asyncCallWithArgumentList(QStringLiteral("unitName"), argumentList);
    return qvariant_cast< QString >(v.value());
  }

  inline bool valueBool(const QString &key)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(key);
    QDBusPendingReply<bool> v =
        asyncCallWithArgumentList(QStringLiteral("valueBool"), argumentList);
    return qvariant_cast< bool >(v.value());
  }

  inline double valueFloat(const QString &key)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(key);
    QDBusPendingReply<double> v =
        asyncCallWithArgumentList(QStringLiteral("valueFloat"), argumentList);
    return qvariant_cast< double >(v.value());
  }

  inline int valueInt(const QString &key)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(key);
    QDBusPendingReply<int> v =
        asyncCallWithArgumentList(QStringLiteral("valueInt"), argumentList);
    return qvariant_cast< int >(v.value());
  }

  inline QString valueString(const QString &key)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(key);
    QDBusPendingReply<QString> v =
        asyncCallWithArgumentList(QStringLiteral("valueString"), argumentList);
    return qvariant_cast< QString >(v.value());
  }

Q_SIGNALS: // SIGNALS
  void countrySelectionNeededChanged(bool selection);
  void hasBackendMapnikChanged(bool in0);
  void hasBackendOsmScoutChanged(bool in0);
  void hasBackendSystemDChanged(bool in0);
  void hasBackendValhallaChanged(bool in0);
  void osmScoutSettingsChanged();
  void profilesUsedChanged(bool used);
};

namespace org {
  namespace osm {
    namespace scout {
      namespace server1 {
        typedef ::AppSettings settings;
      }
    }
  }
}
#endif
