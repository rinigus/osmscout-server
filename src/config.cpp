#include "config.h"

////////////////////////////////////////////////
/// GLOBAL VARIABLES: ALLOCATION
///

DBMaster *osmScoutMaster = NULL;

GeoMaster *geoMaster = NULL;

MapnikMaster *mapnikMaster = NULL;

std::atomic<bool> useGeocoderNLP;
std::atomic<bool> useMapnik;
