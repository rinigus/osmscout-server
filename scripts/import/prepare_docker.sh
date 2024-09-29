#!/bin/bash

set -e

echo "Prepare directories for Docker-based import"

source .env

for d in \
    $STORE_PLANET \
    $STORE_OUTPUT \
    $STORE_IMPORTED \
    $STORE_MBTILES \
    $STORE_NOMINATIM \
    $STORE_VALHALLA \
    $STORE_MISC
do
    echo Make directory $d
    mkdir -p $d
done

echo Replace USER_ID and GROUP_ID with the current user data: `id -u`:`id -g`

sed -i "/^USER_ID=/c\USER_ID=$(id -u)" .env
sed -i "/^GROUP_ID=/c\GROUP_ID=$(id -g)" .env
