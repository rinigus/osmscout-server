/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2016-2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "modulechecker.h"

/*
 * Implementation of interface class ModuleChecker
 */

ModuleChecker::ModuleChecker(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

ModuleChecker::~ModuleChecker()
{
}

