#!/usr/bin/env bash

set -e

OUTPUT=/data/output.mbtiles
PLANETILER_STORAGE_TMP=${PLANETILER_STORAGE_TMP:-ram}

if [ -f $OUTPUT ]; then
  echo "Planet MBTiles are available already: skipping download and import"
  exit 0
fi

# determined from entrypoint of the docker image using docker inspect
java $JAVA_TOOL_OPTIONS -cp @/app/jib-classpath-file com.onthegomap.planetiler.Main \
  --bounds=planet \
  --fetch-wikidata \
  --output=${OUTPUT} \
  --nodemap-type=array \
  --storage=${PLANETILER_STORAGE_TMP} \
  "$@"
