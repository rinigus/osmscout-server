#!/bin/bash

set -e

BNDSRC=`pwd`/backends-source
BNDINSTALL=`pwd`/backends-install

PRIORITY=$BNDINSTALL/share/geocoder-nlp/data/priority.list
IMPORTER=$BNDINSTALL/bin/geocoder-importer

export LD_LIBRARY_PATH=$BNDINSTALL/lib

if [ $# -lt 5 ] ; then
	echo "Usage: $0 poly.json base_dir continent_country country_2_letter_code external_postcodes"
	exit 1
fi

POLYJSON=$1
BASE_DIR=$2
CONTINENT_COUNTRY=$3
COUNTRY_CODE=$4
POSTCODES=$5

SQLDIR="$BASE_DIR/geocoder-nlp/$CONTINENT_COUNTRY"

rm -rf "$SQLDIR"
mkdir -p "$SQLDIR"

"$IMPORTER" --poly "$POLYJSON" --priority "$PRIORITY" --postal-country "$COUNTRY_CODE" "$SQLDIR"

./pack.sh "$SQLDIR" `backends-install/bin/geocoder-importer --version`
