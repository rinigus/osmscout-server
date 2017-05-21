#!/bin/bash

set -e

BNDINSTALL=`pwd`/backends-install

if [ $# -lt 3 ] ; then
	echo "Usage: $0 mapfile_pbf base_dir continent_country"
	exit 1
fi

PBF=$1
BASE_DIR=$2
COUNTRY=$3
SQLDIR="$BASE_DIR/mapnik/countries/$COUNTRY"

rm -rf "$SQLDIR"
mkdir -p "$SQLDIR"

$BNDINSTALL/mapnik-styles-sqlite/importer/import_pbf.sh "$PBF" "$SQLDIR/mapnik.sqlite"

./pack.sh "$SQLDIR" 2
