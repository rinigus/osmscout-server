#!/bin/bash

PROGPATH=$(dirname "$0")
source $PROGPATH/env.sh

createdb --host=$POSTGRES_DB --port=$POSTGRES_PORT --user=$POSTGRES_USER $POSTGRES_DB

psql --host=$POSTGRES_DB --port=$POSTGRES_PORT --user=$POSTGRES_USER --dbname="$db" <<-'EOSQL'
    CREATE EXTENSION postgis;
    CREATE EXTENSION hstore;
    CREATE EXTENSION unaccent;
    CREATE EXTENSION fuzzystrmatch;
    CREATE EXTENSION osml10n;
EOSQL
