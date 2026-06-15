#!/bin/bash

set -e

echo "Prepare directories for Docker or Podman based import"

SKIP_IMAGES=false
while [ "$#" -gt 0 ]; do
  case "$1" in
    --skip-images)
      SKIP_IMAGES=true
      ;;
    *)
      message "Unknown option: $1"
      exit 1
      ;;
  esac
  shift
done

# init
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# shellcheck disable=SC1091
source "${SCRIPT_DIR}/podman-import-common.sh"

load_import_env .env

# making directories
for d in \
    $STORE_PLANET \
    $STORE_OUTPUT \
    $STORE_IMPORTED \
    $STORE_MBTILES \
    $STORE_NOMINATIM \
    $STORE_NOMINATIM_DB \
    $STORE_NOMINATIM_FLAT \
    $STORE_VALHALLA \
    $STORE_MISC
do
    echo Make directory $d
    mkdir -p $d
done

echo Replace USER_ID and GROUP_ID with the current user data: `id -u`:`id -g`

sed -i "/^USER_ID=/c\USER_ID=$(id -u)" .env
sed -i "/^GROUP_ID=/c\GROUP_ID=$(id -g)" .env

if [ "$SKIP_IMAGES" = true ]; then
  exit 0
fi

# prepare images

build_image_if_missing "$WGET_IMAGE" \
  -f Dockerfile.wget \
  .

build_image_if_missing "$NOMINATIM_GIS_IMAGE" \
  -f nominatim/gis/Dockerfile \
  nominatim/gis

build_image_if_missing "$NOMINATIM_FEED_IMAGE" \
  -f nominatim/feed/Dockerfile \
  nominatim/feed

build_image_if_missing "$VALHALLA_TILES2PACKS_IMAGE" \
  --build-arg "VALHALLA_VERSION=${VALHALLA_VERSION}" \
  -f valhalla/Dockerfile.tiles2packs \
  valhalla

build_image_if_missing "$POSTPROCESS_IMAGE" \
  -f Dockerfile.postprocess \
  .
