#!/bin/sh

set -e

echo "Build start time: `date`"

qmake CONFIG+=disable_mapnik osmscout-server_console.pro
make

echo "Build end time: `date`"
