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
}

sync_style "$SCRIPTPATH/mapnik-styles-sqlite/install/OSMBright/"          "$STYLE"/default/day
sync_style "$SCRIPTPATH/mapnik-styles-sqlite/install/MidnightCommander/"  "$STYLE"/default/night

