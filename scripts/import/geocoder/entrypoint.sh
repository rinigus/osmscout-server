#!/bin/bash

# script for geocoder-nlp import
# this script is an entry point for the corresponding Docker image

set -e

IMPORT=/import
LIBPOSTAL_DATA=/planet_pbf/libpostal
GEOCODER_JOBS="${GEOCODER_JOBS:-$(nproc)}"
MISC_DIR=/misc
MODE="${1:---all}"

if [ "$#" -gt 1 ]; then
    echo "Usage: $0 [--all|--download]"
    exit 1
fi

case "$MODE" in
    --all|--download)
        ;;
    *)
        echo "Usage: $0 [--all|--download]"
        exit 1
        ;;
esac

# prepare temporary directory with the expected structure
RDIR=`mktemp -d`
SDIR=`pwd`

cd $RDIR

# notation from earlier scripts
ln -s $IMPORT distribution

ln -s $SDIR/* .

# download libpostal datasets
if [ ! -d "$LIBPOSTAL_DATA" ]; then
    echo Libpostal: downloading datasets
    echo Target folder: $LIBPOSTAL_DATA
    mkdir -p $LIBPOSTAL_DATA
    libpostal_data download all $LIBPOSTAL_DATA/libpostal
else
    echo Libpostal datasets directory $LIBPOSTAL_DATA exists, skipping downloads
fi

if [ "$MODE" = "--download" ]; then
    echo "Download step finished"
    cd $SDIR
    rm -rf $RDIR
    exit 0
fi

# prepare import makefile
mkdir -p "$MISC_DIR"
if [ ! -f "$MISC_DIR/Makefile.import" ]; then
    echo "Generate Makefile for GeocoderNLP import and collect other data for finalizing import"
    python ./prepare_import.py --output $MISC_DIR
else
    echo "Makefile for GeocoderNLP import found: $MISC_DIR/Makefile.import"
    echo "Skipping Makefile generation"
fi

echo Importing GeocoderNLP using $GEOCODER_JOBS jobs
make -f $MISC_DIR/Makefile.import -j$GEOCODER_JOBS

# finished: remove tmp directory
cd $SDIR
rm -rf $RDIR
