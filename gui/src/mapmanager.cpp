/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "mapmanager.h"

/*
 * Implementation of interface class MapManager
 */

MapManager::MapManager(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
  connect(this, &MapManager::reloadData, &MapManager::availabilityChanged);
  connect(this, &MapManager::reloadData, &MapManager::deletingChanged);
  connect(this, &MapManager::reloadData, &MapManager::downloadingChanged);
  connect(this, &MapManager::reloadData, &MapManager::missingChanged);
  connect(this, &MapManager::reloadData, &MapManager::readyChanged);
  connect(this, &MapManager::reloadData, &MapManager::selectedMapChanged);
  connect(this, &MapManager::reloadData, &MapManager::storageAvailableChanged);
  connect(this, &MapManager::reloadData, &MapManager::subscriptionChanged);
}

MapManager::~MapManager()
{
}

