#ifndef CONFIG_H
#define CONFIG_H

// global configuration settings

#define MAPMANAGER_SETTINGS "maps/"

#define OSM_SETTINGS "libosmscout/"
#define ROUTING_SPEED_SETTINGS "libosmscout-speed/"

#define GEOMASTER_SETTINGS "geocoder-nlp/"

//////////////////////////////////////////////
/// global variables

#include "dbmaster.h"
#include "geomaster.h"
#include <atomic>

extern DBMaster *osmScoutMaster;

extern GeoMaster *geoMaster;

extern std::atomic<bool> useGeocoderNLP;

#endif // CONFIG_H
