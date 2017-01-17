#ifndef CONFIG_H
#define CONFIG_H

// global configuration settings

#define OSM_SETTINGS "libosmscout/"
#define ROUTING_SPEED_SETTINGS "libosmscout-speed/"

#define GEOMASTER_SETTINGS "geocoder-nlp/"

//////////////////////////////////////////////
/// global variables

#include "dbmaster.h"
#include "geomaster.h"

extern DBMaster *osmScoutMaster;

extern GeoMaster *geoMaster;

#endif // CONFIG_H
