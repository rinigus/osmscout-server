#!/bin/bash

set -e

BNDSRC=`pwd`/backends-source
BNDINSTALL=`pwd`/backends-install

OSMSCOUT_IMPORTER=$BNDINSTALL/bin/Import
OSMSCOUT_MAPSTYLE=$BNDSRC/geocoder-nlp/importer/stylesheet/map.ost
GEOCODER_IMPORTER=$BNDINSTALL/bin/geocoder-importer

export LD_LIBRARY_PATH=$BNDINSTALL/lib

if [ $# -lt 5 ] ; then
	echo "Usage: $0 mapfile_pbf base_dir continent country country_2_letter_code"
	exit 1
fi

PBF=$1
BASE_DIR=$2
CONTINENT=$3
COUNTRY=$4
COUNTRY_CODE=$5

IMPDIR="$BASE_DIR/osmscout/$CONTINENT-$COUNTRY"
SQLDIR="$BASE_DIR/geocoder-nlp/$CONTINENT-$COUNTRY"
SQL="$SQLDIR/location.sqlite"

rm -rf "$IMPDIR" "$SQLDIR"
mkdir -p "$IMPDIR"
mkdir -p "$SQLDIR"

"$OSMSCOUT_IMPORTER" --typefile "$OSMSCOUT_MAPSTYLE" --delete-temporary-files true --delete-debugging-files true --delete-analysis-files true --delete-report-files true --destinationDirectory "$IMPDIR" "$PBF"

"$GEOCODER_IMPORTER" "$IMPDIR" "$SQL" "$COUNTRY_CODE"

./pack.sh "$IMPDIR" 10
./pack.sh "$SQLDIR" 1

