/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2023 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "serverdbusroot.h"

ServerDBusRoot::ServerDBusRoot(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
  : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

ServerDBusRoot::~ServerDBusRoot()
{
}
