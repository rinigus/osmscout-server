#!/bin/bash

set -e

PROGPATH=`realpath $(dirname "$0")`
PLANET=`realpath "$1"`

cd $PROGPATH

rm -rf tiles *.bin
mkdir tiles

install/valhalla_build_timezones valhalla.json

# should be set after timezones
export LD_LIBRARY_PATH=$PROGPATH/install

install/valhalla_build_admins --config valhalla.json $PLANET
install/valhalla_build_tiles --config valhalla.json $PLANET
