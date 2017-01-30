#include "config.h"

////////////////////////////////////////////////
/// GLOBAL VARIABLES: ALLOCATION
///

DBMaster *osmScoutMaster = NULL;

GeoMaster *geoMaster = NULL;

std::atomic<bool> useGeocoderNLP{false};
