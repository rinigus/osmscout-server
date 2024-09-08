#!/bin/bash

# script for postprocessing MBTiles and Valhalla tiles
# prep for geocoder-nlp import

# this script is an entry point for the corresponding Docker image

IMPORT=/import

MBTILES_INPUT=/mapbox-planet/output.mbtiles
MBTILES_TILESDIR=/mapbox-planet/tiles

if [ ! -f "$MBTILES_TILESDIR" ]; then
    echo MBTILES: splitting planet into smaller databases
    python ./mapbox_planetiler_split.py \
        $MBTILES_INPUT hierarchy $MBTILES_TILESDIR
else
    echo MBTILES directory exists. Skipping split tiles generation.
fi
