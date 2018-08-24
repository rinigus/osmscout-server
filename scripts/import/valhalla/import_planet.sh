#!/bin/bash

set -e

PROGPATH=`realpath $(dirname "$0")`
PLANET=`realpath "$1"`

cd $PROGPATH

rm -rf tiles *.bin
mkdir tiles

install/bin/valhalla_build_timezones valhalla.json

install/bin/valhalla_build_admins --config valhalla.json $PLANET
install/bin/valhalla_build_tiles --config valhalla.json $PLANET

# gzip all tiles
gzip -r tiles/?
