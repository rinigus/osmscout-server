#!/bin/bash

set -e

BNDSRC=`pwd`/backends-source
BNDINSTALL=`pwd`/backends-install

MAPSTYLE=$BNDSRC/geocoder-nlp/importer/stylesheet/map.ost
IMPORTER=$BNDINSTALL/bin/geocoder-importer-pbf

export LD_LIBRARY_PATH=$BNDINSTALL/lib

if [ $# -lt 5 ] ; then
	echo "Usage: $0 mapfile_pbf base_dir continent_country country_2_letter_code external_postcodes"
	exit 1
fi

PBF=$1
BASE_DIR=$2
CONTINENT_COUNTRY=$3
COUNTRY_CODE=$4
POSTCODES=$5

SQLDIR="$BASE_DIR/geocoder-nlp/$CONTINENT_COUNTRY"

rm -rf "$SQLDIR"
mkdir -p "$SQLDIR"

"$IMPORTER" "$PBF" "$SQLDIR" "$POSTCODES" "$COUNTRY_CODE"

./pack.sh "$SQLDIR" `backends-install/bin/geocoder-importer --version`
