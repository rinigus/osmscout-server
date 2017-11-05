#!/bin/bash
set -e

PROGPATH=$(dirname "$0")
source $PROGPATH/env.sh

openmaptiles/import-water/import-water.sh
openmaptiles/import-natural-earth/import-natural-earth.sh
openmaptiles/import-lakelines/import_lakelines.sh
openmaptiles/import-osmborder/import/import_osmborder_lines.sh
