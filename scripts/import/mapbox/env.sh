#!/bin/bash

# setup environment variables used by import scripts

export POSTGRES_USER=$USER
export POSTGRES_PASSWORD=test

export POSTGRES_DB=osm
export POSTGRES_HOST=`pwd`/pg-socket
export POSTGRES_PORT=35432

export IMPORT_DATA_DIR=`pwd`/data
export NATURAL_EARTH_DB=$IMPORT_DATA_DIR/natural_earth_vector.sqlite
export IMPORT_DIR=$IMPORT_DATA_DIR

export IMPOSM_CACHE_DIR=`pwd`/imposm-cache

export PATH=.:$PATH
