#!/bin/bash

# script for postprocessing MBTiles and Valhalla tiles
# prep for geocoder-nlp import

# this script is an entry point for the corresponding Docker image

set -e

# Directories should be in sync with Dockerfile.postprocess
# to be sure that *_country_pack.py would work
# IMPORT=/import

# MBTILES_INPUT=/mapbox-planet/output.mbtiles
# MBTILES_TILESDIR=/mapbox-planet/tiles
# MBTILES_IMPORT_META=/mapbox-planet/packages-meta
# MBTILES_IMPORT=$IMPORT/mapboxgl/packages

# VALHALLA_TILESDIR=/valhalla/valhalla_tiles
# VALHALLA_TILES_CONF=/valhalla/tiles_in_packages.json
# VALHALLA_IMPORT_META=/valhalla/packages-meta
# VALHALLA_IMPORT=$IMPORT/valhalla/packages


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
    python ./mapbox_scripts/make_packs.py
else
    echo MBTILES packaged for OSM Scout Server already in $MBTILES_IMPORT
fi

echo Mapbox GL glyphs imported and already packed, skipping in this script

# Valhalla

# gzip all uncompressed tiles
echo Compress all tiles after import by Valhalla if needed
find $VALHALLA_TILESDIR/? -type f ! -name "*.gz" -exec gzip {} \;

if [ ! -d "$VALHALLA_IMPORT" ]; then
    echo Valhalla: packaging for OSM Scout Server
    echo Target folder: $VALHALLA_IMPORT
    mkdir -p $VALHALLA_IMPORT
    mkdir -p $VALHALLA_IMPORT_META
    python ./valhalla_scripts/make_packs.py
else
    echo Valhalla tiles packaged for OSM Scout Server already in $VALHALLA_IMPORT
fi

# prepare countries
MISC_DIR=/osmscout

# echo AAAAAAAAAAAAAAAAAAAAAAAAA
# rm $MISC_DIR/Makefile.import
# echo FIX ME FIX ME

if [ ! -f "$MISC_DIR/Makefile.import" ]; then
    echo "Generate Makefile for GeocoderNLP import and collect other data for finalizing import"
    python ./prepare_countries.py --output $MISC_DIR
else
    echo "Makefile for GeocoderNLP import found: $MISC_DIR/Makefile.import"
    echo "Skipping makefile generation"
fi
