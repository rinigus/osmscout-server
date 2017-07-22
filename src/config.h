#ifndef CONFIG_H
#define CONFIG_H

// global configuration settings

#define GENERAL_SETTINGS "server-general/"

#define MAPMANAGER_SETTINGS "maps/"

#define OSM_SETTINGS "libosmscout/"
#define ROUTING_SPEED_SETTINGS "libosmscout-speed/"

#define GEOMASTER_SETTINGS "geocoder-nlp/"

#define MAPNIKMASTER_SETTINGS "mapnik/"

#define VALHALLA_MASTER_SETTINGS "valhalla/"

//////////////////////////////////////////////
/// global variables

#include "dbmaster.h"
#include "geomaster.h"
#include "mapnikmaster.h"
#include "valhallamaster.h"

#include <atomic>

extern DBMaster *osmScoutMaster;

extern GeoMaster *geoMaster;

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
