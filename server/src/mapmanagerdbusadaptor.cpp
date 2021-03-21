/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "mapmanagerdbusadaptor.h"

#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

using namespace MapManager;

/*
 * Implementation of adaptor class ManagerDBusAdaptor
 */

ManagerDBusAdaptor::ManagerDBusAdaptor(QObject *parent)
  : QDBusAbstractAdaptor(parent)
{
  // constructor
  setAutoRelaySignals(true);
}

ManagerDBusAdaptor::~ManagerDBusAdaptor()
{
  // destructor
}

bool ManagerDBusAdaptor::deleting() const
{
  // get the value of property deleting
  return qvariant_cast< bool >(parent()->property("deleting"));
}

bool ManagerDBusAdaptor::downloading() const
{
  // get the value of property downloading
  return qvariant_cast< bool >(parent()->property("downloading"));
}

bool ManagerDBusAdaptor::missing() const
{
  // get the value of property missing
  return qvariant_cast< bool >(parent()->property("missing"));
}

bool ManagerDBusAdaptor::ready() const
{
  // get the value of property ready
  return qvariant_cast< bool >(parent()->property("ready"));
}

QString ManagerDBusAdaptor::selectedMap() const
{
  // get the value of property selectedMap
  return qvariant_cast< QString >(parent()->property("selectedMap"));
}

bool ManagerDBusAdaptor::storageAvailable() const
{
  // get the value of property storageAvailable
  return qvariant_cast< bool >(parent()->property("storageAvailable"));
}

void ManagerDBusAdaptor::addCountry(const QString &id)
{
  // handle method call org.osm.scout.server1.manager.addCountry
  QMetaObject::invokeMethod(parent(), "addCountry", Q_ARG(QString, id));
}

bool ManagerDBusAdaptor::checkProvidedAvailable()
{
  // handle method call org.osm.scout.server1.manager.checkProvidedAvailable
  bool out0;
  QMetaObject::invokeMethod(parent(), "checkProvidedAvailable", Q_RETURN_ARG(bool, out0));
  return out0;
}

void ManagerDBusAdaptor::checkStorageAvailable()
{
  // handle method call org.osm.scout.server1.manager.checkStorageAvailable
  QMetaObject::invokeMethod(parent(), "checkStorageAvailable");
}

bool ManagerDBusAdaptor::createDirectory(const QString &path)
{
  // handle method call org.osm.scout.server1.manager.createDirectory
  bool out0;
  QMetaObject::invokeMethod(parent(), "createDirectory", Q_RETURN_ARG(bool, out0), Q_ARG(QString, path));
  return out0;
}

QString ManagerDBusAdaptor::defaultStorageDirectory()
{
  // handle method call org.osm.scout.server1.manager.defaultStorageDirectory
  QString out0;
  QMetaObject::invokeMethod(parent(), "defaultStorageDirectory", Q_RETURN_ARG(QString, out0));
  return out0;
}

bool ManagerDBusAdaptor::deleteNonNeededFiles(const QStringList &files)
{
  // handle method call org.osm.scout.server1.manager.deleteNonNeededFiles
  bool out0;
  QMetaObject::invokeMethod(parent(), "deleteNonNeededFiles", Q_RETURN_ARG(bool, out0), Q_ARG(QStringList, files));
  return out0;
}

QString ManagerDBusAdaptor::getAvailableCountries()
{
  // handle method call org.osm.scout.server1.manager.getAvailableCountries
  QString out0;
  QMetaObject::invokeMethod(parent(), "getAvailableCountries", Q_RETURN_ARG(QString, out0));
  return out0;
}

bool ManagerDBusAdaptor::getCountries()
{
  // handle method call org.osm.scout.server1.manager.getCountries
  bool out0;
  QMetaObject::invokeMethod(parent(), "getCountries", Q_RETURN_ARG(bool, out0));
  return out0;
}

QString ManagerDBusAdaptor::getCountryDetails(const QString &id)
{
  // handle method call org.osm.scout.server1.manager.getCountryDetails
  QString out0;
  QMetaObject::invokeMethod(parent(), "getCountryDetails", Q_RETURN_ARG(QString, out0), Q_ARG(QString, id));
  return out0;
}

