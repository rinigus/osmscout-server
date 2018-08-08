#!/bin/bash

set -e

BNDSRC=`pwd`/backends-source
BNDINSTALL=`pwd`/backends-install

OSMSCOUT_IMPORTER=$BNDINSTALL/bin/Import
OSMSCOUT_MAPSTYLE=$BNDSRC/libosmscout/stylesheets/map.ost

export LD_LIBRARY_PATH=$BNDINSTALL/lib

if [ $# -lt 4 ] ; then
	echo "Usage: $0 mapfile_pbf base_dir continent_country poly_file"
	exit 1
fi

PBF=$1
BASE_DIR=$2
CONTINENT_COUNTRY=$3
POLY_FILE=$4

IMPDIR="$BASE_DIR/osmscout/$CONTINENT_COUNTRY"

rm -rf "$IMPDIR" 
mkdir -p "$IMPDIR"

# workaround https://github.com/rinigus/osmscout-server/issues/148
USEPOLY="YES"
if [[ $POLY_FILE == *"russia/far-eastern-fed-district/chukotka"* ]]; then
  USEPOLY="NO"
fi

if [[ "$USEPOLY" == "YES" ]]; then
    # workaround [fixed upstream, can remove after update] https://github.com/Framstag/libosmscout/issues/332
    mkdir -p tmp_poly
    NPOLY=tmp_poly/$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1).poly
    cp "$POLY_FILE" $NPOLY
    "$OSMSCOUT_IMPORTER" --altLangOrder en --typefile "$OSMSCOUT_MAPSTYLE" --delete-temporary-files true --delete-debugging-files true --delete-analysis-files true --delete-report-files true --destinationDirectory "$IMPDIR" "$PBF" $NPOLY #"$POLY_FILE"
    rm $NPOLY
else
    echo "Import skipping POLY"
    "$OSMSCOUT_IMPORTER" --altLangOrder en --typefile "$OSMSCOUT_MAPSTYLE" --delete-temporary-files true --delete-debugging-files true --delete-analysis-files true --delete-report-files true --destinationDirectory "$IMPDIR" "$PBF" 
fi

# determined from libosmscout/include/osmscout/TypeConfig.h:  static const uint32_t FILE_FORMAT_VERSION=11
./pack.sh "$IMPDIR" `backends-install/bin/liboscmscout-version`
