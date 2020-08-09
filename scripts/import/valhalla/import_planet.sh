#!/bin/bash

set -e

PROGPATH=`realpath $(dirname "$0")`
PLANET=`realpath "$1"`

cd $PROGPATH

rm -rf tiles *.bin
mkdir tiles

tz_location=$(cat valhalla.json | jq -r .mjolnir.timezone)
install-dist/valhalla_build_timezones > ${tz_location}

## should be set after timezones
#export LD_LIBRARY_PATH=$PROGPATH/install/lib64

install/bin/valhalla_build_admins --config valhalla.json $PLANET
install/bin/valhalla_build_tiles --config valhalla.json $PLANET

# gzip all tiles
gzip -r tiles/?
