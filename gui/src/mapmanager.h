/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef MANAGER_H
#define MANAGER_H

#include "config-common.h"

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.osm.scout.server1.manager
 */
class MapManager: public QDBusAbstractInterface
{
  Q_OBJECT
public:
  static inline const char *staticInterfaceName()
  { return DBUS_INTERFACE_MANAGER; }

public:
  MapManager(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

  ~MapManager();

  /// \brief true when Map's storage dir is available
  Q_PROPERTY(bool storageAvailable READ storageAvailable NOTIFY storageAvailableChanged)

  /// \brief true when Map Manager can receive configuration chages
  Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)

  /// \brief true when download is active
  Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

  /// \brief true when manager is deleting
  Q_PROPERTY(bool deleting READ deleting NOTIFY deletingChanged)

  /// \brief true when some data is missing
  Q_PROPERTY(bool missing READ missing NOTIFY missingChanged)

  /// \brief current selected map
  Q_PROPERTY(QString selectedMap READ selectedMap NOTIFY selectedMapChanged)

  inline bool deleting() const
  { return qvariant_cast< bool >(property("deleting")); }

  inline bool downloading() const
  { return qvariant_cast< bool >(property("downloading")); }

  inline bool missing() const
  { return qvariant_cast< bool >(property("missing")); }

  inline bool ready() const
  { return qvariant_cast< bool >(property("ready")); }

  inline QString selectedMap() const
  { return qvariant_cast< QString >(property("selectedMap")); }

  inline bool storageAvailable() const
  { return qvariant_cast< bool >(property("storageAvailable")); }

public Q_SLOTS: // METHODS
  inline void addCountry(const QString &id)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(id);
    asyncCallWithArgumentList(QStringLiteral("addCountry"), argumentList);
  }

  inline bool checkProvidedAvailable()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<bool> v =
        asyncCallWithArgumentList(QStringLiteral("checkProvidedAvailable"), argumentList);
    return qvariant_cast< bool >(v.value());
  }

  inline void checkStorageAvailable()
  {
    QList<QVariant> argumentList;
    asyncCallWithArgumentList(QStringLiteral("checkStorageAvailable"), argumentList);
  }

  inline bool createDirectory(const QString &path)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(path);
    QDBusPendingReply<bool> v =
        asyncCallWithArgumentList(QStringLiteral("createDirectory"), argumentList);
    return qvariant_cast< bool >(v.value());
  }

  inline QString defaultStorageDirectory()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<QString> v =
        asyncCallWithArgumentList(QStringLiteral("defaultStorageDirectory"), argumentList);
    return qvariant_cast< QString >(v.value());
  }

  inline bool deleteNonNeededFiles(const QStringList &files)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(files);
    QDBusPendingReply<bool> v =
        asyncCallWithArgumentList(QStringLiteral("deleteNonNeededFiles"), argumentList);
    return qvariant_cast< bool >(v.value());
  }

  inline QString getAvailableCountries()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<QString> v =
        asyncCallWithArgumentList(QStringLiteral("getAvailableCountries"), argumentList);
    return qvariant_cast< QString >(v.value());
  }

  inline bool getCountries()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<bool> v =
        asyncCallWithArgumentList(QStringLiteral("getCountries"), argumentList);
    return qvariant_cast< bool >(v.value());
  }

  inline QString getCountryDetails(const QString &id)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(id);
    QDBusPendingReply<QString> v =
        asyncCallWithArgumentList(QStringLiteral("getCountryDetails"), argumentList);
    return qvariant_cast< QString >(v.value());
  }

  inline QStringList getDirsWithNonNeededFiles()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<QStringList> v =
        asyncCallWithArgumentList(QStringLiteral("getDirsWithNonNeededFiles"), argumentList);
    return qvariant_cast< QStringList >(v.value());
  }

  inline QStringList getNonNeededFilesList()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<QStringList> v =
        asyncCallWithArgumentList(QStringLiteral("getNonNeededFilesList"), argumentList);
    return qvariant_cast< QStringList >(v.value());
  }

  inline qlonglong getNonNeededFilesSize()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<qlonglong> v =
        asyncCallWithArgumentList(QStringLiteral("getNonNeededFilesSize"), argumentList);
    return qvariant_cast< qlonglong >(v.value());
  }

  inline QString getProvidedCountries()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<QString> v =
        asyncCallWithArgumentList(QStringLiteral("getProvidedCountries"), argumentList);
    return qvariant_cast< QString >(v.value());
  }

  inline QString getRequestedCountries()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<QString> v =
        asyncCallWithArgumentList(QStringLiteral("getRequestedCountries"), argumentList);
    return qvariant_cast< QString >(v.value());
  }

  inline void getUpdates()
  {
    QList<QVariant> argumentList;
    asyncCallWithArgumentList(QStringLiteral("getUpdates"), argumentList);
  }

  inline bool isCountryAvailable(const QString &id)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(id);
    QDBusPendingReply<bool> v =
        asyncCallWithArgumentList(QStringLiteral("isCountryAvailable"), argumentList);
    return qvariant_cast< bool >(v.value());
  }

  inline bool isCountryCompatible(const QString &id)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(id);
    QDBusPendingReply<bool> v =
        asyncCallWithArgumentList(QStringLiteral("isCountryCompatible"), argumentList);
    return qvariant_cast< bool >(v.value());
  }

  inline bool isCountryRequested(const QString &id)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(id);
    QDBusPendingReply<bool> v =
        asyncCallWithArgumentList(QStringLiteral("isCountryRequested"), argumentList);
    return qvariant_cast< bool >(v.value());
  }

  inline QString missingInfo()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<QString> v =
        asyncCallWithArgumentList(QStringLiteral("missingInfo"), argumentList);
    return qvariant_cast< QString >(v.value());
  }

  inline void rmCountry(const QString &id)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(id);
    asyncCallWithArgumentList(QStringLiteral("rmCountry"), argumentList);
  }

  inline void stopDownload()
  {
    QList<QVariant> argumentList;
    asyncCallWithArgumentList(QStringLiteral("stopDownload"), argumentList);
  }

  inline bool updateProvided()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<bool> v =
        asyncCallWithArgumentList(QStringLiteral("updateProvided"), argumentList);
    return qvariant_cast< bool >(v.value());
  }

  inline QString updatesFound()
  {
    QList<QVariant> argumentList;
    QDBusPendingReply<QString> v =
        asyncCallWithArgumentList(QStringLiteral("updatesFound"), argumentList);
    return qvariant_cast< QString >(v.value());
  }

Q_SIGNALS: // SIGNALS
  void availabilityChanged();
  void deletingChanged(bool state);
  void downloadProgress(const QString &info);
  void downloadingChanged(bool state);
  void errorMessage(const QString &info);
  void missingChanged(bool missing);
  void missingInfoChanged(const QString &info);
  void readyChanged(bool ready);
  void selectedMapChanged(const QString &selected);
  void storageAvailableChanged(bool available);
  void subscriptionChanged();
  void updatesForDataFound(const QString &info);
};

namespace org {
  namespace osm {
    namespace scout {
      namespace server1 {
        typedef ::MapManager manager;
      }
    }
  }
}
#endif
