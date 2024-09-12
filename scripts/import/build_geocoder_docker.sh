#!/bin/bash

set -e

PRIORITY=/app/geocoder-nlp/priority.list
IMPORTER=geocoder-importer

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
ERRORLOG="$BASE_DIR/geocoder-nlp/errors/$CONTINENT_COUNTRY"

rm -rf "$SQLDIR"
mkdir -p "$SQLDIR"

"$IMPORTER" --poly "$POLYJSON" --priority "$PRIORITY" --postal-country "$COUNTRY_CODE" --log-errors-to-file="$ERRORLOG" "$SQLDIR"

./pack.sh "$SQLDIR" `$IMPORTER --version`
