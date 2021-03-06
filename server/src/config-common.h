/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2016-2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CONFIGCOMMON_H
#define CONFIGCOMMON_H

// global configuration settings

#define GENERAL_SETTINGS "server-general/"

#define MAPMANAGER_SETTINGS "maps/"

#define OSM_SETTINGS "libosmscout/"
#define ROUTING_SPEED_SETTINGS "libosmscout-speed/"

#define GEOMASTER_SETTINGS "geocoder-nlp/"

#define MAPNIKMASTER_SETTINGS "mapnik/"

#define VALHALLA_MASTER_SETTINGS "valhalla/"

#define HTTP_SERVER_SETTINGS "http-listener/"

#define REQUEST_MAPPER_SETTINGS "request-mapper/"

#define GENERAL_APP_VERSION 4

// d-bus access
#define DBUS_SERVICE "io.github.rinigus.OSMScoutServer"
#define DBUS_PATH_ROOT "/io/github/rinigus/OSMScoutServer"
#define DBUS_INTERFACE_ROOT "io.github.rinigus.OSMScoutServer"

#define DBUS_PATH_GEOMASTER DBUS_PATH_ROOT "/geomaster"
#define DBUS_INTERFACE_GEOMASTER DBUS_INTERFACE_ROOT ".geomaster"

#define DBUS_PATH_INFOHUB DBUS_PATH_ROOT "/infohub"
#define DBUS_INTERFACE_INFOHUB DBUS_INTERFACE_ROOT ".infohub"

#define DBUS_PATH_LOGGER DBUS_PATH_ROOT "/logger"
#define DBUS_INTERFACE_LOGGER DBUS_INTERFACE_ROOT ".logger"

#define DBUS_PATH_MANAGER DBUS_PATH_ROOT "/manager"
#define DBUS_INTERFACE_MANAGER DBUS_INTERFACE_ROOT ".manager"

#define DBUS_PATH_MODULES DBUS_PATH_ROOT "/modules"
#define DBUS_INTERFACE_MODULES DBUS_INTERFACE_ROOT ".modules"

#define DBUS_PATH_MAPMATCHING DBUS_PATH_ROOT "/mapmatching"
#define DBUS_INTERFACE_MAPMATCHING DBUS_INTERFACE_ROOT ".mapmatching"

#define DBUS_PATH_SETTINGS DBUS_PATH_ROOT "/settings"
#define DBUS_INTERFACE_SETTINGS DBUS_INTERFACE_ROOT ".settings"

#define DBUS_PATH_SYSTEMD DBUS_PATH_ROOT "/systemd_service"
#define DBUS_INTERFACE_SYSTEMD DBUS_INTERFACE_ROOT ".systemd_service"


#endif // CONFIGCOMMON_H
