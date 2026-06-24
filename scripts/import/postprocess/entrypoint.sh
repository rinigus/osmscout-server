#!/bin/bash

# script for postprocessing MBTiles and Valhalla tiles
# prep for geocoder-nlp import

# this script is an entry point for the corresponding Docker image

set -e

IMPORT=/import

MBTILES_INPUT=/mapbox-planet/output.mbtiles
MBTILES_TILESDIR=/mapbox-planet/tiles
MBTILES_IMPORT_META=/mapbox-planet/packages-meta
MBTILES_IMPORT=$IMPORT/mapboxgl/packages

VALHALLA_TILESDIR=/valhalla/valhalla_tiles
VALHALLA_TILES_CONF=/valhalla/tiles_in_packages.json
VALHALLA_IMPORT_META=/valhalla/packages-meta
VALHALLA_IMPORT=$IMPORT/valhalla/packages

MISC_DIR=/osmscout

# prepare temporary directory with the expected structure
RDIR=`mktemp -d`
SDIR=`pwd`

cd $RDIR
mkdir mapbox && mkdir valhalla
ln -s $MBTILES_TILESDIR mapbox/tiles
ln -s $MBTILES_IMPORT mapbox/packages
ln -s $MBTILES_IMPORT_META mapbox/packages_meta

ln -s $VALHALLA_TILESDIR valhalla/tiles
ln -s $VALHALLA_IMPORT valhalla/packages
ln -s $VALHALLA_IMPORT_META valhalla/packages_meta
ln -s $VALHALLA_TILES_CONF valhalla/tiles_in_packages.json

# notation from earlier scripts
ln -s $IMPORT distribution

ln -s $SDIR/* .
export PYTHONPATH=$PYTHONPATH:`pwd`

# MBTILES
if [ ! -d "$MBTILES_TILESDIR" ]; then
    echo MBTILES: splitting planet into smaller databases
    echo Target folder: $MBTILES_TILESDIR
    python ./mapbox_planetiler_split.py \
        $MBTILES_INPUT hierarchy $MBTILES_TILESDIR
else
    echo MBTILES directory exists. Skipping split tiles generation.
fi

if [ ! -d "$MBTILES_IMPORT" ]; then
    echo MBTILES: packaging for OSM Scout Server
    echo Target folder: $MBTILES_IMPORT
    mkdir -p $MBTILES_IMPORT
    mkdir -p $MBTILES_IMPORT_META
    ./mapbox_packs.sh
else
    echo MBTILES packaged for OSM Scout Server already in $MBTILES_IMPORT
fi

echo Mapbox GL glyphs imported and already packed, skipping in this script

# Valhalla

# gzip all uncompressed tiles
echo Compress all tiles after import by Valhalla if needed
find $VALHALLA_TILESDIR/? -type f ! -name "*.gz" -print0 | xargs -0 -P "$(nproc)" -n 1 gzip

if [ ! -d "$VALHALLA_IMPORT" ]; then
    echo Valhalla: packaging for OSM Scout Server
    echo Target folder: $VALHALLA_IMPORT
    mkdir -p $VALHALLA_IMPORT
    mkdir -p $VALHALLA_IMPORT_META
    python ./valhalla_packs.py
else
    echo Valhalla tiles packaged for OSM Scout Server already in $VALHALLA_IMPORT
fi

# prepare countries
if [ ! -f "$MISC_DIR/countries.json" ]; then
    echo "Generate data for finalizing import"
    python ./prepare_countries.py --output $MISC_DIR
else
    echo "countries.json found: $MISC_DIR/countries.json"
    echo "Skipping data generation"
fi

# finished: remove tmp directory
cd $SDIR
rm -rf $RDIR
