#include "config.h"

////////////////////////////////////////////////
/// GLOBAL VARIABLES: ALLOCATION
///

#ifdef USE_OSMSCOUT
DBMaster *osmScoutMaster = NULL;
#endif

GeoMaster *geoMaster = NULL;

MapboxGLMaster *mapboxglMaster = NULL;

#ifdef USE_MAPNIK
MapnikMaster *mapnikMaster = NULL;
#endif

#ifdef USE_VALHALLA
ValhallaMaster *valhallaMaster = NULL;
ValhallaMapMatcher *valhallaMapMatcher = NULL;
#endif

std::atomic<bool> useGeocoderNLP;
std::atomic<bool> useMapnik;
std::atomic<bool> useValhalla;
