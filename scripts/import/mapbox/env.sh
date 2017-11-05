#!/bin/bash

# setup environment variables used by import scripts

POSTGRES_USER=$USER
POSTGRES_PASSWORD=

POSTGRES_DB=osm
POSTGRES_HOST=`pwd`/pg-socket
POSTGRES_PORT=35432
