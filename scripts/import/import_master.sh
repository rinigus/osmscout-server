#!/bin/bash

set -e

#rm -f planet/planet-latest.osm.pbf
#(cd planet && wget http://ftp5.gwdg.de/pub/misc/openstreetmap/planet.openstreetmap.org/pbf/planet-latest.osm{.pbf,.pbf.md5} && md5sum -c *.md5)

# Valhalla import
valhalla/import_planet.sh planet/planet-latest.osm.pbf
valhalla/make_packs.py
rm -rf distribution/valhalla/packages
mkdir -p distribution/valhalla/packages
cp valhalla/packages/*tar.bz2 distribution/valhalla/packages/

rm -rf splitted
./prepare_splitter.py
nice -n 19 make -f Makefile.splitter

./prepare_countries.py
nice -n 19 make -f Makefile.import -j16

./prepare_distribution.py

./check_import.py

#./uploader.sh

