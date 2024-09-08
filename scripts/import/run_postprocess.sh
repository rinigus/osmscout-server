#!/bin/bash

# script for postprocessing MBTiles and Valhalla tiles
# prep for geocoder-nlp import

# this script is an entry point for the corresponding Docker image

set -e

IMPORT=/import

# MBTILES
MBTILES_INPUT=/mapbox-planet/output.mbtiles
MBTILES_TILESDIR=/mapbox-planet/tiles
MBTILES_IMPORT_META=/mapbox-planet/packages-meta
MBTILES_IMPORT=$IMPORT/mapboxgl/packages

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
    python ./mapbox/make_packs.py \
        --tiles=$MBTILES_TILESDIR \
        --packages=$MBTILES_IMPORT \
        --meta=$MBTILES_IMPORT_META
else
    echo MBTILES packaged for OSM Scout Server already in $MBTILES_IMPORT
fi

echo Mapbox GL glyphs imported and already packed, skipping in this script

# Valhalla

VALHALLA_TILESDIR=/valhalla/valhalla_tiles
VALHALLA_TILES_CONF=/valhalla/tiles_in_packages.json
VALHALLA_IMPORT_META=/valhalla/packages-meta
VALHALLA_IMPORT=$IMPORT/valhalla/packages

# gzip all uncompressed tiles
echo Compress all tiles after import by Valhalla if needed
find "$VALHALLA_TILESDIR" -type f ! -name "*.gz" -exec gzip {} \;

if [ ! -d "$VALHALLA_IMPORT" ]; then
    echo Valhalla: packaging for OSM Scout Server
    echo Target folder: $VALHALLA_IMPORT
    python ./valhalla/make_packs.py \
        --tiles=$VALHALLA_TILESDIR \
        --tiles-conf=$VALHALLA_TILES_CONF \
        --packages=$VALHALLA_IMPORT \
        --meta=$VALHALLA_IMPORT_META
else
    echo Valhalla tiles packaged for OSM Scout Server already in $VALHALLA_IMPORT
fi

# prepare countries
