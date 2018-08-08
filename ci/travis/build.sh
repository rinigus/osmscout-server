#!/bin/sh

set -e

echo "Build start time: `date`"

ln -s pro/osmscout-server_console.pro .
qmake CONFIG+="disable_mapnik disable_systemd disable_valhalla" osmscout-server_console.pro
make

echo "Build end time: `date`"
