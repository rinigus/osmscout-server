#!/bin/sh

set -e

echo "Build start time: `date`"

ln -s console/osmscout-server_console.pro .
qmake CONFIG+="" osmscout-server_console.pro
make

echo "Build end time: `date`"
