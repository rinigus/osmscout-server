/*
 * Copyright (C) 2019 Rinigus https://github.com/rinigus, 2019 Purism SPC
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

#include "dbusroot.h"
#include "infohub.h"

DBusRoot::DBusRoot(QString host, int port, QObject *parent):
  QObject(parent)
{
  m_url = QString("http://%1:%2").arg(host).arg(port);
}

void DBusRoot::Activate(QMap<QString, QVariant> platform_data)
{
  InfoHub::logInfo("DBus Activate called");
}

void DBusRoot::Open(QStringList uris, QMap<QString, QVariant> platform_data)
{
  InfoHub::logInfo("DBus Open called");
}

void DBusRoot::ActivateAction(QString action_name, QVariantList parameter, QMap<QString, QVariant> platform_data)
{
  InfoHub::logInfo("DBus ActivateAction called");
}
