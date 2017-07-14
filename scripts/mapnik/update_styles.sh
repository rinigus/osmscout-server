#!/bin/bash

set -e

D=`pwd`

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

STYLE=`readlink -f $SCRIPTPATH/../../mapnik`

RSYNC_OPTIONS='-av --exclude "*.mss" --exclude "*.mml"'

echo "Generation of Mapnik styles directories under $STYLE"

cd "$SCRIPTPATH/mapnik-styles-sqlite"
./make.py

cd "$D"

rm -rf "$STYLE"

sync_style () {
    mkdir -p $2
    rsync -av --exclude "*.mss" --exclude "*.mml" $1 $2
    (cd $2 && ln -s ../../res/icons .)
    (cd $2 && ln -s ../../res/img .)
    (cd $2 && ln -s ../../res/shield-day .)
    (cd $2 && ln -s ../../res/shield-night .)
}

sync_style "$SCRIPTPATH/mapnik-styles-sqlite/install/OSMBright/"          "$STYLE"/default/day
sync_style "$SCRIPTPATH/mapnik-styles-sqlite/install/MidnightCommander/"  "$STYLE"/default/night

sync_style "$SCRIPTPATH/mapnik-styles-sqlite/install/OSMBrightCar/"          "$STYLE"/car/day
sync_style "$SCRIPTPATH/mapnik-styles-sqlite/install/MidnightCommanderCar/"  "$STYLE"/car/night

sync_style "$SCRIPTPATH/mapnik-styles-sqlite/install/OSMBrightEnglish/"          "$STYLE"/default/day-en

rsync -av "$SCRIPTPATH/mapnik-styles-sqlite/res" "$STYLE"/
