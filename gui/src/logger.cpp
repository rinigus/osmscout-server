/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "logger.h"

/*
 * Implementation of interface class Logger
 */

Logger::Logger(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
  connect(this, &Logger::reloadData, &Logger::logChanged);
}

Logger::~Logger()
{
}

