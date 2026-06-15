#!/bin/bash

set -e

echo "Prepare directories for Docker or Podman based import"

# init
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# shellcheck disable=SC1091
source "${SCRIPT_DIR}/podman-import-common.sh"

usage() {
  cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Prepare directories and container images for Docker or Podman based import.

Options:
  --skip-images  Prepare directories and .env only; do not build images
  --rebuild      Rebuild images even if they already exist
  -h, --help     Show this help message and exit
EOF
}

SKIP_IMAGES=false
REBUILD_IMAGES=false

while [ "$#" -gt 0 ]; do
  case "$1" in
    --skip-images)
      SKIP_IMAGES=true
      ;;
    --rebuild)
      REBUILD_IMAGES=true
      ;;
    -h | --help)
      usage
      exit 0
      ;;
    --)
      shift
      break
      ;;
    -*)
      message "Unknown option: $1"
      usage >&2
      exit 1
      ;;
    *)
      message "Unexpected argument: $1"
      usage >&2
      exit 1
      ;;
  esac
  shift
done

if [ "$#" -gt 0 ]; then
  message "Unexpected argument: $1"
  usage >&2
  exit 1
fi

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

#########################
# prepare images

build_image_if_missing() {
  local image="$1"
  shift

  if podman image exists "$image"; then
    message "Image $image already exists, skipping build."
    return
  fi

  message "Building image $image..."
  podman build -t "$image" "$@"
}

build_image() {
  local image="$1"
  shift

  if [ "$REBUILD_IMAGES" = true ]; then
    message "Rebuilding image $image..."
    podman build -t "$image" "$@"
    return
  fi

  build_image_if_missing "$image" "$@"
}

build_image "$WGET_IMAGE" \
  -f Dockerfile.wget \
  .

build_image "$NOMINATIM_GIS_IMAGE" \
  -f nominatim/gis/Dockerfile \
  nominatim/gis

build_image "$NOMINATIM_FEED_IMAGE" \
  -f nominatim/feed/Dockerfile \
  nominatim/feed

build_image "$VALHALLA_TILES2PACKS_IMAGE" \
  --build-arg "VALHALLA_VERSION=${VALHALLA_VERSION}" \
  -f valhalla/Dockerfile.tiles2packs \
  valhalla

build_image "$POSTPROCESS_IMAGE" \
  -f Dockerfile.postprocess \
  .
