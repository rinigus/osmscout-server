#!/bin/bash

set -e

PLANET_TILES=mapbox/tiles
PACKAGES_DIR=mapbox/packages
PACKAGES_META=${PACKAGES_DIR}_meta
PACKAGES_TIMESTAMP=${PACKAGES_DIR}/timestamp
VERSION=3

date +'%Y-%m-%d_%H:%M' > "$PACKAGES_TIMESTAMP"

for tile in "$PLANET_TILES"/*sqlite; do
    bname=$(basename "$tile")
    bbox="$tile.bbox"

    cp "$tile" "$PACKAGES_DIR"

    if [ -e "$bbox" ]; then
        cp "$bbox" "$PACKAGES_META"
    fi

    cmd="./pack.sh ${PACKAGES_DIR}/${bname} ${VERSION}"
    echo "$cmd"
    ./pack.sh "${PACKAGES_DIR}/${bname}" "$VERSION"
done

echo "Made Mapbox GL packages"
