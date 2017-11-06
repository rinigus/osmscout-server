#!/bin/bash
set -e

PROGPATH=$(dirname "$0")
source $PROGPATH/env.sh

wget -O pgfutter https://github.com/lukasmartinelli/pgfutter/releases/download/v1.1/pgfutter_linux_amd64
chmod +x pgfutter

mkdir -p $GOPATH
go get github.com/omniscale/imposm3
go install github.com/omniscale/imposm3/cmd/imposm3

mkdir -p openmaptiles
for i in https://github.com/openmaptiles/openmaptiles.git \
  https://github.com/openmaptiles/import-lakelines.git \
  https://github.com/openmaptiles/import-natural-earth.git \
  https://github.com/rinigus/import-osmborder.git \
  https://github.com/openmaptiles/import-water.git \
  ; do
  (cd openmaptiles && git clone $i)
done
