/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef MAPMANAGERDBUSADAPTOR_H
#define MAPMANAGERDBUSADAPTOR_H

#include "config.h"

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.osm.scout.server1.manager
 */
namespace MapManager {

  class ManagerDBusAdaptor: public QDBusAbstractAdaptor
  {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DBUS_INTERFACE_MANAGER)

  public:
    ManagerDBusAdaptor(QObject *parent);
    virtual ~ManagerDBusAdaptor();

  public: // PROPERTIES
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

    bool deleting() const;
    bool downloading() const;
    bool missing() const;
    bool ready() const;
    QString selectedMap() const;
    bool storageAvailable() const;

  public Q_SLOTS: // METHODS
    void addCountry(const QString &id);
    bool checkProvidedAvailable();
    void checkStorageAvailable();
    bool createDirectory(const QString &path);
    QString defaultStorageDirectory();
    bool deleteNonNeededFiles(const QStringList &files);
    QString getAvailableCountries();
    bool getCountries();
    QString getCountryDetails(const QString &id);
    QStringList getDirsWithNonNeededFiles();
    QStringList getNonNeededFilesList();
    qlonglong getNonNeededFilesSize();
    QString getProvidedCountries();
    QString getRequestedCountries();
    void getUpdates();
    bool isCountryAvailable(const QString &id);
    bool isCountryCompatible(const QString &id);
    bool isCountryRequested(const QString &id);
    QString missingInfo();
    void rmCountry(const QString &id);
    void stopDownload();
    bool updateProvided();
    QString updatesFound();

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
}

#endif
