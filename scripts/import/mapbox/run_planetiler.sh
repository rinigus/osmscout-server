#!/usr/bin/env bash

set -e

# Check options
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <pbf_folder> <area>"
  exit 1
fi

PBF_FOLDER=$1
AREA=$2

OUTPUT=/data/output.mbtiles
PLANETILER_STORAGE_TMP=${PLANETILER_STORAGE_TMP:-ram}

if [ -f $OUTPUT ]; then
  echo "Planet MBTiles are available already: skipping download and import"
  exit 0
fi

# determined from entrypoint of the docker image using docker inspect
java $JAVA_TOOL_OPTIONS -cp @/app/jib-classpath-file com.onthegomap.planetiler.Main \
  --bounds=planet \
  --download \
  --download_dir=${PBF_FOLDER} \
  --download-threads=10 --download-chunk-size-mb=1000 \
  --fetch-wikidata \
  --area=${AREA} \
  --output=${OUTPUT} \
  --nodemap-type=array --storage=${PLANETILER_STORAGE_TMP}
