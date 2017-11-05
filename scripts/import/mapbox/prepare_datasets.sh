#!/bin/bash
set -e

PROGPATH=$(dirname "$0")
source $PROGPATH/env.sh

mkdir -p $IMPORT_DATA_DIR

# water
wget -P $IMPORT_DATA_DIR http://data.openstreetmapdata.com/water-polygons-split-3857.zip
unzip -oj $IMPORT_DATA_DIR/water-polygons-split-3857.zip -d $IMPORT_DATA_DIR
wget -P $IMPORT_DATA_DIR http://data.openstreetmapdata.com/simplified-water-polygons-complete-3857.zip
unzip -oj $IMPORT_DATA_DIR/simplified-water-polygons-complete-3857.zip -d $IMPORT_DATA_DIR

# natural earth
wget -P $IMPORT_DATA_DIR http://naciscdn.org/naturalearth/packages/natural_earth_vector.sqlite.zip
unzip -oj $IMPORT_DATA_DIR/natural_earth_vector.sqlite.zip -d $IMPORT_DATA_DIR

# lake centerlines
wget -P $IMPORT_DATA_DIR https://github.com/lukasmartinelli/osm-lakelines/releases/download/v0.9/lake_centerline.geojson

# borders
wget -O $IMPORT_DIR/osmborder_lines.csv https://github.com/openmaptiles/import-osmborder/releases/download/v0.1/osmborder_lines.csv
