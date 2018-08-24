#!/bin/sh

set -e

echo "Build start time: `date`"

CFG=""

if [ -z ${OSMSCOUT+x} ];
then
    CFG+="disable_osmscout "
fi
if [ -z ${MAPNIK+x} ];
then
    CFG+="disable_mapnik "
fi
if [ -z ${SYSTEMD+x} ];
then
    CFG+="disable_systemd "
fi
if [ -z ${VALHALLA+x} ];
then
    CFG+="disable_valhalla "
fi

echo "Configuration:" $CFG

ln -s pro/osmscout-server_console.pro .
qmake CONFIG+="$CFG" osmscout-server_console.pro
make

echo "Build end time: `date`"
