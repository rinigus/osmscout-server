#!/bin/bash

set -e

PROGPATH=`realpath $(dirname "$0")`
PLANET=`realpath "$1"`

cd $PROGPATH

rm -rf tiles *.bin
mkdir tiles

install-dist/valhalla_build_timezones valhalla.json > tiles/tz_world.sqlite

## should be set after timezones
export LD_LIBRARY_PATH=$PROGPATH/install-dist:$LD_LIBRARY_PATH
#export LD_LIBRARY_PATH=$PROGPATH/install/lib64

install-dist/valhalla_build_admins --config valhalla.json $PLANET
echo Admins done
echo
install-dist/valhalla_build_tiles --config valhalla.json $PLANET

# gzip all tiles
gzip -r tiles/?
