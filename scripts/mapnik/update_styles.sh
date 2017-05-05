#!/bin/bash

set -e

D=`pwd`

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

STYLE=`readlink -f $SCRIPTPATH/../../mapnik`

echo "Generation of Mapnik styles directories under $STYLE"

cd "$SCRIPTPATH/mapnik-styles-sqlite"
mkdir -p install
./make.py
cd install/OSMBright
carto project.mml > mapnik.xml

cd "$D"

rm -rf "$STYLE"
mkdir -p "$STYLE"

rsync -av --exclude "*.mss" --exclude "*.mml" "$SCRIPTPATH/mapnik-styles-sqlite/install/" "$STYLE"
rsync -av "$SCRIPTPATH/mapnik-styles-sqlite/fonts" "$STYLE"