QStringList ManagerDBusAdaptor::getDirsWithNonNeededFiles()
{
  // handle method call org.osm.scout.server1.manager.getDirsWithNonNeededFiles
  QStringList out0;
  QMetaObject::invokeMethod(parent(), "getDirsWithNonNeededFiles", Q_RETURN_ARG(QStringList, out0));
  return out0;
}

QStringList ManagerDBusAdaptor::getNonNeededFilesList()
{
  // handle method call org.osm.scout.server1.manager.getNonNeededFilesList
  QStringList out0;
  QMetaObject::invokeMethod(parent(), "getNonNeededFilesList", Q_RETURN_ARG(QStringList, out0));
  return out0;
}

qlonglong ManagerDBusAdaptor::getNonNeededFilesSize()
{
  // handle method call org.osm.scout.server1.manager.getNonNeededFilesSize
  qlonglong out0;
  QMetaObject::invokeMethod(parent(), "getNonNeededFilesSize", Q_RETURN_ARG(qlonglong, out0));
  return out0;
}

QString ManagerDBusAdaptor::getProvidedCountries()
{
  // handle method call org.osm.scout.server1.manager.getProvidedCountries
  QString out0;
  QMetaObject::invokeMethod(parent(), "getProvidedCountries", Q_RETURN_ARG(QString, out0));
  return out0;
}

QString ManagerDBusAdaptor::getRequestedCountries()
{
  // handle method call org.osm.scout.server1.manager.getRequestedCountries
  QString out0;
  QMetaObject::invokeMethod(parent(), "getRequestedCountries", Q_RETURN_ARG(QString, out0));
  return out0;
}

void ManagerDBusAdaptor::getUpdates()
{
  // handle method call org.osm.scout.server1.manager.getUpdates
  QMetaObject::invokeMethod(parent(), "getUpdates");
}

bool ManagerDBusAdaptor::isCountryAvailable(const QString &id)
{
  // handle method call org.osm.scout.server1.manager.isCountryAvailable
  bool out0;
  QMetaObject::invokeMethod(parent(), "isCountryAvailable", Q_RETURN_ARG(bool, out0), Q_ARG(QString, id));
  return out0;
}

bool ManagerDBusAdaptor::isCountryCompatible(const QString &id)
{
  // handle method call org.osm.scout.server1.manager.isCountryCompatible
  bool out0;
  QMetaObject::invokeMethod(parent(), "isCountryCompatible", Q_RETURN_ARG(bool, out0), Q_ARG(QString, id));
  return out0;
}

bool ManagerDBusAdaptor::isCountryRequested(const QString &id)
{
  // handle method call org.osm.scout.server1.manager.isCountryRequested
  bool out0;
  QMetaObject::invokeMethod(parent(), "isCountryRequested", Q_RETURN_ARG(bool, out0), Q_ARG(QString, id));
  return out0;
}

QString ManagerDBusAdaptor::missingInfo()
{
  // handle method call org.osm.scout.server1.manager.missingInfo
  QString out0;
  QMetaObject::invokeMethod(parent(), "missingInfo", Q_RETURN_ARG(QString, out0));
  return out0;
}

void ManagerDBusAdaptor::rmCountry(const QString &id)
{
  // handle method call org.osm.scout.server1.manager.rmCountry
  QMetaObject::invokeMethod(parent(), "rmCountry", Q_ARG(QString, id));
}

void ManagerDBusAdaptor::stopDownload()
{
  // handle method call org.osm.scout.server1.manager.stopDownload
  QMetaObject::invokeMethod(parent(), "stopDownload");
}

bool ManagerDBusAdaptor::updateProvided()
{
  // handle method call org.osm.scout.server1.manager.updateProvided
  bool out0;
  QMetaObject::invokeMethod(parent(), "updateProvided", Q_RETURN_ARG(bool, out0));
  return out0;
}

QString ManagerDBusAdaptor::updatesFound()
{
  // handle method call org.osm.scout.server1.manager.updatesFound
  QString out0;
  QMetaObject::invokeMethod(parent(), "updatesFound", Q_RETURN_ARG(QString, out0));
  return out0;
}

