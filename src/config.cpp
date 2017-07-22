#include "config.h"

////////////////////////////////////////////////
/// GLOBAL VARIABLES: ALLOCATION
///

DBMaster *osmScoutMaster = NULL;

GeoMaster *geoMaster = NULL;

#ifdef USE_MAPNIK
MapnikMaster *mapnikMaster = NULL;
#endif

#ifdef USE_VALHALLA
ValhallaMaster *valhallaMaster = NULL;
#endif

std::atomic<bool> useGeocoderNLP;
std::atomic<bool> useMapnik;
std::atomic<bool> useValhalla;
