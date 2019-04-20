#!/bin/bash

set -e

echo "Build start time: `date`"

CFG=""

if [ -z ${OSMSCOUT+x} ];
then
    CFG="$CFG disable_osmscout"
fi
if [ -z ${MAPNIK+x} ];
then
    CFG="$CFG disable_mapnik"
fi
if [ -z ${SYSTEMD+x} ];
then
    CFG="$CFG disable_systemd"
fi
if [ -z ${VALHALLA+x} ];
then
    CFG="$CFG disable_valhalla"
fi

echo "Configuration:" $CFG

if [ -z ${GUI+x} ];
then
    FLAVOR=console
else
    FLAVOR=qtcontrols
fi

echo "Building for" $FLAVOR

qmake CONFIG+="$CFG" SCOUT_FLAVOR=$FLAVOR
make

echo "Build end time: `date`"
