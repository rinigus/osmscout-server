#ifndef CONFIG_H
#define CONFIG_H

#ifdef IS_CONSOLE_QT
#define APP_PREFIX ""
#endif
#ifdef IS_SAILFISH_OS
#define APP_PREFIX "harbour-"
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

#define GENERAL_APP_VERSION 1

//////////////////////////////////////////////
/// global variables

#include "dbmaster.h"
#include "geomaster.h"
#include "mapboxglmaster.h"
#include "mapnikmaster.h"
#include "valhallamaster.h"

#include <atomic>

extern DBMaster *osmScoutMaster;

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
