#!/bin/sh

set -e

echo "Build start time: `date`"

qmake osmscout-server_console.pro
make

echo "Build end time: `date`"
