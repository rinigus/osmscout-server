/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "appsettings.h"

/*
 * Implementation of interface class AppSettings
 */

AppSettings::AppSettings(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
  connect(this, &AppSettings::reloadData,
          &AppSettings::countrySelectionNeededChanged);
  connect(this, &AppSettings::reloadData,
          &AppSettings::hasBackendMapnikChanged);
  connect(this, &AppSettings::reloadData,
          &AppSettings::hasBackendOsmScoutChanged);
  connect(this, &AppSettings::reloadData,
          &AppSettings::hasBackendSystemDChanged);
  connect(this, &AppSettings::reloadData,
          &AppSettings::hasBackendValhallaChanged);
  connect(this, &AppSettings::reloadData,
          &AppSettings::osmScoutSettingsChanged);
  connect(this, &AppSettings::reloadData,
          &AppSettings::profilesUsedChanged);
}

AppSettings::~AppSettings()
{
}

