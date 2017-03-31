#!/bin/bash

set -e

rm -f planet/planet-latest.osm.pbf
wget -O planet/planet-latest.osm.pbf http://ftp5.gwdg.de/pub/misc/openstreetmap/planet.openstreetmap.org/pbf/planet-latest.osm.pbf

rm -rf splitted
./prepare_splitter.py
nice -n 19 make -f Makefile.splitter

./prepare_countries.py
nice -n 19 make -f Makefile.import -j8

./prepare_distribution.py
#./uploader.sh

