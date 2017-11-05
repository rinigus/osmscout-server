#!/bin/bash
set -e

PROGPATH=$(dirname "$0")
source $PROGPATH/env.sh

wget -O pgfutter https://github.com/lukasmartinelli/pgfutter/releases/download/v1.1/pgfutter_linux_amd64
chmod +x pgfutter

wget https://imposm.org/static/rel/imposm3-0.4.0dev-20170519-3f00374-linux-x86-64.tar.gz
tar xvf imposm3-0.4.0dev-20170519-3f00374-linux-x86-64.tar.gz
mv imposm3-0.4.0dev-20170519-3f00374-linux-x86-64 imposm3

mkdir -p openmaptiles
for i in https://github.com/openmaptiles/openmaptiles.git \
  https://github.com/openmaptiles/import-lakelines.git \
  https://github.com/openmaptiles/import-natural-earth.git \
  https://github.com/rinigus/import-osmborder.git \
  https://github.com/openmaptiles/import-water.git \
  ; do
  (cd openmaptiles && git clone $i)
done
