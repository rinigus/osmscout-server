/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
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

#ifndef CONFIG_H
#define CONFIG_H

#ifndef APP_NAME
#define APP_NAME "osmscout-server"
#endif

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
#define DBUS_SERVICE "org.osm.scout.server1"
#define DBUS_PATH_ROOT "/org/osm/scout/server1"
#define DBUS_INTERFACE_ROOT "org.osm.scout.server1"

#define DBUS_PATH_INFOHUB DBUS_PATH_ROOT "/infohub"
#define DBUS_INTERFACE_INFOHUB DBUS_INTERFACE_ROOT ".infohub"

#define DBUS_PATH_LOGGER DBUS_PATH_ROOT "/logger"
#define DBUS_INTERFACE_LOGGER DBUS_INTERFACE_ROOT ".logger"

#define DBUS_PATH_MANAGER DBUS_PATH_ROOT "/manager"
#define DBUS_INTERFACE_MANAGER DBUS_INTERFACE_ROOT ".manager"

#define DBUS_PATH_MODULES DBUS_PATH_ROOT "/modules"
#define DBUS_INTERFACE_MODULES DBUS_INTERFACE_ROOT ".modules"

#define DBUS_PATH_MAPMATCHING DBUS_PATH_ROOT "/mapmatching1"
#define DBUS_INTERFACE_MAPMATCHING DBUS_INTERFACE_ROOT ".mapmatching1"

#define DBUS_PATH_SETTINGS DBUS_PATH_ROOT "/settings"
#define DBUS_INTERFACE_SETTINGS DBUS_INTERFACE_ROOT ".settings"

#define DBUS_PATH_SYSTEMD DBUS_PATH_ROOT "/systemd_service"
#define DBUS_INTERFACE_SYSTEMD DBUS_INTERFACE_ROOT ".systemd_service"

//////////////////////////////////////////////
/// global variables

#include "dbmaster.h"
#include "geomaster.h"
#include "mapboxglmaster.h"
#include "mapnikmaster.h"
#include "valhallamaster.h"

#include <atomic>

#ifdef USE_OSMSCOUT
extern DBMaster *osmScoutMaster;
#endif

extern GeoMaster *geoMaster;

extern MapboxGLMaster *mapboxglMaster;

#ifdef USE_MAPNIK
extern MapnikMaster *mapnikMaster;
#endif

#ifdef USE_VALHALLA
extern ValhallaMaster *valhallaMaster;
#endif

extern std::atomic<bool> useGeocoderNLP;
extern std::atomic<bool> useMapnik;
extern std::atomic<bool> useValhalla;

#endif // CONFIG_H
