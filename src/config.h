#ifndef CONFIG_H
#define CONFIG_H

// global configuration settings

#define MAPMANAGER_SETTINGS "maps/"

#define OSM_SETTINGS "libosmscout/"
#define ROUTING_SPEED_SETTINGS "libosmscout-speed/"

#define GEOMASTER_SETTINGS "geocoder-nlp/"

#define MAPNIKMASTER_SETTINGS "mapnik/"

//////////////////////////////////////////////
/// global variables

#include "dbmaster.h"
#include "geomaster.h"
#include "mapnikmaster.h"

#include <atomic>

extern DBMaster *osmScoutMaster;

extern GeoMaster *geoMaster;

extern MapnikMaster *mapnikMaster;

extern std::atomic<bool> useGeocoderNLP;
extern std::atomic<bool> useMapnik;

#endif // CONFIG_H
