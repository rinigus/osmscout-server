#!/bin/bash
set -e

PROGPATH=$(dirname "$0")
source $PROGPATH/env.sh

pbf_file="$1"
MAPPING_YAML=openmaptiles/openmaptiles/build/mapping.yaml

readonly PG_CONNECT="postgis: user=$POSTGRES_USER password=$POSTGRES_PASSWORD  host=$POSTGRES_HOST port=$POSTGRES_PORT dbname=$POSTGRES_DB"
echo $PG_CONNECT

mkdir -p "$IMPOSM_CACHE_DIR"

(cd openmaptiles/openmaptiles && make)

imposm3/imposm3 import \
        -connection "$PG_CONNECT" \
        -mapping "$MAPPING_YAML" \
        -overwritecache \
        -cachedir "$IMPOSM_CACHE_DIR" \
        -read "$pbf_file" \
        -deployproduction \
        -write
